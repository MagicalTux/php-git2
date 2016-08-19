#include "php_git2.h"
#include "git2_exception.h"
#include "git2_blob.h"
#include "git2_repository.h"

static zend_class_entry *php_git2_blob_ce;
static zend_object_handlers php_git2_blob_handler;

typedef struct _git2_blob_object {
	zend_object std;
	git_blob *blob;
} git2_blob_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_lookup_oid, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, oid)
ZEND_END_ARG_INFO()

static PHP_METHOD(Blob, lookup_oid) {
	zval *z_repo;
	git_repository *repo;
	char *oid;
	size_t oid_len;
	git2_blob_object_t *intern;
	git_oid id;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &z_repo, git2_reference_class_entry(), &oid, &oid_len) == FAILURE) {
		return;
	}

	repo = git2_repository_fetch_from_zval(z_repo);
	if (repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	if (oid_len != 20) {
		git2_throw_exception(0 TSRMLS_CC, "oid must be 20 bytes long");
		return;
	}

	memcpy(&id, oid, oid_len);

	object_init_ex(return_value, php_git2_blob_ce);
	intern = (git2_blob_object_t*)Z_OBJ_P(return_value);
	int res = git_blob_lookup(&intern->blob, repo, &id);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

#define GIT2_BLOB_FETCH() git2_blob_object_t *intern = (git2_blob_object_t*)Z_OBJ_P(getThis()); \
	if (intern->blob == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Blob object in invalid state"); \
		return; \
	}

#define GIT2_BLOB_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Blob, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_BLOB_FETCH(); \
		RETURN_LONG(git_blob_ ## _x(intern->blob)); \
	}

#define GIT2_BLOB_GET_BOOL(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Blob, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_BLOB_FETCH(); \
		RETURN_BOOL(git_blob_ ## _x(intern->blob)); \
	}

#define GIT2_BLOB_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Blob, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_BLOB_FETCH(); \
		const char *res = git_blob_ ## _x(intern->blob); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(res); \
	}

#define GIT2_BLOB_GET_OID(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Blob, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_BLOB_FETCH(); \
		const git_oid *res = git_blob_ ## _x(intern->blob); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRINGL((const char*)(res->id), GIT_OID_RAWSZ); \
	}

GIT2_BLOB_GET_OID(id)
GIT2_BLOB_GET_BOOL(is_binary)
GIT2_BLOB_GET_LONG(rawsize)

ZEND_BEGIN_ARG_INFO_EX(arginfo_blob_rawcontent, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Blob, rawcontent) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_BLOB_FETCH();

	const void *res = git_blob_rawcontent(intern->blob);
	git_off_t len = git_blob_rawsize(intern->blob);

	RETURN_STRINGL((const char*)res, len);
}

void git2_blob_spawn(zval *return_value, git_blob *blob TSRMLS_DC) {
	git2_blob_object_t *intern;

	object_init_ex(return_value, php_git2_blob_ce);
	intern = (git2_blob_object_t*)Z_OBJ_P(return_value);
	intern->blob = blob;
}

zend_object *php_git2_blob_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_blob_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_blob_object_t));
	memset(intern, 0, sizeof(git2_blob_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_blob_handler;

	return &intern->std;
}

static void php_git2_blob_free_object(zend_object *object TSRMLS_DC) {
	git2_blob_object_t *intern = (git2_blob_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->blob) {
		git_blob_free(intern->blob);
		intern->blob = NULL;
	}

	// no need with PHP7 to free intern
}

#define PHP_GIT2_BLOB_ME_P(_x) PHP_ME(Blob, _x, arginfo_blob_##_x, ZEND_ACC_PUBLIC)

static zend_function_entry git2_blob_methods[] = {
	PHP_ME(Blob, lookup_oid, arginfo_blob_lookup_oid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
//	PHP_ME(Blob, create_fromworkdir, arginfo_blob_create_fromworkdir, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
//	PHP_ME(Blob, create_fromdisk, arginfo_blob_create_fromdisk, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_GIT2_BLOB_ME_P(id)
	PHP_GIT2_BLOB_ME_P(is_binary)
	PHP_GIT2_BLOB_ME_P(rawcontent)
	PHP_GIT2_BLOB_ME_P(rawsize)
/*	PHP_ME(Blob, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_blob_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Blob", git2_blob_methods);
	php_git2_blob_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_blob_ce->create_object = php_git2_blob_create_object;

	memcpy(&php_git2_blob_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_blob_handler.clone_obj = NULL;
	php_git2_blob_handler.free_obj = php_git2_blob_free_object;
}

