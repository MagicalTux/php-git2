#include "php_git2.h"
#include "git2_exception.h"
#include "git2_remote.h"
#include "git2_repository.h"

static zend_class_entry *php_git2_remote_ce;
static zend_object_handlers php_git2_remote_handler;

typedef struct _git2_remote_object {
	zend_object std;
	git_remote *remote;
} git2_remote_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_create_anonymous, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, create_anonymous) {
	zval *z_repo;
	git_repository *repo;
	char *url;
	size_t url_len;
	git2_remote_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Os", &z_repo, git2_reference_class_entry(), &url, &url_len) == FAILURE) {
		RETURN_FALSE;
	}

	repo = git2_repository_fetch_from_zval(z_repo);
	if (repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	object_init_ex(return_value, php_git2_remote_ce);
	intern = (git2_remote_object_t*)Z_OBJ_P(return_value);
	int res = git_remote_create_anonymous(&intern->remote, repo, url);

	if (res != 0) {
		git2_throw_last_error();
		RETURN_NULL();
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_create_with_fetchspec, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, url)
	ZEND_ARG_INFO(0, fetch)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, create_with_fetchspec) {
	zval *z_repo;
	git_repository *repo;
	char *name, *url, *fetch;
	size_t name_len, url_len, fetch_len;
	git2_remote_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Osss", &z_repo, git2_reference_class_entry(), &name, &name_len, &url, &url_len, &fetch, &fetch_len) == FAILURE) {
		RETURN_FALSE;
	}

	repo = git2_repository_fetch_from_zval(z_repo);
	if (repo == NULL) {
		git2_throw_exception(0 TSRMLS_CC, "Parameter must be a valid git repository");
		return;
	}

	object_init_ex(return_value, php_git2_remote_ce);
	intern = (git2_remote_object_t*)Z_OBJ_P(return_value);
	int res = git_remote_create_with_fetchspec(&intern->remote, repo, name, url, fetch);

	if (res != 0) {
		git2_throw_last_error();
		RETURN_NULL();
	}
}

#define GIT2_REMOTE_FETCH() git2_remote_object_t *intern = (git2_remote_object_t*)Z_OBJ_P(getThis()); \
	if (intern->remote == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Remote object in invalid state"); \
		return; \
	}

#define GIT2_REMOTE_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Remote, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REMOTE_FETCH(); \
		RETURN_LONG(git_remote_ ## _x(intern->remote)); \
	}

#define GIT2_REMOTE_GET_BOOL(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Remote, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REMOTE_FETCH(); \
		RETURN_BOOL(git_remote_ ## _x(intern->remote)); \
	}

#define GIT2_REMOTE_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Remote, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REMOTE_FETCH(); \
		const char *res = git_remote_ ## _x(intern->remote); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(res); \
	}

GIT2_REMOTE_GET_STRING(name)
GIT2_REMOTE_GET_STRING(url)
GIT2_REMOTE_GET_STRING(pushurl)
GIT2_REMOTE_GET_LONG(refspec_count)
GIT2_REMOTE_GET_BOOL(connected)

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_connect, 0, 0, 1)
	ZEND_ARG_INFO(0, is_push) /* set to true for push, false for fetch */
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, connect) {
	zend_bool is_push;

	GIT2_REMOTE_FETCH();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &is_push) == FAILURE) {
		return;
	}

	// TODO handle callbacks
	int res = git_remote_connect(intern->remote, is_push ? GIT_DIRECTION_PUSH : GIT_DIRECTION_FETCH, NULL);

	if (res == 0) {
		RETURN_TRUE;
	}

	git2_throw_last_error();
	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_stop, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, stop) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REMOTE_FETCH();

	git_remote_stop(intern->remote);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_disconnect, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, disconnect) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REMOTE_FETCH();

	git_remote_disconnect(intern->remote);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_download, 0, 0, 0)
	ZEND_ARG_INFO(0, refspecs)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, download) {
	HashTable *refspecs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &refspecs) != SUCCESS) {
		return;
	}

	GIT2_REMOTE_FETCH();
	// TODO add git_fetch_options handling
	git_fetch_options git_opts = GIT_FETCH_OPTIONS_INIT;
	git_strarray git_refspecs;
	git_refspecs.count = 0;

	if (refspecs)
		php_git2_ht_to_strarray(&git_refspecs, refspecs);

	int res = git_remote_download(intern->remote, &git_refspecs, &git_opts);

	php_git2_strarray_free(&git_refspecs);

	if (res == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_fetch, 0, 0, 0)
	ZEND_ARG_INFO(0, refspecs)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, fetch) {
	HashTable *refspecs = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &refspecs) != SUCCESS) {
		return;
	}

	GIT2_REMOTE_FETCH();
	// TODO add git_fetch_options handling and reflog_message
	git_fetch_options git_opts = GIT_FETCH_OPTIONS_INIT;
	git_strarray git_refspecs;
	git_refspecs.count = 0;

	if (refspecs)
		php_git2_ht_to_strarray(&git_refspecs, refspecs);

	int res = git_remote_fetch(intern->remote, &git_refspecs, &git_opts, NULL);

	php_git2_strarray_free(&git_refspecs);

	if (res != 0) {
		git2_throw_last_error(TSRMLS_CC);
		return;
	}
	RETURN_TRUE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_upload, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, upload) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REMOTE_FETCH();
	// TODO add git_push_options handling

	int res = git_remote_upload(intern->remote, NULL, NULL);

	if (res == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_push, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Remote, push) {
	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_REMOTE_FETCH();
	// TODO add git_push_options handling

	int res = git_remote_push(intern->remote, NULL, NULL);

	if (res == 0) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}

void git2_remote_spawn(zval **return_value, git_remote *remote TSRMLS_DC) {
	git2_remote_object_t *intern;

	object_init_ex(*return_value, php_git2_remote_ce);
	intern = (git2_remote_object_t*)Z_OBJ_P(*return_value);
	intern->remote = remote;
}

zend_object *php_git2_remote_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_remote_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_remote_object_t));
	memset(intern, 0, sizeof(git2_remote_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_remote_handler;

	return &intern->std;
}

static void php_git2_remote_free_object(zend_object *object TSRMLS_DC) {
	git2_remote_object_t *intern = (git2_remote_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->remote) {
		git_remote_free(intern->remote);
		intern->remote = NULL;
	}

	// no need with PHP7 to free intern
}

#define PHP_GIT2_REMOTE_ME_P(_x) PHP_ME(Remote, _x, arginfo_remote_##_x, ZEND_ACC_PUBLIC)

static zend_function_entry git2_remote_methods[] = {
	PHP_ME(Remote, create_anonymous, arginfo_remote_create_anonymous, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Remote, create_with_fetchspec, arginfo_remote_create_with_fetchspec, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_GIT2_REMOTE_ME_P(name)
	PHP_GIT2_REMOTE_ME_P(url)
	PHP_GIT2_REMOTE_ME_P(pushurl)
	PHP_GIT2_REMOTE_ME_P(refspec_count)
	PHP_GIT2_REMOTE_ME_P(connected)
	PHP_GIT2_REMOTE_ME_P(connect)
	PHP_GIT2_REMOTE_ME_P(stop)
	PHP_GIT2_REMOTE_ME_P(disconnect)
	PHP_GIT2_REMOTE_ME_P(download)
	PHP_GIT2_REMOTE_ME_P(fetch)
	PHP_GIT2_REMOTE_ME_P(upload)
	PHP_GIT2_REMOTE_ME_P(push)
/*	PHP_ME(Remote, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_remote_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Remote", git2_remote_methods);
	php_git2_remote_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_remote_ce->create_object = php_git2_remote_create_object;

	memcpy(&php_git2_remote_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_remote_handler.clone_obj = NULL;
	php_git2_remote_handler.free_obj = php_git2_remote_free_object;

#define GIT2_REMOTE_CONST(_x) zend_declare_class_constant_long(php_git2_remote_ce, ZEND_STRL(#_x), GIT_REMOTE_ ## _x TSRMLS_CC)

	GIT2_REMOTE_CONST(DOWNLOAD_TAGS_UNSPECIFIED);
	GIT2_REMOTE_CONST(DOWNLOAD_TAGS_AUTO);
	GIT2_REMOTE_CONST(DOWNLOAD_TAGS_NONE);
	GIT2_REMOTE_CONST(DOWNLOAD_TAGS_ALL);

	GIT2_REMOTE_CONST(COMPLETION_DOWNLOAD);
	GIT2_REMOTE_CONST(COMPLETION_INDEXING);
	GIT2_REMOTE_CONST(COMPLETION_ERROR);
}

