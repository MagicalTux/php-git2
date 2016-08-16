#include "php_git2.h"
#include "git2_exception.h"
#include "git2_repository.h"
#include "git2_tree.h"
#include "git2_tree_entry.h"

static zend_class_entry *php_git2_tree_ce;
static zend_object_handlers php_git2_tree_handler;

typedef struct _git2_tree_object {
	zend_object std;
	git_tree *tree;
} git2_tree_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_lookup_oid, 0, 0, 2)
	ZEND_ARG_OBJ_INFO(0, repository, Git2\\Repository, 0)
	ZEND_ARG_INFO(0, oid)
ZEND_END_ARG_INFO()

// somehow it seems we can't use just "lookup" as method name, it'll become php_lookup
static PHP_METHOD(Tree, lookup_oid) {
	zval *repo;
	char *oid;
	size_t oid_len;
	git_oid id;
	git_repository *git_repo;
	git2_tree_object_t *intern;

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

	object_init_ex(return_value, php_git2_tree_ce);
	intern = (git2_tree_object_t*)Z_OBJ_P(return_value);
	int res = git_tree_lookup(&intern->tree, git_repo, &id);

	if (res != 0) {
		// TODO Throw exception
		RETURN_NULL();
	}
}


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

struct git2_treewalk_payload {
	zval *this;
	zval **callback_data;
	zend_fcall_info callback_i;
	zend_fcall_info_cache callback_ic;
};

static int git2_treewalk_cb(const char *root, const git_tree_entry *entry, void *payload) {
	int error;
	zval argv[3]; // root, entry, callback_data
	zval retval;
	struct git2_treewalk_payload *p = payload;

	ZVAL_STRING(&argv[0], root);
	git2_tree_entry_spawn(&argv[1], entry);
	ZVAL_COPY_VALUE(&argv[2], *p->callback_data);

	p->callback_i.retval = &retval;
	p->callback_i.param_count = 3;
	p->callback_i.params = argv;

	// TODO HERE
	error = zend_call_function(&p->callback_i, &p->callback_ic);
	if (error == FAILURE) {
		return -1; // causes end of walk
	} else if (!Z_ISUNDEF(retval)) {
		convert_to_long(&retval);
		error = Z_LVAL(retval);
		zval_ptr_dtor(&retval);
	}

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
	zval_ptr_dtor(&argv[2]); // ?
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_tree_walk, 0, 0, 2)
	ZEND_ARG_INFO(0, mode)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(1, callback_data)
ZEND_END_ARG_INFO()

static PHP_METHOD(Tree, walk) {
	long mode;
	zval **callback_data;
	struct git2_treewalk_payload p;
	p.this = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lfZ", &mode, &p.callback_i, &p.callback_ic, &callback_data) != SUCCESS) {
		return;
	}

	GIT2_TREE_FETCH();

	switch(mode) {
		case GIT_TREEWALK_PRE: case GIT_TREEWALK_POST:
			break;
		default:
			git2_throw_exception(0 TSRMLS_CC, "Invalid mode provided, should be Git2::TREEWALK_PRE or Git2::TREEWALK_POST");
			return;
	}

	int res = git_tree_walk(intern->tree, mode, git2_treewalk_cb, &p);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}

	RETURN_TRUE;
}

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
	PHP_ME(Tree, lookup_oid, arginfo_tree_id, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Tree, id, arginfo_tree_id, ZEND_ACC_PUBLIC)
	PHP_ME(Tree, entrycount, arginfo_tree_entrycount, ZEND_ACC_PUBLIC)
	PHP_ME(Tree, walk, arginfo_tree_walk, ZEND_ACC_PUBLIC)
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
#define GIT2_TREE_CONST(_x, _y) zend_declare_class_constant_long(php_git2_tree_ce, ZEND_STRL(#_x), _y TSRMLS_CC)
	
	GIT2_TREE_CONST(WALK_PRE, GIT_TREEWALK_PRE);
	GIT2_TREE_CONST(WALK_POST, GIT_TREEWALK_POST);
}

