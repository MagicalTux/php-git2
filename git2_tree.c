#include "php_git2.h"
#include "git2_exception.h"
#include "git2_tree.h"

static zend_class_entry *php_git2_tree_ce;
static zend_object_handlers php_git2_tree_handler;

typedef struct _git2_tree_object {
	zend_object std;
	git_tree *tree;
} git2_tree_object_t;


#define GIT2_TREE_FETCH() git2_tree_object_t *intern = (git2_tree_object_t*)Z_OBJ_P(getThis()); \
	if (intern->tree == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Tree object in invalid state"); \
		return; \
	}

#define GIT2_TREE_GET_LONG(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Tree, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_TREE_FETCH(); \
		RETURN_LONG(git_tree_ ## _x(intern->tree)); \
	}

#define GIT2_TREE_GET_OID(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Tree, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_TREE_FETCH(); \
		const git_oid *res = git_tree_ ## _x(intern->tree); \
		if (res == NULL) { RETURN_NULL(); } \
		RETURN_STRINGL((const char*)(res->id), GIT_OID_RAWSZ); \
	}

GIT2_TREE_GET_OID(id)
GIT2_TREE_GET_LONG(entrycount)

void git2_tree_spawn(zval **return_value, git_tree *tree TSRMLS_DC) {
	git2_tree_object_t *intern;

	object_init_ex(*return_value, php_git2_tree_ce);
	intern = (git2_tree_object_t*)Z_OBJ_P(*return_value);
	intern->tree = tree;
}

zend_object *php_git2_tree_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_tree_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_tree_object_t));
	memset(intern, 0, sizeof(git2_tree_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_tree_handler;

	return &intern->std;
}

static void php_git2_tree_free_object(zend_object *object TSRMLS_DC) {
	git2_tree_object_t *intern = (git2_tree_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->tree) {
		git_tree_free(intern->tree);
		intern->tree = NULL;
	}

	// no need with PHP7 to free intern
}

static zend_function_entry git2_tree_methods[] = {
	PHP_ME(Tree, id, arginfo_tree_id, ZEND_ACC_PUBLIC)
	PHP_ME(Tree, entrycount, arginfo_tree_entrycount, ZEND_ACC_PUBLIC)
/*	PHP_ME(Tree, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_tree_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Tree", git2_tree_methods);
	php_git2_tree_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_tree_ce->create_object = php_git2_tree_create_object;

	memcpy(&php_git2_tree_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_tree_handler.clone_obj = NULL;
	php_git2_tree_handler.free_obj = php_git2_tree_free_object;

	// normalize
#define GIT2_REF_CONST(_x) zend_declare_class_constant_long(php_git2_tree_ce, ZEND_STRL(#_x), GIT_REF_ ## _x TSRMLS_CC)
	GIT2_REF_CONST(FORMAT_NORMAL);
	GIT2_REF_CONST(FORMAT_ALLOW_ONELEVEL);
	GIT2_REF_CONST(FORMAT_REFSPEC_PATTERN);
	GIT2_REF_CONST(FORMAT_REFSPEC_SHORTHAND);
}

