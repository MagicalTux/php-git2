#include "php_git2.h"
#include "git2_remote.h"
#include "git2_repository.h"

static zend_class_entry *php_git2_remote_ce;
static zend_object_handlers php_git2_remote_handler;

typedef struct _git2_remote_object {
	zend_object std;
	git_remote *remote;
} git2_remote_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_create_anonymous, 0, 0, 2)
	ZEND_ARG_INFO(0, repository)
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
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Parameter must be a valid git repository", 0 TSRMLS_CC);
		return;
	}

	object_init_ex(return_value, php_git2_remote_ce);
	intern = (git2_remote_object_t*)Z_OBJ_P(return_value);
	int res = git_remote_create_anonymous(&intern->remote, repo, url);

	if (res != 0) {
		// TODO Throw exception
		RETURN_NULL();
	}
}


#define GIT2_REMOTE_FETCH() git2_remote_object_t *intern = (git2_remote_object_t*)Z_OBJ_P(getThis()); \
	if (intern->remote == NULL) { \
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Git2\\Remote object in invalid state", 0 TSRMLS_CC); \
		return; \
	}

#define GIT2_REMOTE_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_remote_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Remote, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_REMOTE_FETCH(); \
		RETURN_LONG(git_remote_ ## _x(intern->remote)); \
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

static zend_function_entry git2_remote_methods[] = {
	PHP_ME(Remote, create_anonymous, arginfo_remote_create_anonymous, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Remote, name, arginfo_remote_name, ZEND_ACC_PUBLIC)
	PHP_ME(Remote, url, arginfo_remote_url, ZEND_ACC_PUBLIC)
	PHP_ME(Remote, pushurl, arginfo_remote_pushurl, ZEND_ACC_PUBLIC)
	PHP_ME(Remote, refspec_count, arginfo_remote_refspec_count, ZEND_ACC_PUBLIC)
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

