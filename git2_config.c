#include "php_git2.h"
#include "git2_exception.h"
#include "git2_config.h"
#include "git2_config_entry.h"

static zend_class_entry *php_git2_config_ce;
static zend_object_handlers php_git2_config_handler;

typedef struct _git2_config_object {
	zend_object std;
	git_config *config;
} git2_config_object_t;

#define GIT2_CONFIG_FETCH() git2_config_object_t *intern = (git2_config_object_t*)Z_OBJ_P(getThis()); \
	if (intern->config == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Config object in invalid state"); \
		return; \
	}

ZEND_BEGIN_ARG_INFO_EX(arginfo_config_get_entry, 0, 0, 1)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static PHP_METHOD(Config, get_entry) {
	char *c_name;
	size_t c_name_len;
	git_config_entry *e;
	GIT2_CONFIG_FETCH();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &c_name, &c_name_len) == FAILURE) {
		RETURN_NULL();
	}

	if (git_config_get_entry(&e, intern->config, c_name) != 0) {
		RETURN_NULL();
	}

	git2_config_entry_spawn(&return_value, e);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_config_export, 0, 0, 0)
ZEND_END_ARG_INFO()

static PHP_METHOD(Config, export) {
	git_config_iterator *it;
	git_config_entry *e;

	if (zend_parse_parameters_none() == FAILURE) return;
	GIT2_CONFIG_FETCH();

	if (git_config_iterator_new(&it, intern->config) != 0) {
		RETURN_NULL();
	}

	array_init(return_value);

	while(1) {
		int res = git_config_next(&e, it);
		if (res != 0) break;

		add_assoc_string(return_value, e->name, (char*)e->value);
	}
	git_config_iterator_free(it);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_config_set_string, 0, 0, 2)
	ZEND_ARG_INFO(0, name)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

static PHP_METHOD(Config, set_string) {
	char *name, *value;
	size_t *name_len, *value_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &value, &value_len) == FAILURE) {
		RETURN_FALSE;
	}

	GIT2_CONFIG_FETCH();

	int res = git_config_set_string(intern->config, name, value);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}

	RETURN_TRUE;
}

void git2_config_spawn(zval **return_value, git_config *config TSRMLS_DC) {
	git2_config_object_t *intern;

	object_init_ex(*return_value, php_git2_config_ce);
	intern = (git2_config_object_t*)Z_OBJ_P(*return_value);
	intern->config = config;
}

zend_object *php_git2_config_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_config_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_config_object_t));
	memset(intern, 0, sizeof(git2_config_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_config_handler;

	return &intern->std;
}

static void php_git2_config_free_object(zend_object *object TSRMLS_DC) {
	git2_config_object_t *intern = (git2_config_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->config) {
		git_config_free(intern->config);
		intern->config = NULL;
	}

	// no need with PHP7 to free intern
}

static zend_function_entry git2_config_methods[] = {
	PHP_ME(Config, get_entry, arginfo_config_get_entry, ZEND_ACC_PUBLIC)
	PHP_ME(Config, export, arginfo_config_export, ZEND_ACC_PUBLIC)
	PHP_ME(Config, set_string, arginfo_config_set_string, ZEND_ACC_PUBLIC)
/*	PHP_ME(Config, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_config_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Config", git2_config_methods);
	php_git2_config_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_config_ce->create_object = php_git2_config_create_object;

	memcpy(&php_git2_config_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_config_handler.clone_obj = NULL;
	php_git2_config_handler.free_obj = php_git2_config_free_object;

#define GIT2_CONFIG_CONST(_x) zend_declare_class_constant_long(php_git2_config_ce, ZEND_STRL(#_x), GIT_CONFIG_ ## _x TSRMLS_CC)
#define GIT2_CONST(_x) zend_declare_class_constant_long(php_git2_config_ce, ZEND_STRL(#_x), GIT_ ## _x TSRMLS_CC)

	// config var priority
	GIT2_CONFIG_CONST(LEVEL_SYSTEM);
	GIT2_CONFIG_CONST(LEVEL_XDG);
	GIT2_CONFIG_CONST(LEVEL_GLOBAL);
	GIT2_CONFIG_CONST(LEVEL_LOCAL);
	GIT2_CONFIG_CONST(LEVEL_APP);
	GIT2_CONFIG_CONST(HIGHEST_LEVEL);

	// cvar
	GIT2_CONST(CVAR_FALSE);
	GIT2_CONST(CVAR_TRUE);
	GIT2_CONST(CVAR_INT32);
	GIT2_CONST(CVAR_STRING);
}

