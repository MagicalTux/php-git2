#include "php_git2.h"
#include "git2_exception.h"
#include "git2_commit.h"
#include "git2_repository.h"
#include "git2_tree.h"

static zend_class_entry *php_git2_commit_ce;
static zend_object_handlers php_git2_commit_handler;

typedef struct _git2_commit_object {
	zend_object std;
	git_commit *commit;
} git2_commit_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_lookup_oid, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, oid)
ZEND_END_ARG_INFO()

// somehow it seems we can't use just "lookup" as method name, it'll become php_lookup
static PHP_METHOD(Commit, lookup_oid) {
	zval *repo;
	char *oid;
	size_t oid_len;
	git_oid id;
	git_repository *git_repo;
	git2_commit_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &repo, git2_reference_class_entry(), &oid, &oid_len) != SUCCESS) {
		return;
	}

	git_repo = git2_repository_fetch_from_zval(repo);
	if (git_repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	if (oid_len != 20) {
		git2_throw_exception(0 TSRMLS_CC, "A git oid must be exactly 20 bytes");
		return;
	}
	memcpy(&id, oid, oid_len);

	object_init_ex(return_value, php_git2_commit_ce);
	intern = (git2_commit_object_t*)Z_OBJ_P(return_value);
	int res = git_commit_lookup(&intern->commit, git_repo, &id);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_lookup_prefix, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, oid_prefix)
ZEND_END_ARG_INFO()

static PHP_METHOD(Commit, lookup_prefix) {
	zval *repo;
	char *oid;
	size_t oid_len;
	git_oid id;
	git_repository *git_repo;
	git2_commit_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &repo, git2_reference_class_entry(), &oid, &oid_len) != SUCCESS) {
		return;
	}

	git_repo = git2_repository_fetch_from_zval(repo);
	if (git_repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	if (oid_len > 20) {
		git2_throw_exception(0 TSRMLS_CC, "A git oid must be 20 bytes or less");
		return;
	}
	memcpy(&id, oid, oid_len);

	object_init_ex(return_value, php_git2_commit_ce);
	intern = (git2_commit_object_t*)Z_OBJ_P(return_value);
	int res = git_commit_lookup_prefix(&intern->commit, git_repo, &id, oid_len);

	if (res != 0) {
		// TODO Throw exception
		RETURN_NULL();
	}
}

#define GIT2_COMMIT_FETCH() git2_commit_object_t *intern = (git2_commit_object_t*)Z_OBJ_P(getThis()); \
	if (intern->commit == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Commit object in invalid state"); \
		return; \
	}

#define GIT2_COMMIT_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Commit, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_COMMIT_FETCH(); \
		RETURN_LONG(git_commit_ ## _x(intern->commit)); \
	}

#define GIT2_COMMIT_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Commit, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_COMMIT_FETCH(); \
		const char *res = git_commit_ ## _x(intern->commit); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(git_commit_ ## _x(intern->commit)); \
	}

#define GIT2_COMMIT_GET_OID(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Commit, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_COMMIT_FETCH(); \
		const git_oid *res = git_commit_ ## _x(intern->commit); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRINGL((const char*)(res->id), GIT_OID_RAWSZ); \
	}

GIT2_COMMIT_GET_OID(id)
GIT2_COMMIT_GET_STRING(message_encoding)
GIT2_COMMIT_GET_STRING(message)
GIT2_COMMIT_GET_STRING(message_raw)
GIT2_COMMIT_GET_STRING(summary)
GIT2_COMMIT_GET_LONG(time)
GIT2_COMMIT_GET_LONG(time_offset)
GIT2_COMMIT_GET_STRING(raw_header)

ZEND_BEGIN_ARG_INFO_EX(arginfo_commit_tree, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Commit, tree) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_COMMIT_FETCH();

	git_tree *out;
	int res = git_commit_tree(&out, intern->commit);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}

	git2_tree_spawn(return_value, out);
}

void git2_commit_spawn(zval *return_value, git_commit *commit TSRMLS_DC) {
	git2_commit_object_t *intern;

	object_init_ex(return_value, php_git2_commit_ce);
	intern = (git2_commit_object_t*)Z_OBJ_P(return_value);
	intern->commit = commit;
}

zend_object *php_git2_commit_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_commit_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_commit_object_t));
	memset(intern, 0, sizeof(git2_commit_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_commit_handler;

	return &intern->std;
}

static void php_git2_commit_free_object(zend_object *object TSRMLS_DC) {
	git2_commit_object_t *intern = (git2_commit_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->commit) {
		git_commit_free(intern->commit);
		intern->commit = NULL;
	}

	// no need with PHP7 to free intern
}

#define PHP_GIT2_COMMIT_ME_P(_x) PHP_ME(Commit, _x, arginfo_commit_ ## _x, ZEND_ACC_PUBLIC)

static zend_function_entry git2_commit_methods[] = {
	PHP_ME(Commit, lookup_oid, arginfo_commit_lookup_oid, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Commit, lookup_prefix, arginfo_commit_lookup_prefix, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_GIT2_COMMIT_ME_P(id)
	PHP_GIT2_COMMIT_ME_P(message_encoding)
	PHP_GIT2_COMMIT_ME_P(message)
	PHP_GIT2_COMMIT_ME_P(message_raw)
	PHP_GIT2_COMMIT_ME_P(summary)
	PHP_GIT2_COMMIT_ME_P(time)
	PHP_GIT2_COMMIT_ME_P(time_offset)
	PHP_GIT2_COMMIT_ME_P(raw_header)
	PHP_GIT2_COMMIT_ME_P(tree)
/*	PHP_ME(Commit, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_commit_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Commit", git2_commit_methods);
	php_git2_commit_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_commit_ce->create_object = php_git2_commit_create_object;

	memcpy(&php_git2_commit_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_commit_handler.clone_obj = NULL;
	php_git2_commit_handler.free_obj = php_git2_commit_free_object;
}

