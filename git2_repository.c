#include "php_git2.h"
#include "git2_repository.h"

static zend_class_entry *php_git2_repository_ce;
static zend_object_handlers php_git2_repository_handler;

typedef struct _git2_repository_object {
	zend_object std;
	// TODO
} git2_repository_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_repository_open, 0, 0, 1)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

PHP_METHOD(Repository, open) {
	char *path;
	int path_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "p", &path, &path_len) == FAILURE) {
		RETURN_FALSE;
	}

	object_init_ex(return_value, php_git2_repository_ce);

//	intern = Z_BINARY_OBJ_P(return_value);
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

	// TODO
}

static zend_function_entry git2_repository_methods[] = {
	PHP_ME(Repository, open, arginfo_repository_open, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
/*	PHP_ME(Repository, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_repository_init() {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Repository", git2_repository_methods);
	php_git2_repository_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_repository_ce->create_object = php_git2_repository_create_object;

	memcpy(&php_git2_repository_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_repository_handler.clone_obj = NULL;
	php_git2_repository_handler.free_obj = php_git2_repository_free_object;
}

