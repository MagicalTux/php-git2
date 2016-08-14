#include "php_git2.h"
#include "git2_config_entry.h"

static zend_class_entry *php_git2_config_entry_ce;
static zend_object_handlers php_git2_config_entry_handler;

typedef struct _git2_config_entry_object {
	zend_object std;
	git_config_entry *e;
} git2_config_entry_object_t;


#define GIT2_CONFIG_ENTRY_FETCH() git2_config_entry_object_t *intern = (git2_config_entry_object_t*)Z_OBJ_P(getThis()); \
	if (intern->e == NULL) { \
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "Git2\\Entry object in invalid state", 0 TSRMLS_CC); \
		return; \
	}

#define GIT2_CONFIG_ENTRY_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_config_entry_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Entry, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_CONFIG_ENTRY_FETCH(); \
		const char *res = intern->e->_x; \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(res); \
	}

GIT2_CONFIG_ENTRY_GET_STRING(name)
GIT2_CONFIG_ENTRY_GET_STRING(value)

void git2_config_entry_spawn(zval **return_value, git_config_entry *e TSRMLS_DC) {
	git2_config_entry_object_t *intern;

	object_init_ex(*return_value, php_git2_config_entry_ce);
	intern = (git2_config_entry_object_t*)Z_OBJ_P(*return_value);
	intern->e = e;
}

zend_object *php_git2_config_entry_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_config_entry_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_config_entry_object_t));
	memset(intern, 0, sizeof(git2_config_entry_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_config_entry_handler;

	return &intern->std;
}

static void php_git2_config_entry_free_object(zend_object *object TSRMLS_DC) {
	git2_config_entry_object_t *intern = (git2_config_entry_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->e) {
		git_config_entry_free(intern->e);
		intern->e = NULL;
	}

	// no need with PHP7 to free intern
}

static zend_function_entry git2_config_entry_methods[] = {
	PHP_ME(Entry, name, arginfo_config_entry_name, ZEND_ACC_PUBLIC)
	PHP_ME(Entry, value, arginfo_config_entry_value, ZEND_ACC_PUBLIC)
/*	PHP_ME(Entry, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_config_entry_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2\\Config", "Entry", git2_config_entry_methods);
	php_git2_config_entry_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_config_entry_ce->create_object = php_git2_config_entry_create_object;

	memcpy(&php_git2_config_entry_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_config_entry_handler.clone_obj = NULL;
	php_git2_config_entry_handler.free_obj = php_git2_config_entry_free_object;
}

