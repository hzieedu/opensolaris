/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * main() of idmapd(1M)
 */

#include "idmapd.h"
#include <signal.h>
#include <rpc/pmap_clnt.h> /* for pmap_unset */
#include <string.h> /* strcmp */
#include <unistd.h> /* setsid */
#include <sys/types.h>
#include <memory.h>
#include <stropts.h>
#include <netconfig.h>
#include <sys/resource.h> /* rlimit */
#include <syslog.h>
#include <rpcsvc/daemon_utils.h> /* DAEMON_UID and DAEMON_GID */
#include <priv_utils.h> /* privileges */
#include <locale.h>
#include <sys/systeminfo.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <zone.h>
#include <door.h>
#include <tsol/label.h>
#include <sys/resource.h>
#include <sys/sid.h>
#include <sys/idmap.h>

static void	hup_handler(int);
static void	term_handler(int);
static void	init_idmapd();
static void	fini_idmapd();

#ifndef SIG_PF
#define	SIG_PF void(*)(int)
#endif

#define	_RPCSVC_CLOSEDOWN 120

int _rpcsvcstate = _IDLE;	/* Set when a request is serviced */
int _rpcsvccount = 0;		/* Number of requests being serviced */
mutex_t _svcstate_lock;		/* lock for _rpcsvcstate, _rpcsvccount */
idmapd_state_t	_idmapdstate;

SVCXPRT *xprt = NULL;

static int dfd = -1;		/* our door server fildes, for unregistration */

#ifdef DEBUG
#define	RPC_SVC_FG
#endif

/*
 * This is needed for mech_krb5 -- we run as daemon, yes, but we want
 * mech_krb5 to think we're root.
 *
 * Someday we'll have gss/mech_krb5 extensions for acquiring initiator
 * creds with keytabs/raw keys, and someday we'll have extensions to
 * libsasl to specify creds/name to use on the initiator side, and
 * someday we'll have extensions to libldap to pass those through to
 * libsasl.  Until then this interposer will have to do.
 *
 * Also, we have to tell lint to shut up: it thinks app_krb5_user_uid()
 * is defined but not used.
 */
/*LINTLIBRARY*/
uid_t
app_krb5_user_uid(void)
{
	return (0);
}

static void
set_signal_handlers() {
	(void) sigset(SIGPIPE, SIG_IGN);
	(void) sigset(SIGHUP, hup_handler);
	(void) sigset(SIGTERM, term_handler);
}

/*ARGSUSED*/
static void
hup_handler(int sig) {
	(void) idmapdlog(LOG_INFO, "idmapd: Refreshing config.");
	WRLOCK_CONFIG();
	(void) idmap_cfg_fini(_idmapdstate.cfg);
	_idmapdstate.cfg = NULL;
	if (load_config() < 0) {
		UNLOCK_CONFIG();
		(void) idmapdlog(LOG_NOTICE,
			"idmapd: Failed to reload config");
		term_handler(sig);
	}
	UNLOCK_CONFIG();
	print_idmapdstate();
}

/*ARGSUSED*/
static void
term_handler(int sig) {
	(void) idmapdlog(LOG_INFO, "idmapd: Terminating.");
	fini_idmapd();
	_exit(0);
}

static int pipe_fd = -1;

static void
daemonize_ready(void) {
	char data = '\0';
	/*
	 * wake the parent
	 */
	(void) write(pipe_fd, &data, 1);
	(void) close(pipe_fd);
}

static int
daemonize_start(void) {
	char	data;
	int	status;
	int	devnull;
	int	filedes[2];
	pid_t	pid;

	devnull = open("/dev/null", O_RDONLY);
	if (devnull < 0)
		return (-1);
	(void) dup2(devnull, 0);
	(void) dup2(2, 1);	/* stderr only */
	if (pipe(filedes) < 0)
		return (-1);
	if ((pid = fork1()) < 0)
		return (-1);
	if (pid != 0) {
		/*
		 * parent
		 */
		struct sigaction act;
		act.sa_sigaction = SIG_DFL;
		(void) sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		(void) sigaction(SIGPIPE, &act, NULL); /* ignore SIGPIPE */
		(void) close(filedes[1]);
		if (read(filedes[0], &data, 1) == 1) {
			/* presume success */
			_exit(0);
		}
		status = -1;
		(void) wait4(pid, &status, 0, NULL);
		if (WIFEXITED(status))
			_exit(WEXITSTATUS(status));
		else
			_exit(-1);
	}

	/*
	 * child
	 */
	pipe_fd = filedes[1];
	(void) close(filedes[0]);
	(void) setsid();
	(void) umask(0077);
	openlog("idmap", LOG_PID, LOG_DAEMON);
	_idmapdstate.daemon_mode = TRUE;
	return (0);
}


int
main(int argc, char **argv)
{
	int c;
#ifdef RPC_SVC_FG
	bool_t daemonize = FALSE;
#else
	bool_t daemonize = TRUE;
#endif

	while ((c = getopt(argc, argv, "d")) != EOF) {
		switch (c) {
			case 'd':
				daemonize = FALSE;
				break;
			default:
				break;
		}
	}

	/* set locale and domain for internationalization */
	(void) setlocale(LC_ALL, "");
	(void) textdomain(TEXT_DOMAIN);

	if (is_system_labeled() && (getzoneid() != GLOBAL_ZONEID)) {
		(void) idmapdlog(LOG_ERR,
		    "idmapd: With TX, idmapd runs only in the global zone");
		exit(1);
	}

	/* create directories as root and chown to daemon uid */
	if (create_directory(IDMAP_DBDIR, DAEMON_UID, DAEMON_GID) < 0)
		exit(1);
	if (create_directory(IDMAP_CACHEDIR, DAEMON_UID, DAEMON_GID) < 0)
		exit(1);

	INIT_IDMAPD_STATE();

	(void) mutex_init(&_svcstate_lock, USYNC_THREAD, NULL);
	set_signal_handlers();

	if (daemonize == TRUE) {
		if (daemonize_start() < 0) {
			(void) perror("idmapd: unable to daemonize");
			exit(-1);
		}
	} else
		(void) umask(0077);

	init_idmapd();

	if (__init_daemon_priv(PU_RESETGROUPS|PU_CLEARLIMITSET,
	    DAEMON_UID, DAEMON_GID,
	    PRIV_PROC_AUDIT, PRIV_FILE_DAC_READ,
	    (char *)NULL) == -1) {
		(void) idmapdlog(LOG_ERR,
		    gettext("idmapd: unable to drop privileges"));
		exit(1);
	}

	__fini_daemon_priv(PRIV_PROC_FORK, PRIV_PROC_EXEC, PRIV_PROC_SESSION,
	    PRIV_FILE_LINK_ANY, PRIV_PROC_INFO, (char *)NULL);

	if (daemonize == TRUE)
		daemonize_ready();

	/* With doors RPC this just wastes this thread, oh well */
	svc_run();
	return (0);
}

static void
init_idmapd() {
	int	error;

	memset(&_idmapdstate, 0, sizeof (_idmapdstate));

	if (sysinfo(SI_HOSTNAME, _idmapdstate.hostname,
			sizeof (_idmapdstate.hostname)) == -1) {
		error = errno;
		idmapdlog(LOG_ERR,
	"idmapd: unable to determine hostname, error: %d",
			error);
		exit(1);
	}

	if (sysinfo(SI_SRPC_DOMAIN, _idmapdstate.domainname,
			sizeof (_idmapdstate.domainname)) == -1) {
		error = errno;
		idmapdlog(LOG_ERR,
	"idmapd: unable to determine name service domain, error: %d",
			error);
		exit(1);
	}

	setegid(DAEMON_GID);
	seteuid(DAEMON_UID);
	if (init_mapping_system() < 0) {
		idmapdlog(LOG_ERR,
		"idmapd: unable to initialize mapping system");
		exit(1);
	}
	seteuid(0);
	setegid(0);

	xprt = svc_door_create(idmap_prog_1, IDMAP_PROG, IDMAP_V1, 0);
	if (xprt == NULL) {
		idmapdlog(LOG_ERR,
		"idmapd: unable to create door RPC service");
		goto errout;
	}

	dfd = xprt->xp_fd;

	if (dfd == -1) {
		idmapdlog(LOG_ERR, "idmapd: unable to register door");
		goto errout;
	}
	if ((error = idmap_reg(dfd)) != 0) {
		idmapdlog(LOG_ERR, "idmapd: unable to register door (%s)",
				strerror(error));
		goto errout;
	}

	if ((error = allocids(_idmapdstate.new_eph_db,
			8192, &_idmapdstate.next_uid,
			8192, &_idmapdstate.next_gid)) != 0) {
		idmapdlog(LOG_ERR, "idmapd: unable to allocate ephemeral IDs "
			"(%s)", strerror(error));
		_idmapdstate.next_uid = _idmapdstate.limit_uid = SENTINEL_PID;
		_idmapdstate.next_gid = _idmapdstate.limit_gid = SENTINEL_PID;
	} else {
		_idmapdstate.limit_uid = _idmapdstate.next_uid + 8192;
		_idmapdstate.limit_gid = _idmapdstate.next_gid + 8192;
	}

	print_idmapdstate();

	return;

errout:
	fini_idmapd();
	exit(1);
}

static void
fini_idmapd() {
	idmap_unreg(dfd);
	fini_mapping_system();
	if (xprt != NULL)
		svc_destroy(xprt);
}

void
idmapdlog(int pri, const char *format, ...) {
	va_list args;

	va_start(args, format);
	if (_idmapdstate.daemon_mode == FALSE) {
		(void) vfprintf(stderr, format, args);
		(void) fprintf(stderr, "\n");
	}
	(void) vsyslog(pri, format, args);
	va_end(args);
}