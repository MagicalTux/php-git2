#include "php_git2.h"
#include "git2_tree_entry.h"

static zend_class_entry *php_git2_tree_entry_ce;
static zend_object_handlers php_git2_tree_entry_handler;

typedef struct _git2_tree_entry_object {
	zend_object std;
	git_tree_entry *e;
	zend_bool readonly;
} git2_tree_entry_object_t;

#define GIT2_TREE_ENTRY_FETCH() git2_tree_entry_object_t *intern = (git2_tree_entry_object_t*)Z_OBJ_P(getThis()); \
	if (intern->e == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Tree\\Entry object in invalid state"); \
		return; \
	}

#define GIT2_TREE_ENTRY_GET_STRING(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_entry_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Entry, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_TREE_ENTRY_FETCH(); \
		const char *res = git_tree_entry_ ## _x(intern->e); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRING(res); \
	}

#define GIT2_TREE_ENTRY_GET_OID(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_entry_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Entry, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_TREE_ENTRY_FETCH(); \
		const git_oid *res = git_tree_entry_ ## _x(intern->e); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRINGL((const char*)(res->id), GIT_OID_RAWSZ); \
	}

#define GIT2_TREE_ENTRY_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_entry_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Entry, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_TREE_ENTRY_FETCH(); \
		RETURN_LONG(git_tree_entry_ ## _x(intern->e)); \
	}

GIT2_TREE_ENTRY_GET_STRING(name)
GIT2_TREE_ENTRY_GET_OID(id)
GIT2_TREE_ENTRY_GET_LONG(type)
GIT2_TREE_ENTRY_GET_LONG(filemode)
GIT2_TREE_ENTRY_GET_LONG(filemode_raw)

zend_object *php_git2_tree_entry_create_object(zend_class_entry *class_type TSRMLS_DC);

void git2_tree_entry_spawn(zval *return_value, git_tree_entry *e TSRMLS_DC) {
	git2_tree_entry_object_t *intern;

	object_init_ex(return_value, php_git2_tree_entry_ce);
	intern = (git2_tree_entry_object_t*)Z_OBJ_P(return_value);
	intern->e = e;
	intern->readonly = 0;
}

void git2_tree_entry_spawn_ephemeral(zval *return_value, const git_tree_entry *e TSRMLS_DC) {
	git2_tree_entry_object_t *intern;

	object_init_ex(return_value, php_git2_tree_entry_ce);
	intern = (git2_tree_entry_object_t*)Z_OBJ_P(return_value);
	intern->e = e;
	intern->readonly = 1;
}

zend_object *php_git2_tree_entry_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_tree_entry_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_tree_entry_object_t));
	memset(intern, 0, sizeof(git2_tree_entry_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_tree_entry_handler;

	return &intern->std;
}

static void php_git2_tree_entry_free_object(zend_object *object TSRMLS_DC) {
	git2_tree_entry_object_t *intern = (git2_tree_entry_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if ((intern->e) && (!intern->readonly)) {
		git_tree_entry_free(intern->e);
		intern->e = NULL;
	}

	// no need with PHP7 to free intern
}

#define PHP_GIT2_TREE_ENTRY_ME(_x) PHP_ME(Entry, _x, arginfo_tree_entry_##_x, ZEND_ACC_PUBLIC)
static zend_function_entry git2_tree_entry_methods[] = {
	PHP_GIT2_TREE_ENTRY_ME(name)
	PHP_GIT2_TREE_ENTRY_ME(id)
	PHP_GIT2_TREE_ENTRY_ME(type)
	PHP_GIT2_TREE_ENTRY_ME(filemode)
	PHP_GIT2_TREE_ENTRY_ME(filemode_raw)
/*	PHP_ME(Entry, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_tree_entry_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2\\Tree", "Entry", git2_tree_entry_methods);
	php_git2_tree_entry_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_tree_entry_ce->create_object = php_git2_tree_entry_create_object;

	memcpy(&php_git2_tree_entry_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_tree_entry_handler.clone_obj = NULL;
	php_git2_tree_entry_handler.free_obj = php_git2_tree_entry_free_object;
}

