/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License, Version 1.0 only
 * (the "License").  You may not use this file except in compliance
 * with the License.
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
 * Copyright 2005 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include "libzfs_jni_pool.h"
#include "libzfs_jni_util.h"
#include <strings.h>

/*
 * Types
 */

typedef struct ImportablePoolBean {
	zjni_Object_t super;

	jmethodID method_setName;
	jmethodID method_setId;
	jmethodID method_setState;
	jmethodID method_setHealth;
} ImportablePoolBean_t;

typedef struct VirtualDeviceBean {
	zjni_Object_t super;

	jmethodID method_setPoolName;
	jmethodID method_setIndex;
	jmethodID method_setSize;
	jmethodID method_setUsed;
} VirtualDeviceBean_t;

typedef struct DiskVirtualDeviceBean {
	VirtualDeviceBean_t super;

	jmethodID method_setDiskName;
} DiskVirtualDeviceBean_t;

typedef struct FileVirtualDeviceBean {
	VirtualDeviceBean_t super;

	jmethodID method_setFileName;
} FileVirtualDeviceBean_t;

typedef struct RAIDVirtualDeviceBean {
	VirtualDeviceBean_t super;
} RAIDVirtualDeviceBean_t;

typedef struct MirrorVirtualDeviceBean {
	VirtualDeviceBean_t super;
} MirrorVirtualDeviceBean_t;

/*
 * Function prototypes
 */

static void new_ImportablePoolBean(JNIEnv *, ImportablePoolBean_t *);
static void new_VirtualDevice(JNIEnv *, VirtualDeviceBean_t *);
static void new_DiskVirtualDeviceBean(JNIEnv *, DiskVirtualDeviceBean_t *);
static void new_FileVirtualDeviceBean(JNIEnv *, FileVirtualDeviceBean_t *);
static void new_RAIDVirtualDeviceBean(JNIEnv *, RAIDVirtualDeviceBean_t *);
static void new_MirrorVirtualDeviceBean(JNIEnv *, MirrorVirtualDeviceBean_t *);
static jobject uint64_to_state(JNIEnv *, uint64_t);
static int populate_ImportablePoolBean(
    JNIEnv *, ImportablePoolBean_t *, char *, uint64_t, uint64_t, char *);
static int populate_VirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *, VirtualDeviceBean_t *);
static int populate_DiskVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *, DiskVirtualDeviceBean_t *);
static int populate_FileVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *, FileVirtualDeviceBean_t *);
static int populate_RAIDVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *, RAIDVirtualDeviceBean_t *);
static int populate_MirrorVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *, MirrorVirtualDeviceBean_t *);
static jobject create_ImportablePoolBean(
    JNIEnv *, char *, uint64_t, uint64_t, char *);
static jobject create_DiskVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *);
static jobject create_FileVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *);
static jobject create_RAIDVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *);
static jobject create_MirrorVirtualDeviceBean(
    JNIEnv *, zpool_handle_t *, nvlist_t *);

/*
 * Static functions
 */

/* Create a ImportablePoolBean */
static void
new_ImportablePoolBean(JNIEnv *env, ImportablePoolBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {
		object->class =
		    (*env)->FindClass(env,
			ZFSJNI_PACKAGE_DATA "ImportablePoolBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	bean->method_setName = (*env)->GetMethodID(
	    env, object->class, "setName", "(Ljava/lang/String;)V");

	bean->method_setId = (*env)->GetMethodID(
	    env, object->class, "setId", "(J)V");

	bean->method_setState = (*env)->GetMethodID(
	    env, object->class, "setState",
	    "(L" ZFSJNI_PACKAGE_DATA "ImportablePool$State;)V");

	bean->method_setHealth = (*env)->GetMethodID(
	    env, object->class, "setHealth", "(Ljava/lang/String;)V");
}

/* Create a VirtualDeviceBean */
static void
new_VirtualDevice(JNIEnv *env, VirtualDeviceBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {
		object->class =
		    (*env)->FindClass(env,
			ZFSJNI_PACKAGE_DATA "VirtualDeviceBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	bean->method_setPoolName = (*env)->GetMethodID(
	    env, object->class, "setPoolName", "(Ljava/lang/String;)V");

	bean->method_setIndex = (*env)->GetMethodID(
	    env, object->class, "setIndex", "(J)V");

	bean->method_setSize = (*env)->GetMethodID(
	    env, object->class, "setSize", "(J)V");

	bean->method_setUsed = (*env)->GetMethodID(
	    env, object->class, "setUsed", "(J)V");
}

/* Create a DiskVirtualDeviceBean */
static void
new_DiskVirtualDeviceBean(JNIEnv *env, DiskVirtualDeviceBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {
		object->class = (*env)->FindClass(
		    env, ZFSJNI_PACKAGE_DATA "DiskVirtualDeviceBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	new_VirtualDevice(env, (VirtualDeviceBean_t *)bean);

	bean->method_setDiskName = (*env)->GetMethodID(
	    env, object->class, "setDiskName", "(Ljava/lang/String;)V");

}

/* Create a FileVirtualDeviceBean */
static void
new_FileVirtualDeviceBean(JNIEnv *env, FileVirtualDeviceBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {
		object->class = (*env)->FindClass(
		    env, ZFSJNI_PACKAGE_DATA "FileVirtualDeviceBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	new_VirtualDevice(env, (VirtualDeviceBean_t *)bean);

	bean->method_setFileName = (*env)->GetMethodID(
	    env, object->class, "setFileName", "(Ljava/lang/String;)V");
}

/* Create a RAIDVirtualDeviceBean */
static void
new_RAIDVirtualDeviceBean(JNIEnv *env, RAIDVirtualDeviceBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {

		object->class = (*env)->FindClass(
		    env, ZFSJNI_PACKAGE_DATA "RAIDVirtualDeviceBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	new_VirtualDevice(env, (VirtualDeviceBean_t *)bean);
}

/* Create a MirrorVirtualDeviceBean */
static void
new_MirrorVirtualDeviceBean(JNIEnv *env, MirrorVirtualDeviceBean_t *bean)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	if (object->object == NULL) {
		object->class = (*env)->FindClass(
		    env, ZFSJNI_PACKAGE_DATA "MirrorVirtualDeviceBean");

		object->constructor =
		    (*env)->GetMethodID(env, object->class, "<init>", "()V");

		object->object =
		    (*env)->NewObject(env, object->class, object->constructor);
	}

	new_VirtualDevice(env, (VirtualDeviceBean_t *)bean);
}

static jobject
uint64_to_state(JNIEnv *env, uint64_t pool_state)
{
	jobject state_obj;

	jclass class_State = (*env)->FindClass(
	    env, ZFSJNI_PACKAGE_DATA "ImportablePool$State");

	jmethodID method_valueOf = (*env)->GetStaticMethodID(
	    env, class_State, "valueOf",
	    "(Ljava/lang/String;)L"
	    ZFSJNI_PACKAGE_DATA "ImportablePool$State;");

	char *str = zjni_get_state_str(pool_state);
	if (str != NULL) {
	    jstring utf = (*env)->NewStringUTF(env, str);

	    state_obj = (*env)->CallStaticObjectMethod(
		env, class_State, method_valueOf, utf);
	}

	return (state_obj);
}

static int
populate_ImportablePoolBean(JNIEnv *env, ImportablePoolBean_t *bean,
    char *name, uint64_t guid, uint64_t pool_state, char *health)
{
	zjni_Object_t *object = (zjni_Object_t *)bean;

	/* Set name */
	(*env)->CallVoidMethod(env, object->object, bean->method_setName,
	    (*env)->NewStringUTF(env, name));

	/* Set state */
	(*env)->CallVoidMethod(
	    env, object->object, bean->method_setState,
	    uint64_to_state(env, pool_state));

	/* Set guid */
	(*env)->CallVoidMethod(
	    env, object->object, bean->method_setId, (jlong)guid);

	/* Set health */
	(*env)->CallVoidMethod(env, object->object, bean->method_setHealth,
	    (*env)->NewStringUTF(env, health));

	return (0);
}

static int
populate_VirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev, VirtualDeviceBean_t *bean)
{
	int result;
	uint64_t vdev_id;
	zjni_Object_t *object = (zjni_Object_t *)bean;

	/* Set pool name */
	jstring poolUTF = (*env)->NewStringUTF(env, zpool_get_name(zhp));
	(*env)->CallVoidMethod(
	    env, object->object, bean->method_setPoolName, poolUTF);

	/* Get index */
	result = nvlist_lookup_uint64(vdev, ZPOOL_CONFIG_GUID, &vdev_id);
	if (result != 0) {
		zjni_throw_exception(env,
		    "could not retrieve virtual device ID (pool %s)",
		    zpool_get_name(zhp));
	} else {

		uint64_t used;
		uint64_t total;

		(*env)->CallVoidMethod(
		    env, object->object, bean->method_setIndex, (jlong)vdev_id);

		/* Set used space */
		used = zpool_get_space_used(zhp);

		(*env)->CallVoidMethod(
		    env, object->object, bean->method_setUsed, (jlong)used);

		/* Set available space */
		total = zpool_get_space_total(zhp);

		(*env)->CallVoidMethod(
		    env, object->object, bean->method_setSize, (jlong)total);
	}

	return (result != 0);
}

static int
populate_DiskVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev, DiskVirtualDeviceBean_t *bean)
{
	char *path;
	int result = populate_VirtualDeviceBean(
	    env, zhp, vdev, (VirtualDeviceBean_t *)bean);

	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (-1);
	}

	/* Set path */
	result = nvlist_lookup_string(vdev, ZPOOL_CONFIG_PATH, &path);
	if (result != 0) {
		zjni_throw_exception(env,
		    "could not retrive path from disk virtual device (pool %s)",
		    zpool_get_name(zhp));
	} else {

		jstring pathUTF = (*env)->NewStringUTF(env, path);
		(*env)->CallVoidMethod(env, ((zjni_Object_t *)bean)->object,
		    bean->method_setDiskName, pathUTF);
	}

	return (result != 0);
}

static int
populate_FileVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev, FileVirtualDeviceBean_t *bean)
{
	char *path;
	int result = populate_VirtualDeviceBean(
	    env, zhp, vdev, (VirtualDeviceBean_t *)bean);

	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (-1);
	}

	/* Set path */
	result = nvlist_lookup_string(vdev, ZPOOL_CONFIG_PATH, &path);
	if (result != 0) {
		zjni_throw_exception(env,
		    "could not retrive path from disk virtual device (pool %s)",
		    zpool_get_name(zhp));
	} else {

		jstring pathUTF = (*env)->NewStringUTF(env, path);
		(*env)->CallVoidMethod(env, ((zjni_Object_t *)bean)->object,
		    bean->method_setFileName, pathUTF);
	}

	return (result != 0);
}

static int
populate_RAIDVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev, RAIDVirtualDeviceBean_t *bean)
{
	return (populate_VirtualDeviceBean(env, zhp, vdev,
	    (VirtualDeviceBean_t *)bean));
}

static int
populate_MirrorVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev, MirrorVirtualDeviceBean_t *bean)
{
	return (populate_VirtualDeviceBean(env, zhp, vdev,
	    (VirtualDeviceBean_t *)bean));
}

static jobject
create_ImportablePoolBean(JNIEnv *env, char *name,
    uint64_t guid, uint64_t pool_state, char *health)
{
	int result;
	ImportablePoolBean_t bean_obj = {0};
	ImportablePoolBean_t *bean = &bean_obj;

	/* Construct ImportablePoolBean */
	new_ImportablePoolBean(env, bean);

	result = populate_ImportablePoolBean(
	    env, bean, name, guid, pool_state, health);
	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (NULL);
	}

	return (((zjni_Object_t *)bean)->object);
}

static jobject
create_DiskVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp, nvlist_t *vdev)
{
	int result;
	DiskVirtualDeviceBean_t bean_obj = {0};
	DiskVirtualDeviceBean_t *bean = &bean_obj;

	/* Construct DiskVirtualDeviceBean */
	new_DiskVirtualDeviceBean(env, bean);

	result = populate_DiskVirtualDeviceBean(env, zhp, vdev, bean);
	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (NULL);
	}

	return (((zjni_Object_t *)bean)->object);
}

static jobject
create_FileVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp, nvlist_t *vdev)
{
	int result;
	FileVirtualDeviceBean_t bean_obj = {0};
	FileVirtualDeviceBean_t *bean = &bean_obj;

	/* Construct FileVirtualDeviceBean */
	new_FileVirtualDeviceBean(env, bean);

	result = populate_FileVirtualDeviceBean(env, zhp, vdev, bean);
	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (NULL);
	}

	return (((zjni_Object_t *)bean)->object);
}

static jobject
create_RAIDVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp, nvlist_t *vdev)
{
	int result;
	RAIDVirtualDeviceBean_t bean_obj = {0};
	RAIDVirtualDeviceBean_t *bean = &bean_obj;

	((zjni_Object_t *)bean)->object = NULL;

	/* Construct RAIDVirtualDeviceBean */
	new_RAIDVirtualDeviceBean(env, bean);

	result = populate_RAIDVirtualDeviceBean(env, zhp, vdev, bean);
	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (NULL);
	}

	return (((zjni_Object_t *)bean)->object);
}

static jobject
create_MirrorVirtualDeviceBean(JNIEnv *env, zpool_handle_t *zhp, nvlist_t *vdev)
{
	int result;
	MirrorVirtualDeviceBean_t bean_obj = {0};
	MirrorVirtualDeviceBean_t *bean = &bean_obj;

	/* Construct MirrorVirtualDeviceBean */
	new_MirrorVirtualDeviceBean(env, bean);

	result = populate_MirrorVirtualDeviceBean(env, zhp, vdev, bean);
	if (result) {
		/* Must not call any more Java methods to preserve exception */
		return (NULL);
	}

	return (((zjni_Object_t *)bean)->object);
}

/*
 * Package-private functions
 */

/*
 * Gets the root vdev (an nvlist_t *) for the given pool.
 */
nvlist_t *
zjni_get_root_vdev(zpool_handle_t *zhp)
{
	nvlist_t *root = NULL;

	if (zhp != NULL) {
		nvlist_t *attrs = zpool_get_config(zhp, NULL);

		if (attrs != NULL) {
			int result = nvlist_lookup_nvlist(
			    attrs, ZPOOL_CONFIG_VDEV_TREE, &root);
			if (result != 0) {
				root = NULL;
			}
		}
	}

	return (root);
}

/*
 * Gets the vdev (an nvlist_t *) with the given vdev_id, below the
 * given vdev.  If the given vdev is NULL, all vdevs within the given
 * pool are searched.
 */
nvlist_t *
zjni_get_vdev(zpool_handle_t *zhp, nvlist_t *vdev_parent,
    uint64_t vdev_id_to_find)
{
	int result;

	/* Was a vdev specified? */
	if (vdev_parent == NULL) {
		/* No -- retrieve the top-level pool vdev */
		vdev_parent = zjni_get_root_vdev(zhp);
	} else {
		/* Get index of this vdev and compare with vdev_id_to_find */
		uint64_t id;
		result = nvlist_lookup_uint64(
		    vdev_parent, ZPOOL_CONFIG_GUID, &id);
		if (result == 0 && id == vdev_id_to_find) {
			return (vdev_parent);
		}
	}

	if (vdev_parent != NULL) {

		nvlist_t **children;
		uint_t nelem = 0;

		/* Get the vdevs under this vdev */
		result = nvlist_lookup_nvlist_array(
		    vdev_parent, ZPOOL_CONFIG_CHILDREN, &children, &nelem);

		if (result == 0) {

			int i;
			nvlist_t *child;

			/* For each vdev child... */
			for (i = 0; i < nelem; i++) {
				child = zjni_get_vdev(zhp, children[i],
				    vdev_id_to_find);
				if (child != NULL) {
					return (child);
				}
			}
		}
	}

	return (NULL);
}

jobject
zjni_get_VirtualDevice_from_vdev(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev)
{
	jobject obj = NULL;
	char *type = NULL;
	int result = nvlist_lookup_string(vdev, ZPOOL_CONFIG_TYPE, &type);

	if (result == 0) {
		if (strcmp(type, VDEV_TYPE_DISK) == 0) {
			obj = create_DiskVirtualDeviceBean(env, zhp, vdev);
		} else if (strcmp(type, VDEV_TYPE_FILE) == 0) {
			obj = create_FileVirtualDeviceBean(env, zhp, vdev);
		} else if (strcmp(type, VDEV_TYPE_RAIDZ) == 0) {
			obj = create_RAIDVirtualDeviceBean(env, zhp, vdev);
		} else if (strcmp(type, VDEV_TYPE_MIRROR) == 0) {
			obj = create_MirrorVirtualDeviceBean(env, zhp, vdev);
		} else if (strcmp(type, VDEV_TYPE_REPLACING) == 0) {

			/* Get the vdevs under this vdev */
			nvlist_t **children;
			uint_t nelem = 0;
			int result = nvlist_lookup_nvlist_array(
			    vdev, ZPOOL_CONFIG_CHILDREN, &children, &nelem);

			if (result == 0 && nelem > 0) {

				/* Get last vdev child (replacement device) */
				nvlist_t *child = children[nelem - 1];

				obj = zjni_get_VirtualDevice_from_vdev(env,
				    zhp, child);
			}
		}
	}

	return (obj);
}

jobject
zjni_get_VirtualDevices_from_vdev(JNIEnv *env, zpool_handle_t *zhp,
    nvlist_t *vdev_parent)
{
	/* Create an array list for the vdevs */
	zjni_ArrayList_t list_class = {0};
	zjni_ArrayList_t *list_class_p = &list_class;
	zjni_new_ArrayList(env, list_class_p);

	/* Was a vdev specified? */
	if (vdev_parent == NULL) {
		/* No -- retrieve the top-level pool vdev */
		vdev_parent = zjni_get_root_vdev(zhp);
	}

	if (vdev_parent != NULL) {

		/* Get the vdevs under this vdev */
		nvlist_t **children;
		uint_t nelem = 0;
		int result = nvlist_lookup_nvlist_array(
		    vdev_parent, ZPOOL_CONFIG_CHILDREN, &children, &nelem);

		if (result == 0) {

			/* For each vdev child... */
			int i;
			for (i = 0; i < nelem; i++) {
				nvlist_t *child = children[i];

				/* Create a Java object from this vdev */
				jobject obj =
				    zjni_get_VirtualDevice_from_vdev(env,
					zhp, child);

				if ((*env)->ExceptionOccurred(env) != NULL) {
					/*
					 * Must not call any more Java methods
					 * to preserve exception
					 */
					return (NULL);
				}

				if (obj != NULL) {
				    /* Add child to child vdev list */
				    (*env)->CallBooleanMethod(env,
					((zjni_Object_t *)list_class_p)->object,
					((zjni_Collection_t *)list_class_p)->
					method_add, obj);
				}
			}
		}
	}

	return (zjni_Collection_to_array(
	    env, (zjni_Collection_t *)list_class_p,
	    ZFSJNI_PACKAGE_DATA "VirtualDevice"));
}

int
zjni_create_add_ImportablePool(char *name,
    uint64_t guid, uint64_t pool_state, char *health, void *data) {

	JNIEnv *env = ((zjni_ArrayCallbackData_t *)data)->env;
	zjni_Collection_t *list = ((zjni_ArrayCallbackData_t *)data)->list;

	/* Construct ImportablePool object */
	jobject bean = create_ImportablePoolBean(
	    env, name, guid, pool_state, health);
	if (bean == NULL) {
		return (-1);
	}

	/* Add bean to list */
	(*env)->CallBooleanMethod(env, ((zjni_Object_t *)list)->object,
	    ((zjni_Collection_t *)list)->method_add, bean);

	return (0);
}

/*
 * Extern functions
 */

/*
 * Iterates through each importable pool on the system.  For each
 * importable pool, runs the given function with the given void as the
 * last arg.
 */
int
zjni_ipool_iter(int argc, char **argv, zjni_ipool_iter_f func, void *data)
{
	nvlist_t *pools = zpool_find_import(argc, argv);

	if (pools != NULL) {

		nvpair_t *elem = NULL;
		while ((elem = nvlist_next_nvpair(pools, elem)) != NULL) {
			nvlist_t *config;
			char *name;
			uint64_t guid;
			uint64_t pool_state;
			char *health;

			if (nvpair_value_nvlist(elem, &config) != 0 ||
			    nvlist_lookup_string(config,
				ZPOOL_CONFIG_POOL_NAME, &name) != 0 ||
			    nvlist_lookup_uint64(config,
				ZPOOL_CONFIG_POOL_GUID, &guid) != 0 ||
			    nvlist_lookup_uint64(config,
				ZPOOL_CONFIG_POOL_STATE, &pool_state) != 0 ||
			    nvlist_lookup_string(config,
				ZPOOL_CONFIG_POOL_HEALTH, &health) != 0) {

				return (-1);
			}

			/* Run the given function */
			if (func(name, guid, pool_state, health, data)) {
				return (-1);
			}
		}
	}

	return (0);
}

char *
zjni_get_state_str(uint64_t pool_state)
{
	char *str = NULL;
	switch (pool_state) {
		case POOL_STATE_ACTIVE:
			str = "POOL_STATE_ACTIVE";
			break;

		case POOL_STATE_EXPORTED:
			str = "POOL_STATE_EXPORTED";
			break;

		case POOL_STATE_DESTROYED:
			str = "POOL_STATE_DESTROYED";
			break;

		case POOL_STATE_UNINITIALIZED:
			str = "POOL_STATE_UNINITIALIZED";
			break;

		case POOL_STATE_UNAVAIL:
			str = "POOL_STATE_UNAVAIL";
			break;
	}

	return (str);
}
