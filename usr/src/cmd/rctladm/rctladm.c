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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/rctl_impl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <libintl.h>
#include <locale.h>
#include <rctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"

#define	ENABLE	1
#define	DISABLE	0

#define	CONFIGPATH	"/etc/rctladm.conf"
#define	CONFIGOWNER	0	/* uid 0 (root) */
#define	CONFIGGROUP	1	/* gid 1 (other) */
#define	CONFIGPERM	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) /* 0644 */

/*
 *	Macros to produce a quoted string containing the value of a
 *	preprocessor macro. For example, if SIZE is defined to be 256,
 *	VAL2STR(SIZE) is "256". This is used to construct format
 *	strings for scanf-family functions below.
 */
#define	QUOTE(x)	#x
#define	VAL2STR(x)	QUOTE(x)

static const char USAGE[] =
	"Usage:\trctladm -l\n"
	"\trctladm -u\n"
	"\trctladm -e actions -d actions rctl_name\n";

static const char OPTS[] = "d:e:lu";
static int dflg, eflg, lflg, uflg;

static uint_t op_failures;

static void rctladm_enable(const char *, char *);

#define	BUFSIZE 256

static void
usage()
{
	(void) fprintf(stderr, gettext(USAGE));
	exit(E_USAGE);
}

#define	LOG_HIGHEST LOG_DEBUG
static const char *syslog_priorities[] = {
	"emerg",	/* LOG_EMERG	*/
	"alert",	/* LOG_ALERT	*/
	"crit",		/* LOG_CRIT	*/
	"err",		/* LOG_ERR	*/
	"warning",	/* LOG_WARNING	*/
	"notice",	/* LOG_NOTICE	*/
	"info",		/* LOG_INFO	*/
	"debug"		/* LOG_DEBUG	*/
};

static int
rctladm_syslog_prio(const char *priority)
{
	uint_t i;

	for (i = 0; i < LOG_HIGHEST + 1; i++) {
		if ((strcasecmp(priority, syslog_priorities[i]) == 0))
			return (i);
	}

	die(gettext("unknown syslog priority \"%s\"\n"), priority);

	/*NOTREACHED*/
}

/*ARGSUSED*/
static int
rctl_save_walk_cb(const char *rctl_name, void *file)
{
	FILE *fp = file;
	rctlblk_t *gblk;
	uint_t action;
	rctl_opaque_t *gopq;

	if ((gblk = malloc(rctlblk_size())) == NULL)
		die(gettext("unable to allocate control block"));


	if (rctlctl(rctl_name, gblk, RCTLCTL_GET) == -1) {
		warn(gettext("unable to obtain control block contents for %s"),
		    rctl_name);
	} else {
		action = rctlblk_get_global_action(gblk);
		gopq = (rctl_opaque_t *)gblk;

		(void) fprintf(fp, "%s=", rctl_name);
		if (action & RCTL_GLOBAL_SYSLOG)
			(void) fprintf(fp, "syslog=%s\n",
			    syslog_priorities[gopq->rcq_global_syslog_level]);
		else
			(void) fprintf(fp, "none\n");
	}

	free(gblk);

	return (0);
}

static void
rctladm_save_config()
{
	int fd;
	FILE *fp;

	/*
	 * Non-root users shouldn't update the configuration file.
	 */
	if (geteuid() != 0)
		return;

	if ((fd = open(CONFIGPATH, O_WRONLY|O_CREAT|O_TRUNC, CONFIGPERM)) == -1)
		die(gettext("failed to open %s"), CONFIGPATH);

	if ((fp = fdopen(fd, "w")) == NULL)
		die(gettext("failed to open stream for %s"), CONFIGPATH);

	(void) fputs(
	    "#\n"
	    "# rctladm.conf\n"
	    "#\n"
	    "# Parameters for resource controls configuration.\n"
	    "# Do NOT edit this file by hand -- use rctladm(1m) instead.\n"
	    "#\n",
	    fp);

	(void) rctl_walk(rctl_save_walk_cb, fp);

	(void) fflush(fp);
	(void) fsync(fd);
	(void) fchmod(fd, CONFIGPERM);
	(void) fchown(fd, CONFIGOWNER, CONFIGGROUP);
	(void) fclose(fp);
}

static void
rctladm_setup_action(char *name, char *action, int line)
{
	if (action[0] == '\0') {
		warn(gettext("\"%s\", line %d, syntax error\n"), CONFIGPATH,
		    line);
		return;
	}
	rctladm_enable(name, action);
}

static void
rctladm_read_config()
{
	int fd;
	FILE *fp;
	char buf[BUFSIZE];
	char name[BUFSIZE+1], actions[BUFSIZE+1];
	char *action;
	int line, len, n;
	rctl_opaque_t *gblk;

	/*
	 * Non-root users shouldn't do this.
	 */
	if (geteuid() != 0)
		die(gettext("you must be root to use this option\n"));

	if ((fd = open(CONFIGPATH, O_RDONLY, CONFIGPERM)) == -1)
		die(gettext("failed to open %s"), CONFIGPATH);

	if ((fp = fdopen(fd, "r")) == NULL)
		die(gettext("failed to open stream for %s"), CONFIGPATH);

	if ((gblk = malloc(rctlblk_size())) == NULL)
		die(gettext("unable to allocate control block"));

	for (line = 1; fgets(buf, BUFSIZE, fp) != NULL; line++) {
		/*
		 * Skip comment lines and empty lines.
		 */
		if (buf[0] == '#' || buf[0] == '\n')
			continue;

		/*
		 * Look for "rctl_name=action;action;...;action, with
		 * optional whitespace on either side, terminated by a newline,
		 * and consuming the whole line.
		 */
		n = sscanf(buf,
		    " %" VAL2STR(BUFSIZE) "[^=]=%" VAL2STR(BUFSIZE) "s \n%n",
		    name, actions, &len);
		if (n >= 1 && name[0] != '\0' &&
		    (n == 1 || len == strlen(buf))) {
			if (n == 1) {
				warn(gettext("\"%s\", line %d, syntax error\n"),
				    CONFIGPATH, line);
				continue;
			}
			if (rctlctl(name, (rctlblk_t *)gblk,
			    RCTLCTL_GET) == -1) {
				warn(gettext("\"%s\", line %d, unknown resource"
				    " control: %s\n"), CONFIGPATH, line, name);
				continue;
			}
			if (actions[0] == ';') {
				warn(gettext("\"%s\", line %d, syntax error\n"),
				    CONFIGPATH, line);
				continue;
			}
			action = strtok(actions, ";");
			rctladm_setup_action(name, action, line);
			while (action = strtok(NULL, ";"))
				rctladm_setup_action(name, action, line);
		}
	}

	if (line == 1)
		die(gettext("failed to read rctl configuration from \"%s\""),
		    CONFIGPATH);
	free(gblk);
	(void) fclose(fp);
}

static void
rctladm_modify_action(const char *rctl_name, uint_t enable, uint_t action,
    int log_level)
{
	rctl_opaque_t *gblk;

	if ((gblk = malloc(rctlblk_size())) == NULL)
		die(gettext("unable to allocate control block"));

	if (rctlctl(rctl_name, (rctlblk_t *)gblk, RCTLCTL_GET) == -1)
		die(gettext("unable to obtain resource control block"));

	if (gblk->rcq_global_flagaction & RCTL_GLOBAL_SYSLOG_NEVER) {
		warn(gettext("\"syslog\" action not valid for %s\n"),
		    rctl_name);
		op_failures++;
		free(gblk);
		return;
	}

	if (enable) {
		gblk->rcq_global_flagaction |= (action &
		    ~RCTL_GLOBAL_ACTION_MASK);
		gblk->rcq_global_syslog_level = log_level;
	} else {
		gblk->rcq_global_flagaction &= ~(action &
		    ~RCTL_GLOBAL_ACTION_MASK);
		gblk->rcq_global_syslog_level = LOG_NOTICE;
	}

	if (rctlctl(rctl_name, (rctlblk_t *)gblk, RCTLCTL_SET) == -1) {
		warn(gettext("unable to update control block contents"));
		op_failures++;
	}

	free(gblk);
}

static int
rctladm_get_log_level(char *action)
{
	char *log_lvl_str;

	/*
	 * Our syslog priority defaults to LOG_NOTICE.
	 */
	if (strcmp("syslog", action) == 0)
		return (LOG_NOTICE);

	if (strncmp("syslog=", action, strlen("syslog=")) != 0)
		die(gettext("unknown action \"%s\"\n"), action);

	log_lvl_str = action + strlen("syslog=");

	return (rctladm_syslog_prio(log_lvl_str));
}


static void
rctladm_enable(const char *rctl_name, char *action)
{
	/*
	 * Two valid values:  "none" and "syslog[=level]".
	 */
	if (strcmp("none", action) == 0) {
		rctladm_modify_action(rctl_name, DISABLE,
		    ~RCTL_GLOBAL_ACTION_MASK, 0);
		return;
	}

	rctladm_modify_action(rctl_name, ENABLE, RCTL_GLOBAL_SYSLOG,
	    rctladm_get_log_level(action));
}

static void
rctladm_disable(const char *rctl_name, char *action)
{
	/*
	 * Two valid values:  "all" and "syslog".
	 */
	if (strcmp("all", action) == 0) {
		rctladm_modify_action(rctl_name, DISABLE,
		    ~RCTL_GLOBAL_ACTION_MASK, 0);
		return;
	} else if (strcmp("syslog", action) == 0) {
		rctladm_modify_action(rctl_name, DISABLE, RCTL_GLOBAL_SYSLOG,
		    0);
		return;
	}

	die(gettext("unknown action \"%s\"\n"), action);
}

static void
rctlblk_display(FILE *f, rctlblk_t *gblk)
{
	uint_t action = rctlblk_get_global_action(gblk);
	uint_t flags = rctlblk_get_global_flags(gblk);
	rctl_opaque_t *gopq = (rctl_opaque_t *)gblk;

	if (flags & RCTL_GLOBAL_SYSLOG_NEVER)
		(void) fprintf(f, "syslog=n/a    ");
	else if (action & RCTL_GLOBAL_SYSLOG)
		(void) fprintf(f, "syslog=%-7s",
		    syslog_priorities[gopq->rcq_global_syslog_level]);
	else
		(void) fprintf(f, "syslog=off    ");

	if (flags & RCTL_GLOBAL_ACTION_MASK)
		(void) fprintf(f, " [");

	if (flags & RCTL_GLOBAL_NOBASIC)
		(void) fprintf(f, " no-basic");
	if (flags & RCTL_GLOBAL_LOWERABLE)
		(void) fprintf(f, " lowerable");
	if (flags & RCTL_GLOBAL_DENY_ALWAYS)
		(void) fprintf(f, " deny");
	if (flags & RCTL_GLOBAL_DENY_NEVER)
		(void) fprintf(f, " no-deny");
	if (flags & RCTL_GLOBAL_CPU_TIME)
		(void) fprintf(f, " cpu-time");
	if (flags & RCTL_GLOBAL_FILE_SIZE)
		(void) fprintf(f, " file-size");
	if (flags & RCTL_GLOBAL_SIGNAL_NEVER)
		(void) fprintf(f, " no-signal");
	if (flags & RCTL_GLOBAL_UNOBSERVABLE)
		(void) fprintf(f, " no-obs");
	if (flags & RCTL_GLOBAL_INFINITE)
		(void) fprintf(f, " inf");
	if (flags & RCTL_GLOBAL_SYSLOG_NEVER)
		(void) fprintf(f, " no-syslog");
	if (flags & RCTL_GLOBAL_SECONDS)
		(void) fprintf(f, " seconds");
	if (flags & RCTL_GLOBAL_BYTES)
		(void) fprintf(f, " bytes");
	if (flags & RCTL_GLOBAL_COUNT)
		(void) fprintf(f, " count");
	if (flags & RCTL_GLOBAL_ACTION_MASK)
		(void) fprintf(f, " ]");

	(void) fprintf(f, "\n");
}

/*ARGSUSED*/
static int
rctl_walk_cb(const char *rctl_name, void *pvt)
{
	rctlblk_t *gblk;

	if ((gblk = malloc(rctlblk_size())) == NULL)
		die(gettext("unable to allocate control block"));

	if (rctlctl(rctl_name, gblk, RCTLCTL_GET) == -1) {
		if (errno == ESRCH)
			warn(gettext("unknown resource control: %s\n"),
			    rctl_name);
		else
			warn(gettext("unable to obtain %s properties"),
			    rctl_name);
		op_failures++;
	} else {
		(void) printf("%-27s ", rctl_name);
		rctlblk_display(stdout, gblk);
	}

	free(gblk);

	return (0);
}

static void
rctladm_list_rctls(int optind, int argc, char *argv[])
{
	if (optind >= argc) {
		(void) rctl_walk(rctl_walk_cb, NULL);
		return;
	}

	for (; optind < argc; optind++)
		(void) rctl_walk_cb(argv[optind], NULL);
}

int
main(int argc, char *argv[])
{
	int c;			/* options character */
	char *action;
	char *rctl;

	(void) setlocale(LC_ALL, "");
	(void) textdomain(TEXT_DOMAIN);
	(void) setprogname(argv[0]);

	while ((c = getopt(argc, argv, OPTS)) != EOF) {
		switch (c) {
			case 'd':
				dflg++;
				action = optarg;
				break;
			case 'e':
				eflg++;
				action = optarg;
				break;
			case 'l':
				lflg = 1;
				break;
			case 'u':
				uflg = 1;
				break;
			case '?':
			default:
				usage();
		}
	}

	if (uflg) {
		rctladm_read_config();
		return (E_SUCCESS);
	}

	if (lflg && (dflg || eflg)) {
		warn(gettext("-l, -d, and -e flags are exclusive\n"));
		usage();
	}

	if (dflg && eflg) {
		warn(gettext("-d and -e flags are exclusive\n"));
		usage();
	}

	if (dflg > 1 || eflg > 1) {
		warn(gettext("only one -d or -e flag per line\n"));
		usage();
	}

	if (lflg || !(dflg || eflg)) {
		rctladm_list_rctls(optind, argc, argv);
		rctladm_save_config();

		return (op_failures ? E_ERROR : E_SUCCESS);
	}

	if (optind >= argc) {
		warn(gettext("must specify one or more "
		    "resource control names\n"));
		usage();
	}

	for (; optind < argc; optind++) {
		rctl = argv[optind];

		if (eflg) {
			rctladm_enable(rctl, action);
			rctladm_save_config();
		} else if (dflg) {
			rctladm_disable(rctl, action);
			rctladm_save_config();
		} else {
			usage();
		}
	}

	return (op_failures ? E_ERROR : E_SUCCESS);
}
