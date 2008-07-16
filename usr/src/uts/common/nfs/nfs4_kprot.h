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
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef _NFS4_KPROT_H
#define	_NFS4_KPROT_H

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * Kernel specific version.
 * NFS Version 4 protocol definitions.  From nfs4_prot.x rev 1.119.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <rpc/rpc.h>
#include <sys/stream.h>
#include <nfs/nfs4_attrmap.h>

#define	NFS4_FHSIZE 128
#define	NFS4_VERIFIER_SIZE 8

/*
 * Reasonable upper bounds to catch badly behaving partners
 */
#define	NFS4_OPAQUE_LIMIT	1024
#define	NFS4_COMPOUND_LIMIT	2048
#define	NFS4_FS_LOCATIONS_LIMIT	65536
#define	NFS4_ACL_LIMIT		65536
#define	NFS4_SECINFO_LIMIT	65536
#define	NFS4_FATTR4_LIMIT	1048576
#define	NFS4_DATA_LIMIT		134217728

enum nfs_ftype4 {
	NF4REG = 1,
	NF4DIR = 2,
	NF4BLK = 3,
	NF4CHR = 4,
	NF4LNK = 5,
	NF4SOCK = 6,
	NF4FIFO = 7,
	NF4ATTRDIR = 8,
	NF4NAMEDATTR = 9
};
typedef enum nfs_ftype4 nfs_ftype4;

/*
 * XXX - nfsstat4 was extended by NFSv4.1. It
 *	 is now defined in nfs4_ext_defs.h
 */

/*
 * A bitmap can only be 56 bits, treat it as a uint64_t for now
 */
typedef uint64_t bitmap4;

typedef uint64_t offset4;

typedef uint32_t count4;

typedef uint64_t length4;

typedef uint64_t clientid4;

typedef uint32_t seqid4;

typedef struct {
	uint_t utf8string_len;
	char *utf8string_val;
} utf8string;

typedef utf8string component4;

typedef struct {
	uint_t pathname4_len;
	component4 *pathname4_val;
} pathname4;

typedef uint64_t nfs_lockid4;

typedef uint64_t nfs_cookie4;

typedef utf8string linktext4;

typedef struct {
	uint_t sec_oid4_len;
	char *sec_oid4_val;
} sec_oid4;

typedef uint32_t qop4;

typedef uint32_t mode4;

typedef uint64_t changeid4;

typedef uint64_t verifier4;

struct nfstime4 {
	int64_t seconds;
	uint32_t nseconds;
};
typedef struct nfstime4 nfstime4;

enum time_how4 {
	SET_TO_SERVER_TIME4 = 0,
	SET_TO_CLIENT_TIME4 = 1
};
typedef enum time_how4 time_how4;

struct settime4 {
	time_how4 set_it;
	nfstime4 time;
};
typedef struct settime4 settime4;

typedef struct {
	uint_t nfs_fh4_len;
	char *nfs_fh4_val;
} nfs_fh4;

struct fsid4 {
	uint64_t major;
	uint64_t minor;
};
typedef struct fsid4 fsid4;

struct fs_location4 {
	uint_t server_len;
	utf8string *server_val;
	pathname4 rootpath;
};
typedef struct fs_location4 fs_location4;

struct fs_locations4 {
	pathname4 fs_root;
	uint_t locations_len;
	fs_location4 *locations_val;
};
typedef struct fs_locations4 fs_locations4;

/*
 * ACL support
 */

#define	ACL4_SUPPORT_ALLOW_ACL 0x00000001
#define	ACL4_SUPPORT_DENY_ACL 0x00000002
#define	ACL4_SUPPORT_AUDIT_ACL 0x00000004
#define	ACL4_SUPPORT_ALARM_ACL 0x00000008

typedef uint32_t acetype4;
#define	ACE4_ACCESS_ALLOWED_ACE_TYPE 0x00000000
#define	ACE4_ACCESS_DENIED_ACE_TYPE 0x00000001
#define	ACE4_SYSTEM_AUDIT_ACE_TYPE 0x00000002
#define	ACE4_SYSTEM_ALARM_ACE_TYPE 0x00000003

typedef uint32_t aceflag4;
#define	ACE4_FILE_INHERIT_ACE 0x00000001
#define	ACE4_DIRECTORY_INHERIT_ACE 0x00000002
#define	ACE4_NO_PROPAGATE_INHERIT_ACE 0x00000004
#define	ACE4_INHERIT_ONLY_ACE 0x00000008
#define	ACE4_SUCCESSFUL_ACCESS_ACE_FLAG 0x00000010
#define	ACE4_FAILED_ACCESS_ACE_FLAG 0x00000020
#define	ACE4_IDENTIFIER_GROUP 0x00000040
#define	ACE4_INHERITED_ACE 0x00000080			/* 4.1 */
/*
 * This defines all valid flag bits, as defined by RFC 3530.  If
 * any additional flag bits are deemed part of the NFSv4 spec,
 * you must also add them to the definition below.
 */
#define	ACE4_VALID_FLAG_BITS (\
    ACE4_FILE_INHERIT_ACE | \
    ACE4_DIRECTORY_INHERIT_ACE | \
    ACE4_NO_PROPAGATE_INHERIT_ACE | \
    ACE4_INHERIT_ONLY_ACE | \
    ACE4_SUCCESSFUL_ACCESS_ACE_FLAG | \
    ACE4_FAILED_ACCESS_ACE_FLAG | \
    ACE4_IDENTIFIER_GROUP)

typedef uint32_t acemask4;
#define	ACE4_READ_DATA 0x00000001
#define	ACE4_LIST_DIRECTORY 0x00000001
#define	ACE4_WRITE_DATA 0x00000002
#define	ACE4_ADD_FILE 0x00000002
#define	ACE4_APPEND_DATA 0x00000004
#define	ACE4_ADD_SUBDIRECTORY 0x00000004
#define	ACE4_READ_NAMED_ATTRS 0x00000008
#define	ACE4_WRITE_NAMED_ATTRS 0x00000010
#define	ACE4_EXECUTE 0x00000020
#define	ACE4_DELETE_CHILD 0x00000040
#define	ACE4_READ_ATTRIBUTES 0x00000080
#define	ACE4_WRITE_ATTRIBUTES 0x00000100

/* XXX - ACE4_WRITE_RETENTION{_HOLD} defined by NFSv4.1 */

#define	ACE4_DELETE 0x00010000
#define	ACE4_READ_ACL 0x00020000
#define	ACE4_WRITE_ACL 0x00040000
#define	ACE4_WRITE_OWNER 0x00080000
#define	ACE4_SYNCHRONIZE 0x00100000
#define	ACE4_GENERIC_READ 0x00120081
#define	ACE4_GENERIC_WRITE 0x00160106
#define	ACE4_GENERIC_EXECUTE 0x001200A0
/*
 * This defines all valid access mask bits, as defined by RFC 3530.  If
 * any additional access mask bits are deemed part of the NFSv4 spec,
 * you must also add them to the definition below.
 */
#define	ACE4_VALID_MASK_BITS (\
    ACE4_READ_DATA | \
    ACE4_LIST_DIRECTORY | \
    ACE4_WRITE_DATA | \
    ACE4_ADD_FILE | \
    ACE4_APPEND_DATA | \
    ACE4_ADD_SUBDIRECTORY | \
    ACE4_READ_NAMED_ATTRS | \
    ACE4_WRITE_NAMED_ATTRS | \
    ACE4_EXECUTE | \
    ACE4_DELETE_CHILD | \
    ACE4_READ_ATTRIBUTES | \
    ACE4_WRITE_ATTRIBUTES | \
    ACE4_DELETE | \
    ACE4_READ_ACL | \
    ACE4_WRITE_ACL | \
    ACE4_WRITE_OWNER | \
    ACE4_SYNCHRONIZE)

/* Used to signify an undefined value for an acemask4 */
#define	ACE4_MASK_UNDEFINED 0x80000000

#define	ACE4_WHO_OWNER		"OWNER@"
#define	ACE4_WHO_GROUP		"GROUP@"
#define	ACE4_WHO_EVERYONE	"EVERYONE@"

struct nfsace4 {
	acetype4 type;
	aceflag4 flag;
	acemask4 access_mask;
	utf8string who;
};
typedef struct nfsace4 nfsace4;

typedef uint32_t aclflag4;				/* 4.1 */
#define	ACL4_AUTO_INHERIT 0x00000001
#define	ACL4_PROTECTED 0x00000002
#define	ACL4_DEFAULTED 0x00000004

struct nfsacl41 {					/* 4.1 */
	aclflag4 na41_flag;
	struct {
		uint_t na41_aces_len;
		nfsace4 *na41_aces_val;
	} na41_aces;
};
typedef struct nfsacl41 nfsacl41;
#define	MODE4_SUID 0x800
#define	MODE4_SGID 0x400
#define	MODE4_SVTX 0x200
#define	MODE4_RUSR 0x100
#define	MODE4_WUSR 0x080
#define	MODE4_XUSR 0x040
#define	MODE4_RGRP 0x020
#define	MODE4_WGRP 0x010
#define	MODE4_XGRP 0x008
#define	MODE4_ROTH 0x004
#define	MODE4_WOTH 0x002
#define	MODE4_XOTH 0x001

struct mode_masked4 {					/* 4.1 */
	mode4 mm_value_to_set;
	mode4 mm_mask_bits;
};
typedef struct mode_masked4 mode_masked4;

/*
 * ACL conversion helpers
 */

typedef enum {
	ace4_unused,
	ace4_user_obj,
	ace4_user,
	ace4_group, /* includes GROUP and GROUP_OBJ */
	ace4_other_obj
} ace4_to_aent_state_t;

typedef struct ace4vals {
	utf8string *key; /* NB: not allocated here; points to existing utf8 */
	avl_node_t avl;
	acemask4 mask;
	acemask4 allowed;
	acemask4 denied;
	int aent_type;
} ace4vals_t;

typedef struct ace4_list {
	ace4vals_t user_obj;
	avl_tree_t user;
	int numusers;
	ace4vals_t group_obj;
	avl_tree_t group;
	int numgroups;
	ace4vals_t other_obj;
	acemask4 acl_mask;
	int hasmask;
	int dfacl_flag;
	ace4_to_aent_state_t state;
	int seen; /* bitmask of all aclent_t a_type values seen */
} ace4_list_t;

struct specdata4 {
	uint32_t specdata1;
	uint32_t specdata2;
};
typedef struct specdata4 specdata4;
#define	FH4_PERSISTENT 0x00000000
#define	FH4_NOEXPIRE_WITH_OPEN 0x00000001
#define	FH4_VOLATILE_ANY 0x00000002
#define	FH4_VOL_MIGRATION 0x00000004
#define	FH4_VOL_RENAME 0x00000008

typedef attrmap4 fattr4_supported_attrs;

typedef nfs_ftype4 fattr4_type;

typedef uint32_t fattr4_fh_expire_type;

typedef changeid4 fattr4_change;

typedef uint64_t fattr4_size;

typedef bool_t fattr4_link_support;

typedef bool_t fattr4_symlink_support;

typedef bool_t fattr4_named_attr;

typedef fsid4 fattr4_fsid;

typedef bool_t fattr4_unique_handles;

typedef uint32_t fattr4_lease_time;

/* XXX - 4.0 extended type definitions */
#include <nfs/nfs4_ext_defs.h>

typedef nfsstat4 fattr4_rdattr_error;

typedef struct {
	uint_t fattr4_acl_len;
	nfsace4 *fattr4_acl_val;
} fattr4_acl;

typedef uint32_t fattr4_aclsupport;

typedef bool_t fattr4_archive;

typedef bool_t fattr4_cansettime;

typedef bool_t fattr4_case_insensitive;

typedef bool_t fattr4_case_preserving;

typedef bool_t fattr4_chown_restricted;

typedef uint64_t fattr4_fileid;

typedef uint64_t fattr4_files_avail;

typedef nfs_fh4 fattr4_filehandle;

typedef uint64_t fattr4_files_free;

typedef uint64_t fattr4_files_total;

typedef fs_locations4 fattr4_fs_locations;

typedef bool_t fattr4_hidden;

typedef bool_t fattr4_homogeneous;

typedef uint64_t fattr4_maxfilesize;

typedef uint32_t fattr4_maxlink;

typedef uint32_t fattr4_maxname;

typedef uint64_t fattr4_maxread;

typedef uint64_t fattr4_maxwrite;

typedef utf8string fattr4_mimetype;

typedef mode4 fattr4_mode;

typedef mode_masked4 fattr4_mode_set_masked;		/* 4.1 */

typedef uint64_t fattr4_mounted_on_fileid;

typedef bool_t fattr4_no_trunc;

typedef uint32_t fattr4_numlinks;

typedef utf8string fattr4_owner;

typedef utf8string fattr4_owner_group;

typedef uint64_t fattr4_quota_avail_hard;

typedef uint64_t fattr4_quota_avail_soft;

typedef uint64_t fattr4_quota_used;

typedef specdata4 fattr4_rawdev;

typedef uint64_t fattr4_space_avail;

typedef uint64_t fattr4_space_free;

typedef uint64_t fattr4_space_total;

typedef uint64_t fattr4_space_used;

typedef bool_t fattr4_system;

typedef nfstime4 fattr4_time_access;

typedef settime4 fattr4_time_access_set;

typedef nfstime4 fattr4_time_backup;

typedef nfstime4 fattr4_time_create;

typedef nfstime4 fattr4_time_delta;

typedef nfstime4 fattr4_time_metadata;

typedef nfstime4 fattr4_time_modify;

typedef settime4 fattr4_time_modify_set;
#define	FATTR4_SUPPORTED_ATTRS 0
#define	FATTR4_TYPE 1
#define	FATTR4_FH_EXPIRE_TYPE 2
#define	FATTR4_CHANGE 3
#define	FATTR4_SIZE 4
#define	FATTR4_LINK_SUPPORT 5
#define	FATTR4_SYMLINK_SUPPORT 6
#define	FATTR4_NAMED_ATTR 7
#define	FATTR4_FSID 8
#define	FATTR4_UNIQUE_HANDLES 9
#define	FATTR4_LEASE_TIME 10
#define	FATTR4_RDATTR_ERROR 11
#define	FATTR4_FILEHANDLE 19
#define	FATTR4_ACL 12
#define	FATTR4_ACLSUPPORT 13
#define	FATTR4_ARCHIVE 14
#define	FATTR4_CANSETTIME 15
#define	FATTR4_CASE_INSENSITIVE 16
#define	FATTR4_CASE_PRESERVING 17
#define	FATTR4_CHOWN_RESTRICTED 18
#define	FATTR4_FILEID 20
#define	FATTR4_FILES_AVAIL 21
#define	FATTR4_FILES_FREE 22
#define	FATTR4_FILES_TOTAL 23
#define	FATTR4_FS_LOCATIONS 24
#define	FATTR4_HIDDEN 25
#define	FATTR4_HOMOGENEOUS 26
#define	FATTR4_MAXFILESIZE 27
#define	FATTR4_MAXLINK 28
#define	FATTR4_MAXNAME 29
#define	FATTR4_MAXREAD 30
#define	FATTR4_MAXWRITE 31
#define	FATTR4_MIMETYPE 32
#define	FATTR4_MODE 33
#define	FATTR4_NO_TRUNC 34
#define	FATTR4_NUMLINKS 35
#define	FATTR4_OWNER 36
#define	FATTR4_OWNER_GROUP 37
#define	FATTR4_QUOTA_AVAIL_HARD 38
#define	FATTR4_QUOTA_AVAIL_SOFT 39
#define	FATTR4_QUOTA_USED 40
#define	FATTR4_RAWDEV 41
#define	FATTR4_SPACE_AVAIL 42
#define	FATTR4_SPACE_FREE 43
#define	FATTR4_SPACE_TOTAL 44
#define	FATTR4_SPACE_USED 45
#define	FATTR4_SYSTEM 46
#define	FATTR4_TIME_ACCESS 47
#define	FATTR4_TIME_ACCESS_SET 48
#define	FATTR4_TIME_BACKUP 49
#define	FATTR4_TIME_CREATE 50
#define	FATTR4_TIME_DELTA 51
#define	FATTR4_TIME_METADATA 52
#define	FATTR4_TIME_MODIFY 53
#define	FATTR4_TIME_MODIFY_SET 54
#define	FATTR4_MOUNTED_ON_FILEID 55

#define	NFS40_ATTR_COUNT (FATTR4_MOUNTED_ON_FILEID + 1)


struct fattr4 {
	attrmap4 attrmask;
	char *attrlist4;
	uint_t attrlist4_len;
};
typedef struct fattr4 fattr4;

struct change_info4 {
	bool_t atomic;
	changeid4 before;
	changeid4 after;
};
typedef struct change_info4 change_info4;

struct clientaddr4 {
	char *r_netid;
	char *r_addr;
};
typedef struct clientaddr4 clientaddr4;

struct cb_client4 {
	uint32_t cb_program;
	clientaddr4 cb_location;
};
typedef struct cb_client4 cb_client4;

struct stateid4 {
	uint32_t seqid;
	char other[12];
};
typedef struct stateid4 stateid4;

struct nfs_client_id4 {
	verifier4 verifier;
	uint_t id_len;
	char *id_val;
};
typedef struct nfs_client_id4 nfs_client_id4;

/* XXX - client_owner4 is a new NFSv4.1 addition */

struct open_owner4 {
	clientid4 clientid;
	uint_t owner_len;
	char *owner_val;
};
typedef struct open_owner4 open_owner4;

struct lock_owner4 {
	clientid4 clientid;
	uint_t owner_len;
	char *owner_val;
};
typedef struct lock_owner4 lock_owner4;

enum nfs_lock_type4 {
	READ_LT = 1,
	WRITE_LT = 2,
	READW_LT = 3,
	WRITEW_LT = 4
};
typedef enum nfs_lock_type4 nfs_lock_type4;

#define	ACCESS4_READ 0x00000001
#define	ACCESS4_LOOKUP 0x00000002
#define	ACCESS4_MODIFY 0x00000004
#define	ACCESS4_EXTEND 0x00000008
#define	ACCESS4_DELETE 0x00000010
#define	ACCESS4_EXECUTE 0x00000020

struct ACCESS4args {
	uint32_t access;
};
typedef struct ACCESS4args ACCESS4args;

struct ACCESS4res {
	nfsstat4 status;
	uint32_t supported;
	uint32_t access;
};
typedef struct ACCESS4res ACCESS4res;

struct CLOSE4args {
	seqid4 seqid;
	stateid4 open_stateid;
};
typedef struct CLOSE4args CLOSE4args;

struct CLOSE4res {
	nfsstat4 status;
	stateid4 open_stateid;
};
typedef struct CLOSE4res CLOSE4res;

struct COMMIT4args {
	offset4 offset;
	count4 count;
};
typedef struct COMMIT4args COMMIT4args;

struct COMMIT4res {
	nfsstat4 status;
	verifier4 writeverf;
};
typedef struct COMMIT4res COMMIT4res;

struct CREATE4args {
	nfs_ftype4 type;
	union {
		linktext4 linkdata;
		specdata4 devdata;
	} ftype4_u;
	component4 objname;
	fattr4 createattrs;
};
typedef struct CREATE4args CREATE4args;

struct CREATE4cargs {
	nfs_ftype4 type;
	union {
		char *clinkdata;
		specdata4 devdata;
	} ftype4_u;
	char *cname;
	fattr4 createattrs;
};
typedef struct CREATE4cargs CREATE4cargs;

struct CREATE4res {
	nfsstat4 status;
	change_info4 cinfo;
	attrmap4 attrset;
};
typedef struct CREATE4res CREATE4res;

struct DELEGPURGE4args {
	clientid4 clientid;
};
typedef struct DELEGPURGE4args DELEGPURGE4args;

struct DELEGPURGE4res {
	nfsstat4 status;
};
typedef struct DELEGPURGE4res DELEGPURGE4res;

struct DELEGRETURN4args {
	stateid4 deleg_stateid;
};
typedef struct DELEGRETURN4args DELEGRETURN4args;

struct DELEGRETURN4res {
	nfsstat4 status;
};
typedef struct DELEGRETURN4res DELEGRETURN4res;

struct mntinfo4;

struct GETATTR4args {
	attrmap4 attr_request;
	struct mntinfo4 *mi;
};
typedef struct GETATTR4args GETATTR4args;

struct nfs4_ga_ext_res;

struct nfs4_ga_res {
	vattr_t				n4g_va;
	unsigned			n4g_change_valid:1;
	unsigned			n4g_mon_fid_valid:1;
	unsigned			n4g_fsid_valid:1;
	uint_t				n4g_attrerr;
	uint_t				n4g_attrwhy;
	attrmap4			n4g_resbmap;
	fattr4_change			n4g_change;
	fattr4_fsid			n4g_fsid;
	fattr4_mounted_on_fileid	n4g_mon_fid;
	struct nfs4_ga_ext_res		*n4g_ext_res;
	vsecattr_t			n4g_vsa;
};
typedef struct nfs4_ga_res nfs4_ga_res_t;

struct GETATTR4res {
	nfsstat4 status;
	fattr4 obj_attributes;
	nfsstat4	ga_status;
	struct nfs4_ga_res ga_res;
};
typedef struct GETATTR4res GETATTR4res;

struct GETFH4res {
	nfsstat4 status;
	nfs_fh4 object;
};
typedef struct GETFH4res GETFH4res;

struct LINK4args {
	component4 newname;
};
typedef struct LINK4args LINK4args;

struct LINK4cargs {
	char *cnewname;
};
typedef struct LINK4cargs LINK4cargs;

struct LINK4res {
	nfsstat4 status;
	change_info4 cinfo;
};
typedef struct LINK4res LINK4res;

struct open_to_lock_owner4 {
	seqid4 open_seqid;
	stateid4 open_stateid;
	seqid4 lock_seqid;
	lock_owner4 lock_owner;
};
typedef struct open_to_lock_owner4 open_to_lock_owner4;

struct exist_lock_owner4 {
	stateid4 lock_stateid;
	seqid4 lock_seqid;
};
typedef struct exist_lock_owner4 exist_lock_owner4;

struct locker4 {
	bool_t new_lock_owner;
	union {
		open_to_lock_owner4 open_owner;
		exist_lock_owner4 lock_owner;
	} locker4_u;
};
typedef struct locker4 locker4;

struct LOCK4args {
	nfs_lock_type4 locktype;
	bool_t reclaim;
	offset4 offset;
	length4 length;
	locker4 locker;
};
typedef struct LOCK4args LOCK4args;

struct LOCK4denied {
	offset4 offset;
	length4 length;
	nfs_lock_type4 locktype;
	lock_owner4 owner;
};
typedef struct LOCK4denied LOCK4denied;

struct LOCK4res {
	nfsstat4 status;
	union {
		stateid4 lock_stateid;
		LOCK4denied denied;
	} LOCK4res_u;
};
typedef struct LOCK4res LOCK4res;

struct LOCKT4args {
	nfs_lock_type4 locktype;
	offset4 offset;
	length4 length;
	lock_owner4 owner;
};
typedef struct LOCKT4args LOCKT4args;

struct LOCKT4res {
	nfsstat4 status;
	LOCK4denied denied;
};
typedef struct LOCKT4res LOCKT4res;

struct LOCKU4args {
	nfs_lock_type4 locktype;
	seqid4 seqid;
	stateid4 lock_stateid;
	offset4 offset;
	length4 length;
};
typedef struct LOCKU4args LOCKU4args;

struct LOCKU4res {
	nfsstat4 status;
	stateid4 lock_stateid;
};
typedef struct LOCKU4res LOCKU4res;

struct LOOKUP4args {
	component4 objname;
};
typedef struct LOOKUP4args LOOKUP4args;

struct LOOKUP4cargs {
	char *cname;
};
typedef struct LOOKUP4cargs LOOKUP4cargs;

struct LOOKUP4res {
	nfsstat4 status;
};
typedef struct LOOKUP4res LOOKUP4res;

struct LOOKUPP4res {
	nfsstat4 status;
};
typedef struct LOOKUPP4res LOOKUPP4res;

struct NVERIFY4args {
	fattr4 obj_attributes;
};
typedef struct NVERIFY4args NVERIFY4args;

struct NVERIFY4res {
	nfsstat4 status;
};
typedef struct NVERIFY4res NVERIFY4res;

enum createmode4 {
	UNCHECKED4 = 0,
	GUARDED4 = 1,
	EXCLUSIVE4 = 2,
	EXCLUSIVE4_1 = 3	/* Added in NFSv4.1 */
};
typedef enum createmode4 createmode4;

enum opentype4 {
	OPEN4_NOCREATE = 0,
	OPEN4_CREATE = 1
};
typedef enum opentype4 opentype4;

enum limit_by4 {
	NFS_LIMIT_SIZE = 1,
	NFS_LIMIT_BLOCKS = 2
};
typedef enum limit_by4 limit_by4;

struct nfs_modified_limit4 {
	uint32_t num_blocks;
	uint32_t bytes_per_block;
};
typedef struct nfs_modified_limit4 nfs_modified_limit4;

struct nfs_space_limit4 {
	limit_by4 limitby;
	union {
		uint64_t filesize;
		nfs_modified_limit4 mod_blocks;
	} nfs_space_limit4_u;
};
typedef struct nfs_space_limit4 nfs_space_limit4;
#define	OPEN4_SHARE_ACCESS_READ 0x00000001
#define	OPEN4_SHARE_ACCESS_WRITE 0x00000002
#define	OPEN4_SHARE_ACCESS_BOTH 0x00000003
#define	OPEN4_SHARE_DENY_NONE 0x00000000
#define	OPEN4_SHARE_DENY_READ 0x00000001
#define	OPEN4_SHARE_DENY_WRITE 0x00000002
#define	OPEN4_SHARE_DENY_BOTH 0x00000003

/*
 * XXX - open_delegation_type4 and open_claim_type4 were
 *	 extended by NFSv4.1. These are now defined in
 *	 nfs4_ext_defs.h
 */

struct open_claim_delegate_cur4 {
	stateid4 delegate_stateid;
	component4 file;
};
typedef struct open_claim_delegate_cur4 open_claim_delegate_cur4;

struct copen_claim_delegate_cur4 {
	stateid4 delegate_stateid;
	char *cfile;
};
typedef struct copen_claim_delegate_cur4 copen_claim_delegate_cur4;

struct open_claim4 {
	open_claim_type4 claim;
	union {
		component4 file;
		open_delegation_type4 delegate_type;
		open_claim_delegate_cur4 delegate_cur_info;
		component4 file_delegate_prev;
		stateid4 oc_delegate_stateid;
	} open_claim4_u;
};
typedef struct open_claim4 open_claim4;

struct creatverfattr {
	verifier4 cva_verf;
	fattr4 cva_attrs;
};
typedef struct creatverfattr creatverfattr;

struct OPEN4args {
	seqid4 seqid;
	uint32_t share_access;
	uint32_t share_deny;
	open_owner4 owner;
	opentype4 opentype;
	createmode4 mode;
	union {
		fattr4 createattrs;
		verifier4 createverf;
		creatverfattr ch_createboth;
	} createhow4_u;
	open_claim_type4 claim;
	union {
		component4 file;
		open_delegation_type4 delegate_type;
		open_claim_delegate_cur4 delegate_cur_info;
		component4 file_delegate_prev;
		stateid4 oc_delegate_stateid;
	} open_claim4_u;
};
typedef struct OPEN4args OPEN4args;

struct OPEN4cargs {
	seqid4 seqid;
	uint32_t share_access;
	uint32_t share_deny;
	open_owner4 owner;
	opentype4 opentype;
	createmode4 mode;
	union {
		fattr4 createattrs;
		verifier4 createverf;
		creatverfattr ch_createboth;
	} createhow4_u;
	open_claim_type4 claim;
	union {
		char *cfile;
		open_delegation_type4 delegate_type;
		copen_claim_delegate_cur4 delegate_cur_info;
		char *cfile_delegate_prev;
		stateid4 coc_delegate_stateid;
	} open_claim4_u;
};
typedef struct OPEN4cargs OPEN4cargs;

struct open_read_delegation4 {
	stateid4 stateid;
	bool_t recall;
	nfsace4 permissions;
};
typedef struct open_read_delegation4 open_read_delegation4;

struct open_write_delegation4 {
	stateid4 stateid;
	bool_t recall;
	nfs_space_limit4 space_limit;
	nfsace4 permissions;
};
typedef struct open_write_delegation4 open_write_delegation4;

/*
 * XXX - We inline the NFSv4.1 modifications to open_delegation4
 *	 here since we need the two types above in order to craft
 *	 the open_delegation4 type. open_none_delegation4 is a new
 *	 type, but we've already included it in nfs4_ext_defs.h
 */
struct open_delegation4 {
	open_delegation_type4 delegation_type;
	union {
		open_read_delegation4 read;
		open_write_delegation4 write;
		open_none_delegation4 od_whynone;	/* XXX - NFSv4.1 */
	} open_delegation4_u;
};
typedef struct open_delegation4 open_delegation4;
#define	OPEN4_RESULT_CONFIRM 0x00000002
#define	OPEN4_RESULT_LOCKTYPE_POSIX 0x00000004
#define	OPEN4_RESULT_PRESERVE_UNLINKED 0x00000008	/* XXX - NFSv4.1 */
#define	OPEN4_RESULT_MAY_NOTIFY_LOCK 0x00000020		/* XXX - NFSv4.1 */

struct OPEN4res {
	nfsstat4 status;
	stateid4 stateid;
	change_info4 cinfo;
	uint32_t rflags;
	attrmap4 attrset;
	open_delegation4 delegation;
};
typedef struct OPEN4res OPEN4res;

struct OPENATTR4args {
	bool_t createdir;
};
typedef struct OPENATTR4args OPENATTR4args;

struct OPENATTR4res {
	nfsstat4 status;
};
typedef struct OPENATTR4res OPENATTR4res;

struct OPEN_CONFIRM4args {
	stateid4 open_stateid;
	seqid4 seqid;
};
typedef struct OPEN_CONFIRM4args OPEN_CONFIRM4args;

struct OPEN_CONFIRM4res {
	nfsstat4 status;
	stateid4 open_stateid;
};
typedef struct OPEN_CONFIRM4res OPEN_CONFIRM4res;

struct OPEN_DOWNGRADE4args {
	stateid4 open_stateid;
	seqid4 seqid;
	uint32_t share_access;
	uint32_t share_deny;
};
typedef struct OPEN_DOWNGRADE4args OPEN_DOWNGRADE4args;

struct OPEN_DOWNGRADE4res {
	nfsstat4 status;
	stateid4 open_stateid;
};
typedef struct OPEN_DOWNGRADE4res OPEN_DOWNGRADE4res;

struct PUTFH4args {
	nfs_fh4 object;
};
typedef struct PUTFH4args PUTFH4args;

/*
 * Client only side PUTFH arguments
 * This is really a nfs4_sharedfh_t * but the forward declaration
 * is problematic;
 */
struct PUTFH4cargs {
	void *sfh;
};
typedef struct PUTFH4cargs PUTFH4cargs;

struct PUTFH4res {
	nfsstat4 status;
};
typedef struct PUTFH4res PUTFH4res;

struct PUTPUBFH4res {
	nfsstat4 status;
};
typedef struct PUTPUBFH4res PUTPUBFH4res;

struct PUTROOTFH4res {
	nfsstat4 status;
};
typedef struct PUTROOTFH4res PUTROOTFH4res;

struct READ4args {
	stateid4 stateid;
	offset4 offset;
	count4 count;
	/* The following are used for the XDR decode path */
	char *res_data_val_alt;
	mblk_t *res_mblk;
	struct uio *res_uiop;
	uint_t res_maxsize;

};
typedef struct READ4args READ4args;

struct READ4res {
	nfsstat4 status;
	bool_t eof;
	uint_t data_len;
	char *data_val;
	mblk_t *mblk;
};
typedef struct READ4res READ4res;

struct rddir4_cache;

struct READDIR4args {
	nfs_cookie4 cookie;
	verifier4 cookieverf;
	count4 dircount;
	count4 maxcount;
	attrmap4 attr_request;
	vnode_t *dvp;
	struct mntinfo4 *mi;
	cred_t *cr;
	struct rddir4_cache *rdc;
	hrtime_t t;
};
typedef struct READDIR4args READDIR4args;

struct READDIR4res_clnt {
	nfsstat4 status;
	verifier4 cookieverf;
	bool_t eof;
	struct dirent64 *dotp, *dotdotp;
	struct rddir4_cache *rdc;
};
typedef struct READDIR4res_clnt READDIR4res_clnt;

struct READDIR4res {
	nfsstat4 status;
	verifier4 cookieverf;
	mblk_t *mblk;
	uint_t data_len;
};
typedef struct READDIR4res READDIR4res;

struct READLINK4res {
	nfsstat4 status;
	linktext4 link;
};
typedef struct READLINK4res READLINK4res;

struct REMOVE4args {
	component4 target;
};
typedef struct REMOVE4args REMOVE4args;

struct REMOVE4cargs {
	char *ctarget;
};
typedef struct REMOVE4cargs REMOVE4cargs;

struct REMOVE4res {
	nfsstat4 status;
	change_info4 cinfo;
};
typedef struct REMOVE4res REMOVE4res;

struct RENAME4args {
	component4 oldname;
	component4 newname;
};
typedef struct RENAME4args RENAME4args;

struct RENAME4cargs {
	char *coldname;
	char *cnewname;
};
typedef struct RENAME4cargs RENAME4cargs;

struct RENAME4res {
	nfsstat4 status;
	change_info4 source_cinfo;
	change_info4 target_cinfo;
};
typedef struct RENAME4res RENAME4res;

struct RENEW4args {
	clientid4 clientid;
};
typedef struct RENEW4args RENEW4args;

struct RENEW4res {
	nfsstat4 status;
};
typedef struct RENEW4res RENEW4res;

struct RESTOREFH4res {
	nfsstat4 status;
};
typedef struct RESTOREFH4res RESTOREFH4res;

struct SAVEFH4res {
	nfsstat4 status;
};
typedef struct SAVEFH4res SAVEFH4res;

struct SECINFO4args {
	component4 name;
};
typedef struct SECINFO4args SECINFO4args;

struct SECINFO4cargs {
	char *cname;
};
typedef struct SECINFO4cargs SECINFO4cargs;

enum rpc_gss_svc_t {
	RPC_GSS_SVC_NONE = 1,
	RPC_GSS_SVC_INTEGRITY = 2,
	RPC_GSS_SVC_PRIVACY = 3
};
typedef enum rpc_gss_svc_t rpc_gss_svc_t;

struct rpcsec_gss_info {
	sec_oid4 oid;
	qop4 qop;
	rpc_gss_svc_t service;
};
typedef struct rpcsec_gss_info rpcsec_gss_info;

struct secinfo4 {
	uint32_t flavor;
	rpcsec_gss_info flavor_info;
};
typedef struct secinfo4 secinfo4;

struct SECINFO4res {
	nfsstat4 status;
	uint_t SECINFO4resok_len;
	secinfo4 *SECINFO4resok_val;
};
typedef struct SECINFO4res SECINFO4res;

struct SETATTR4args {
	stateid4 stateid;
	fattr4 obj_attributes;
};
typedef struct SETATTR4args SETATTR4args;

struct SETATTR4res {
	nfsstat4 status;
	attrmap4 attrsset;
};
typedef struct SETATTR4res SETATTR4res;

struct SETCLIENTID4args {
	nfs_client_id4 client;
	cb_client4 callback;
	uint32_t callback_ident;
};
typedef struct SETCLIENTID4args SETCLIENTID4args;

struct SETCLIENTID4resok {
	clientid4 clientid;
	verifier4 setclientid_confirm;
};
typedef struct SETCLIENTID4resok SETCLIENTID4resok;

struct SETCLIENTID4res {
	nfsstat4 status;
	union {
		SETCLIENTID4resok resok4;
		clientaddr4 client_using;
	} SETCLIENTID4res_u;
};
typedef struct SETCLIENTID4res SETCLIENTID4res;

struct SETCLIENTID_CONFIRM4args {
	clientid4 clientid;
	verifier4 setclientid_confirm;
};
typedef struct SETCLIENTID_CONFIRM4args SETCLIENTID_CONFIRM4args;

struct SETCLIENTID_CONFIRM4res {
	nfsstat4 status;
};
typedef struct SETCLIENTID_CONFIRM4res SETCLIENTID_CONFIRM4res;

struct VERIFY4args {
	fattr4 obj_attributes;
};
typedef struct VERIFY4args VERIFY4args;

struct VERIFY4res {
	nfsstat4 status;
};
typedef struct VERIFY4res VERIFY4res;

enum stable_how4 {
	UNSTABLE4 = 0,
	DATA_SYNC4 = 1,
	FILE_SYNC4 = 2
};
typedef enum stable_how4 stable_how4;

/*
 * mblk doesn't go over the wire.  If non-NULL, it points to an mblk chain
 * for the write data.
 */

struct WRITE4args {
	stateid4 stateid;
	offset4 offset;
	stable_how4 stable;
	uint_t data_len;
	char *data_val;
	mblk_t *mblk;
};
typedef struct WRITE4args WRITE4args;

struct WRITE4res {
	nfsstat4 status;
	count4 count;
	stable_how4 committed;
	verifier4 writeverf;
};
typedef struct WRITE4res WRITE4res;

struct RELEASE_LOCKOWNER4args {
	lock_owner4 lock_owner;
};
typedef struct RELEASE_LOCKOWNER4args RELEASE_LOCKOWNER4args;

struct RELEASE_LOCKOWNER4res {
	nfsstat4 status;
};
typedef struct RELEASE_LOCKOWNER4res RELEASE_LOCKOWNER4res;

struct ILLEGAL4res {
	nfsstat4 status;
};
typedef struct ILLEGAL4res ILLEGAL4res;

/*
 * XXX - nfs_opnum4, nfs_argop4, and nfs_resop4 were all
 *	 extended by NFSv4.1 ; These are now defined in
 *	 nfs41_kprot.h
 */

/*
 * Fixed size tag string for easy client encoding
 */
struct _ctag {
	int ct_type;
	char *ct_str;
	uint32_t ct_tag[3];
};
typedef struct _ctag ctag_t;

/*
 * Client-only encode-only version
 */
struct CB_GETATTR4args {
	nfs_fh4 fh;
	attrmap4 attr_request;
};
typedef struct CB_GETATTR4args CB_GETATTR4args;

struct CB_GETATTR4res {
	nfsstat4 status;
	fattr4 obj_attributes;
};
typedef struct CB_GETATTR4res CB_GETATTR4res;

struct CB_RECALL4args {
	stateid4 stateid;
	bool_t truncate;
	nfs_fh4 fh;
};
typedef struct CB_RECALL4args CB_RECALL4args;

struct CB_RECALL4res {
	nfsstat4 status;
};
typedef struct CB_RECALL4res CB_RECALL4res;

struct CB_ILLEGAL4res {
	nfsstat4 status;
};
typedef struct CB_ILLEGAL4res CB_ILLEGAL4res;

/* XXX - NFSv4.1 protocol defs */
#include <nfs/nfs41_kprot.h>

struct COMPOUND4args_clnt {
	int ctag;
	uint32_t minor_vers;
	uint_t array_len;
	nfs_argop4 *array;
};
typedef struct COMPOUND4args_clnt COMPOUND4args_clnt;

struct COMPOUND4args {
	utf8string tag;
	uint32_t minorversion;
	uint_t array_len;
	nfs_argop4 *array;
};
typedef struct COMPOUND4args COMPOUND4args;

struct COMPOUND4res_clnt {
	nfsstat4 status;
	uint_t array_len;
	uint_t decode_len;
	nfs_resop4 *array;
	COMPOUND4args_clnt *argsp;
};
typedef struct COMPOUND4res_clnt COMPOUND4res_clnt;

struct COMPOUND4res {
	nfsstat4 status;
	utf8string tag;
	uint_t array_len;
	nfs_resop4 *array;
};
typedef struct COMPOUND4res COMPOUND4res;

struct COMPOUND4res_srv {
	nfsstat4 status;
	utf8string tag;
	uint_t array_len;
	nfs_resop4 *array;
	uint32_t minorversion;	/* won't go otw */
};
typedef struct COMPOUND4res_srv COMPOUND4res_srv;

/*
 * XXX - nfs_cb_opnum4, nfs_cb_argop4, and nfs_cb_resop4 were
 *	 all extended by NFSv4.1 ; These are now defined in
 *	 nfs41_kprot.h
 */

struct CB_COMPOUND4args {
	utf8string tag;
	uint32_t minorversion;
	uint32_t callback_ident;
	uint_t array_len;
	nfs_cb_argop4 *array;
};
typedef struct CB_COMPOUND4args CB_COMPOUND4args;

struct CB_COMPOUND4res {
	nfsstat4 status;
	utf8string tag;
	uint_t array_len;
	nfs_cb_resop4 *array;
};
typedef struct CB_COMPOUND4res CB_COMPOUND4res;

#define	NFS4_PROGRAM		100003
#define	NFS_V4			4
#define	NFSPROC4_NULL		0
#define	NFSPROC4_COMPOUND	1

#define	NFS4_CALLBACK		0x40000000
#define	NFS_CB			1
#define	CB_NULL			0
#define	CB_COMPOUND		1

extern  bool_t xdr_bitmap4(XDR *, bitmap4 *);
extern  bool_t xdr_attrmap4(XDR *, attrmap4 *);
extern  bool_t xdr_utf8string(XDR *, utf8string *);
extern  bool_t xdr_nfs_fh4(XDR *, nfs_fh4 *);
extern  bool_t xdr_nfs_fh4_modified(XDR *, nfs_fh4 *);
extern  bool_t xdr_fattr4_fsid(XDR *, fattr4_fsid *);
extern  bool_t xdr_fattr4_acl(XDR *, fattr4_acl *);
extern  bool_t xdr_fattr4_fs_locations(XDR *, fattr4_fs_locations *);
extern  bool_t xdr_fattr4_rawdev(XDR *, fattr4_rawdev *);
extern  bool_t xdr_nfstime4(XDR *, nfstime4 *);
extern  bool_t xdr_settime4(XDR *, settime4 *);
extern	bool_t xdr_open_claim4(XDR *, open_claim4 *);
extern  bool_t xdr_COMPOUND4args_clnt(XDR *, COMPOUND4args_clnt *);
extern  bool_t xdr_COMPOUND4args_srv(XDR *, COMPOUND4args_srv *);
extern  bool_t xdr_COMPOUND4res_clnt(XDR *, COMPOUND4res_clnt *);
extern  bool_t xdr_COMPOUND4res_srv(XDR *, COMPOUND4res_srv *);
extern  bool_t xdr_CB_COMPOUND4args_clnt(XDR *, CB_COMPOUND4args *);
extern  bool_t xdr_CB_COMPOUND4args_srv(XDR *, CB_COMPOUND4args *);
extern  bool_t xdr_CB_COMPOUND4res(XDR *, CB_COMPOUND4res *);

#ifdef __cplusplus
}
#endif

#endif /* _NFS4_KPROT_H */
