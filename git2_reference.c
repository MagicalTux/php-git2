#include "php_git2.h"
#include "git2_exception.h"
#include "git2_repository.h"
#include "git2_reference.h"
#include "git2_commit.h"
#include "git2_tree.h"
#include "git2_blob.h"

static zend_class_entry *php_git2_reference_ce;
static zend_object_handlers php_git2_reference_handler;

typedef struct _git2_reference_object {
	zend_object std;
	git_reference *ref;
} git2_reference_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_lookup_name, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(Reference, lookup_name) {
	zval *z_repo;
	git_repository *repo;
	char *name;
	size_t name_len;
	git2_reference_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &z_repo, git2_reference_class_entry(), &name, &name_len) == FAILURE)
		return;

	repo = git2_repository_fetch_from_zval(z_repo);
	if (repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	object_init_ex(return_value, php_git2_reference_ce);
	intern = (git2_reference_object_t*)Z_OBJ_P(return_value);
	int res = git_reference_lookup(&intern->ref, repo, name);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}


#define GIT2_REFERENCE_FETCH() git2_reference_object_t *intern = (git2_reference_object_t*)Z_OBJ_P(getThis()); \
	if (intern->ref == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Reference object in invalid state"); \
		return; \
	}

#define GIT2_REFERENCE_GET_BOOL(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Reference, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REFERENCE_FETCH(); \
		RETURN_BOOL(git_reference_ ## _x(intern->ref)); \
	}

#define GIT2_REFERENCE_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Reference, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REFERENCE_FETCH(); \
		const char *res = git_reference_ ## _x(intern->ref); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(git_reference_ ## _x(intern->ref)); \
	}

#define GIT2_REFERENCE_GET_OID(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Reference, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REFERENCE_FETCH(); \
		const git_oid *res = git_reference_ ## _x(intern->ref); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRINGL((const char*)(res->id), GIT_OID_RAWSZ); \
	}

GIT2_REFERENCE_GET_STRING(name)
GIT2_REFERENCE_GET_BOOL(is_branch)
GIT2_REFERENCE_GET_BOOL(is_remote)
GIT2_REFERENCE_GET_BOOL(is_tag)
GIT2_REFERENCE_GET_BOOL(is_note)
GIT2_REFERENCE_GET_STRING(shorthand)
GIT2_REFERENCE_GET_STRING(symbolic_target)
GIT2_REFERENCE_GET_OID(target)
GIT2_REFERENCE_GET_OID(target_peel)

ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_resolve, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Reference, resolve) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REFERENCE_FETCH();

	git_reference *out;
	if (git_reference_resolve(&out, intern->ref) != 0) {
		RETURN_FALSE;
	}

	git2_reference_spawn(return_value, out TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_reference_peel, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

static PHP_METHOD(Reference, peel) {
	long type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &type) != SUCCESS) {
		return;
	}
	GIT2_REFERENCE_FETCH();

	switch(type) {
		case GIT_OBJ_COMMIT: case GIT_OBJ_TAG: case GIT_OBJ_TREE: case GIT_OBJ_BLOB: case GIT_OBJ_ANY:
			break;
		default:
			git2_throw_exception(0 TSRMLS_CC, "Invalid type passed");
			return;
	}

	git_object *out;
	int res = git_reference_peel(&out, intern->ref, type);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}

	// TODO find object type, instanciate
	switch(git_object_type(out)) {
		case GIT_OBJ_COMMIT:
			git2_commit_spawn(return_value, (git_commit*)out TSRMLS_CC);
			return;
		case GIT_OBJ_TREE:
			git2_tree_spawn(return_value, (git_tree*)out TSRMLS_CC);
			return;
		case GIT_OBJ_BLOB:
			git2_blob_spawn(return_value, (git_blob*)out TSRMLS_CC);
			return;
		default:
			git2_throw_exception(0 TSRMLS_CC, "Type of object is not implemented");
			return;
	}
}

void git2_reference_spawn(zval *return_value, git_reference *ref TSRMLS_DC) {
	git2_reference_object_t *intern;

	object_init_ex(return_value, php_git2_reference_ce);
	intern = (git2_reference_object_t*)Z_OBJ_P(return_value);
	intern->ref = ref;
}

zend_object *php_git2_reference_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_reference_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_reference_object_t));
	memset(intern, 0, sizeof(git2_reference_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_reference_handler;

	return &intern->std;
}

static void php_git2_reference_free_object(zend_object *object TSRMLS_DC) {
	git2_reference_object_t *intern = (git2_reference_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->ref) {
		git_reference_free(intern->ref);
		intern->ref = NULL;
	}

	// no need with PHP7 to free intern
}

static zend_function_entry git2_reference_methods[] = {
	PHP_ME(Reference, lookup_name, arginfo_reference_lookup_name, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Reference, name, arginfo_reference_name, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, is_branch, arginfo_reference_is_branch, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, is_remote, arginfo_reference_is_remote, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, is_tag, arginfo_reference_is_tag, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, is_note, arginfo_reference_is_note, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, shorthand, arginfo_reference_shorthand, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, resolve, arginfo_reference_resolve, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, target, arginfo_reference_target, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, target_peel, arginfo_reference_target_peel, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, symbolic_target, arginfo_reference_symbolic_target, ZEND_ACC_PUBLIC)
	PHP_ME(Reference, peel, arginfo_reference_peel, ZEND_ACC_PUBLIC)
/*	PHP_ME(Reference, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_reference_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Reference", git2_reference_methods);
	php_git2_reference_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_reference_ce->create_object = php_git2_reference_create_object;

	memcpy(&php_git2_reference_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_reference_handler.clone_obj = NULL;
	php_git2_reference_handler.free_obj = php_git2_reference_free_object;

	// normalize
#define GIT2_REF_CONST(_x) zend_declare_class_constant_long(php_git2_reference_ce, ZEND_STRL(#_x), GIT_REF_ ## _x TSRMLS_CC)
	GIT2_REF_CONST(FORMAT_NORMAL);
	GIT2_REF_CONST(FORMAT_ALLOW_ONELEVEL);
	GIT2_REF_CONST(FORMAT_REFSPEC_PATTERN);
	GIT2_REF_CONST(FORMAT_REFSPEC_SHORTHAND);
}

