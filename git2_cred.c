#include "php_git2.h"
#include "git2_exception.h"
#include "git2_cred.h"
#include "git2_php_util.h"

static zend_class_entry *php_git2_cred_ce;
static zend_object_handlers php_git2_cred_handler;

typedef struct _git2_cred_object {
	zend_object std;
	git_cred *cred;
} git2_cred_object_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_create_default, 0, 0, 0)
ZEND_END_ARG_INFO()

git_cred *git2_cred_extract_from_zval(zval *zv) {
	if (Z_TYPE_P(zv) != IS_OBJECT) return NULL;
	if (Z_OBJCE_P(zv) != php_git2_cred_ce) return NULL;

	git2_cred_object_t *intern = (git2_cred_object_t*)Z_OBJ_P(zv);
	return intern->cred;
}

git_cred *git2_cred_take_from_zval(zval *zv) {
	git_cred *cred;
	if (Z_TYPE_P(zv) != IS_OBJECT) return NULL;
	if (Z_OBJCE_P(zv) != php_git2_cred_ce) return NULL;

	git2_cred_object_t *intern = (git2_cred_object_t*)Z_OBJ_P(zv);
	cred = intern->cred;
	intern->cred = NULL;
	return cred;
}

static PHP_METHOD(Cred, create_default) {
	if (zend_parse_parameters_none() == FAILURE) return;
	git2_cred_object_t *intern;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	int res = git_cred_default_new(&intern->cred);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_create_plaintext, 0, 0, 2)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

static PHP_METHOD(Cred, create_plaintext) {
	char *username, *password;
	size_t username_len, password_len;
	git2_cred_object_t *intern;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &username, &username_len, &password, &password_len) == FAILURE)
		return;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	int res = git_cred_userpass_plaintext_new(&intern->cred, username, password);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_create_ssh_key, 0, 0, 3)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, publickey_file)
	ZEND_ARG_INFO(0, privatekey_file)
	ZEND_ARG_INFO(0, passphrase)
ZEND_END_ARG_INFO()

static PHP_METHOD(Cred, create_ssh_key) {
	char *username, *publickey_file, *privatekey_file, *passphrase = NULL;
	size_t username_len, publickey_file_len, privatekey_file_len, passphrase_len;
	git2_cred_object_t *intern;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &username, &username_len, &publickey_file, &publickey_file_len, &privatekey_file, &privatekey_file_len, &passphrase, &passphrase_len) == FAILURE)
		return;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	int res = git_cred_ssh_key_new(&intern->cred, username, publickey_file, privatekey_file, passphrase);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_create_ssh_key_memory, 0, 0, 3)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, publickey)
	ZEND_ARG_INFO(0, privatekey)
	ZEND_ARG_INFO(0, passphrase)
ZEND_END_ARG_INFO()

static PHP_METHOD(Cred, create_ssh_key_memory) {
	char *username, *publickey, *privatekey, *passphrase = NULL;
	size_t username_len, publickey_len, privatekey_len, passphrase_len;
	git2_cred_object_t *intern;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss|s", &username, &username_len, &publickey, &publickey_len, &privatekey, &privatekey_len, &passphrase, &passphrase_len) == FAILURE)
		return;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	int res = git_cred_ssh_key_memory_new(&intern->cred, username, publickey, privatekey, passphrase);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_create_ssh_key_from_agent, 0, 0, 1)
	ZEND_ARG_INFO(0, username)
ZEND_END_ARG_INFO()

static PHP_METHOD(Cred, create_ssh_key_from_agent) {
	char *username;
	size_t username_len;
	git2_cred_object_t *intern;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &username, &username_len) == FAILURE)
		return;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	int res = git_cred_ssh_key_from_agent(&intern->cred, username);

	if (res != 0) {
		git2_throw_last_error();
		return;
	}
}

#define GIT2_CRED_FETCH() git2_cred_object_t *intern = (git2_cred_object_t*)Z_OBJ_P(getThis()); \
	if (intern->cred == NULL) { \
		git2_throw_exception(0 TSRMLS_CC, "Git2\\Cred object in invalid state"); \
		return; \
	}

#define GIT2_CRED_GET_BOOL(_x) ZEND_BEGIN_ARG_INFO_EX(arginfo_cred_ ## _x, 0, 0, 0) \
	ZEND_END_ARG_INFO() \
	static PHP_METHOD(Cred, _x) { \
		if (zend_parse_parameters_none() == FAILURE) return; \
		GIT2_CRED_FETCH(); \
		RETURN_BOOL(git_cred_ ## _x(intern->cred)); \
	}

GIT2_CRED_GET_BOOL(has_username)

void git2_cred_spawn(zval *return_value, git_cred *cred TSRMLS_DC) {
	git2_cred_object_t *intern;

	object_init_ex(return_value, php_git2_cred_ce);
	intern = (git2_cred_object_t*)Z_OBJ_P(return_value);
	intern->cred = cred;
}

zend_object *php_git2_cred_create_object(zend_class_entry *class_type TSRMLS_DC) {
	git2_cred_object_t *intern = NULL;

	intern = emalloc(sizeof(git2_cred_object_t));
	memset(intern, 0, sizeof(git2_cred_object_t));

	zend_object_std_init(&intern->std, class_type TSRMLS_CC);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_git2_cred_handler;

	return &intern->std;
}

static void php_git2_cred_free_object(zend_object *object TSRMLS_DC) {
	git2_cred_object_t *intern = (git2_cred_object_t*)object;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	if (intern->cred) {
		git_cred_free(intern->cred);
		intern->cred = NULL;
	}

	// no need with PHP7 to free intern
}

#define PHP_GIT2_CRED_ME_P(_x) PHP_ME(Cred, _x, arginfo_cred_##_x, ZEND_ACC_PUBLIC)

static zend_function_entry git2_cred_methods[] = {
	PHP_ME(Cred, create_default, arginfo_cred_create_default, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Cred, create_plaintext, arginfo_cred_create_plaintext, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Cred, create_ssh_key, arginfo_cred_create_ssh_key, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Cred, create_ssh_key_memory, arginfo_cred_create_ssh_key_memory, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Cred, create_ssh_key_from_agent, arginfo_cred_create_ssh_key_from_agent, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_GIT2_CRED_ME_P(has_username)
/*	PHP_ME(Cred, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_cred_init(TSRMLS_D) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Cred", git2_cred_methods);
	php_git2_cred_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_cred_ce->create_object = php_git2_cred_create_object;

	memcpy(&php_git2_cred_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_cred_handler.clone_obj = NULL;
	php_git2_cred_handler.free_obj = php_git2_cred_free_object;

#define GIT2_CRED_CONST(_x) zend_declare_class_constant_long(php_git2_cred_ce, ZEND_STRL(#_x), GIT_CRED ## _x TSRMLS_CC)

	GIT2_CRED_CONST(TYPE_USERPASS_PLAINTEXT);
	GIT2_CRED_CONST(TYPE_SSH_KEY);
	GIT2_CRED_CONST(TYPE_SSH_CUSTOM);
	GIT2_CRED_CONST(TYPE_DEFAULT);
	GIT2_CRED_CONST(TYPE_SSH_INTERACTIVE);
	GIT2_CRED_CONST(TYPE_USERNAME);
	GIT2_CRED_CONST(TYPE_SSH_MEMORY);
}

