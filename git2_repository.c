#include "php_git2.h"
#include "git2_repository.h"

static zend_class_entry *php_git2_repository_ce;
static zend_object_handlers php_git2_repository_handler;

typedef struct _git2_repository_object {
	zend_object std;
	git_repository *repo;
} git2_repository_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_open, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

PHP_METHOD(Repository, open) {
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
		// TODO Throw exception
		RETURN_FALSE;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_init, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, is_bare)
ZEND_END_ARG_INFO()

PHP_METHOD(Repository, init) {
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
		// TODO Throw exception
		RETURN_FALSE;
	}
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
	PHP_ME(Repository, init, arginfo_repository_init, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
/*	PHP_ME(Repository, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_repository_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Repository", git2_repository_methods);
	php_git2_repository_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_repository_ce->create_object = php_git2_repository_create_object;

	memcpy(&php_git2_repository_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_repository_handler.clone_obj = NULL;
	php_git2_repository_handler.free_obj = php_git2_repository_free_object;

	zend_declare_class_constant_long(php_git2_repository_ce, ZEND_STRL("OPEN_NO_SEARCH"), GIT_REPOSITORY_OPEN_NO_SEARCH TSRMLS_CC);
	zend_declare_class_constant_long(php_git2_repository_ce, ZEND_STRL("OPEN_CROSS_FS"), GIT_REPOSITORY_OPEN_CROSS_FS TSRMLS_CC);
	zend_declare_class_constant_long(php_git2_repository_ce, ZEND_STRL("OPEN_BARE"), GIT_REPOSITORY_OPEN_BARE TSRMLS_CC);
}

