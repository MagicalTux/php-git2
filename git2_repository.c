#include "php_git2.h"
#include "git2_repository.h"
#include "git2_config.h"
#include "git2_reference.h"

static zend_class_entry *php_git2_repository_ce;
static zend_object_handlers php_git2_repository_handler;

typedef struct _git2_repository_object {
	zend_object std;
	git_repository *repo;
} git2_repository_object_t;

zend_class_entry *git2_reference_class_entry() {
	return php_git2_repository_ce;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_open, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, open) {
	char *path;
	size_t path_len;
	zend_long flags = 0;
	git2_repository_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &path, &path_len, &flags) == FAILURE) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, php_git2_repository_ce);

	intern = (git2_repository_object_t*)Z_OBJ_P(return_value);

	int res = git_repository_open_ext(&intern->repo, path, flags, NULL);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_open_bare, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, open_bare) {
	char *path;
	size_t path_len;
	git2_repository_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len) == FAILURE) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, php_git2_repository_ce);

	intern = (git2_repository_object_t*)Z_OBJ_P(return_value);

	int res = git_repository_open_bare(&intern->repo, path);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_init, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, is_bare)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, init) {
	char *path;
	size_t path_len;
	zend_bool is_bare = 0;
	git2_repository_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &path, &path_len, &is_bare) == FAILURE) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, php_git2_repository_ce);

	intern = (git2_repository_object_t*)Z_OBJ_P(return_value);

	int res = git_repository_init(&intern->repo, path, is_bare ? 1 : 0);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_init_ext, 0, 0, 2)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, init_ext) {
	char *path;
	size_t path_len;
	HashTable *opts;
	zval *data;
	git2_repository_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sh", &path, &path_len, &opts) == FAILURE) {
		RETURN_FALSE;
	}

	git_repository_init_options opts_libgit2 = GIT_REPOSITORY_INIT_OPTIONS_INIT;
	if ((data = zend_hash_str_find(opts, ZEND_STRL("flags"))) != NULL) {
		convert_to_long(data); // is it safe to call convert_to_long() here?
		opts_libgit2.flags = Z_LVAL_P(data);
	}

	if ((data = zend_hash_str_find(opts, ZEND_STRL("mode"))) != NULL) {
		convert_to_long(data); // is it safe to call convert_to_long() here?
		opts_libgit2.mode = Z_LVAL_P(data);
	}
	// TODO: workdir_path description template_path initial_head origin_url

	object_init_ex(return_value, php_git2_repository_ce);

	intern = (git2_repository_object_t*)Z_OBJ_P(return_value);

	int res = git_repository_init_ext(&intern->repo, path, &opts_libgit2);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}
}

#define GIT2_REPOSITORY_FETCH() git2_repository_object_t *intern = (git2_repository_object_t*)Z_OBJ_P(getThis()); \
	if (intern->repo == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Repository object in invalid state"); \
		return; \
	}

#define GIT2_REPOSITORY_GET_BOOL(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Repository, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REPOSITORY_FETCH(); \
		RETURN_BOOL(git_repository_ ## _x(intern->repo)); \
	}

#define GIT2_REPOSITORY_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Repository, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REPOSITORY_FETCH(); \
		const char *res = git_repository_ ## _x(intern->repo); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(res); \
	}

#define GIT2_REPOSITORY_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Repository, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REPOSITORY_FETCH(); \
		RETURN_LONG(git_repository_ ## _x(intern->repo)); \
	}

GIT2_REPOSITORY_GET_BOOL(head_detached)
GIT2_REPOSITORY_GET_BOOL(head_unborn)
GIT2_REPOSITORY_GET_BOOL(is_empty)
GIT2_REPOSITORY_GET_STRING(path)
GIT2_REPOSITORY_GET_STRING(workdir)
GIT2_REPOSITORY_GET_BOOL(is_bare)
GIT2_REPOSITORY_GET_LONG(state)
GIT2_REPOSITORY_GET_STRING(get_namespace)
GIT2_REPOSITORY_GET_BOOL(is_shallow)

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_config, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, config) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REPOSITORY_FETCH();

	git_config *out;
	int res = git_repository_config(&out, intern->repo);
	if (res != 0) {
		git2_throw_last_error(TSRMLS_C);
		return;
	}

	git2_config_spawn(return_value, out TSRMLS_CC);
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_head, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, head) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REPOSITORY_FETCH();

	git_reference *out;
	int res = git_repository_head(&out, intern->repo);
	if (res != 0) {
		git2_throw_last_error();
		RETURN_FALSE;
	}

	git2_reference_spawn(return_value, out TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_set_head, 0, 0, 1)
	ZEND_ARG_INFO(0, refname)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, set_head) {
	char *refname;
	size_t refname_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &refname, &refname_len) == FAILURE) {
		RETURN_FALSE;
	}

	GIT2_REPOSITORY_FETCH();

	int res = git_repository_set_head(intern->repo, refname);

	if (res != 0) {
		git2_throw_last_error();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_checkout_head, 0, 0, 0)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

static PHP_METHOD(Repository, checkout_head) {
	HashTable *opts = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|H", &opts) == FAILURE) {
		RETURN_FALSE;
	}

	GIT2_REPOSITORY_FETCH();

	// TODO handle opts

	int res = git_checkout_head(intern->repo, NULL);

	if (res != 0) {
		git2_throw_last_error();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

git_repository *git2_repository_fetch_from_zval(zval *zv) {
	if (Z_TYPE_P(zv) != IS_OBJECT) return NULL;
	if (Z_OBJCE_P(zv) != php_git2_repository_ce) return NULL;

	git2_repository_object_t *intern = (git2_repository_object_t*)Z_OBJ_P(zv);
	return intern->repo;
}

zend_object *php_git2_repository_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_repository_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_repository_object_t));
	memset(intern, 0, sizeof(git2_repository_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_repository_handler;

	return &intern->std;
}

static void php_git2_repository_free_object(zend_object *object TSRMLS_DC) {
	git2_repository_object_t *intern = (git2_repository_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->repo) {
		git_repository_free(intern->repo);
		intern->repo = NULL;
	}

	// no need with PHP7 to free intern
}

static zend_function_entry git2_repository_methods[] = {
	PHP_ME(Repository, open, arginfo_repository_open, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Repository, open_bare, arginfo_repository_open_bare, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Repository, init, arginfo_repository_init, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Repository, init_ext, arginfo_repository_init_ext, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Repository, config, arginfo_repository_config, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, head, arginfo_repository_head, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, set_head, arginfo_repository_set_head, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, checkout_head, arginfo_repository_checkout_head, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, head_detached, arginfo_repository_head_detached, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, head_unborn, arginfo_repository_head_unborn, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, is_empty, arginfo_repository_is_empty, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, path, arginfo_repository_path, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, workdir, arginfo_repository_workdir, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, is_bare, arginfo_repository_is_bare, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, state, arginfo_repository_state, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, get_namespace, arginfo_repository_get_namespace, ZEND_ACC_PUBLIC)
	PHP_ME(Repository, is_shallow, arginfo_repository_is_shallow, ZEND_ACC_PUBLIC)

/*	PHP_ME(Repository, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_repository_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Repository", git2_repository_methods);
	php_git2_repository_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_repository_ce->create_object = php_git2_repository_create_object;

	memcpy(&php_git2_repository_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_repository_handler.clone_obj = NULL;
	php_git2_repository_handler.free_obj = php_git2_repository_free_object;

	// open_ext constants
#define GIT2_REP_CONST(_x) zend_declare_class_constant_long(php_git2_repository_ce, ZEND_STRL(#_x), GIT_REPOSITORY_ ## _x TSRMLS_CC)
	GIT2_REP_CONST(OPEN_NO_SEARCH);
	GIT2_REP_CONST(OPEN_CROSS_FS);
	GIT2_REP_CONST(OPEN_BARE);

	// init_ext constants
	GIT2_REP_CONST(INIT_BARE);
	GIT2_REP_CONST(INIT_NO_REINIT);
	GIT2_REP_CONST(INIT_NO_DOTGIT_DIR);
	GIT2_REP_CONST(INIT_MKDIR);
	GIT2_REP_CONST(INIT_MKPATH);
	GIT2_REP_CONST(INIT_EXTERNAL_TEMPLATE);
	GIT2_REP_CONST(INIT_RELATIVE_GITLINK);

	// init_ext mode of operation (actually just a chmod thing)
	GIT2_REP_CONST(INIT_SHARED_UMASK);
	GIT2_REP_CONST(INIT_SHARED_GROUP);
	GIT2_REP_CONST(INIT_SHARED_ALL);

	// state
	GIT2_REP_CONST(STATE_NONE);
	GIT2_REP_CONST(STATE_MERGE);
	GIT2_REP_CONST(STATE_REVERT);
	GIT2_REP_CONST(STATE_CHERRYPICK);
	GIT2_REP_CONST(STATE_BISECT);
	GIT2_REP_CONST(STATE_REBASE);
	GIT2_REP_CONST(STATE_REBASE_INTERACTIVE);
	GIT2_REP_CONST(STATE_REBASE_MERGE);
	GIT2_REP_CONST(STATE_APPLY_MAILBOX);
	GIT2_REP_CONST(STATE_APPLY_MAILBOX_OR_REBASE);
}

