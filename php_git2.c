#include "php_git2.h"

#include "git2_exception.h"
#include "git2_repository.h"
#include "git2_config.h"
#include "git2_config_entry.h"
#include "git2_reference.h"
#include "git2_commit.h"
#include "git2_remote.h"
#include "git2_tree.h"
#include "git2_tree_entry.h"
#include "git2_blob.h"
#include "git2_cred.h"

static zend_class_entry *php_git2_base_ce;
static zend_object_handlers php_git2_base_handler;

static zend_function_entry git2_base_methods[] = {
	{ NULL, NULL, NULL }
};

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(git2) {
	// check version
	int git2_major, git2_minor, git2_rev;
	git_libgit2_version(&git2_major, &git2_minor, &git2_rev);
	if ((git2_major != LIBGIT2_VER_MAJOR) || (git2_minor != LIBGIT2_VER_MINOR) || (git2_rev != LIBGIT2_VER_REVISION)) {
		zend_error(E_WARNING, "Compiled libgit2 and linked libgit2 do not match");
		return FAILURE;
	}

	git_libgit2_init();

	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "Git2", git2_base_methods);
	php_git2_base_ce = zend_register_internal_class(&ce TSRMLS_CC);
	php_git2_base_ce->create_object = NULL;

	memcpy(&php_git2_base_handler, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_git2_base_handler.clone_obj = NULL;

#define GIT2_BASE_CONST(_x) zend_declare_class_constant_long(php_git2_base_ce, ZEND_STRL(#_x), GIT_ ## _x TSRMLS_CC)
	// object types (types.h)
	GIT2_BASE_CONST(OBJ_ANY);
	GIT2_BASE_CONST(OBJ_BAD);
	GIT2_BASE_CONST(OBJ_COMMIT);
	GIT2_BASE_CONST(OBJ_TREE);
	GIT2_BASE_CONST(OBJ_BLOB);
	GIT2_BASE_CONST(OBJ_TAG);
	GIT2_BASE_CONST(OBJ_OFS_DELTA);
	GIT2_BASE_CONST(OBJ_REF_DELTA);

	// filemodes
	GIT2_BASE_CONST(FILEMODE_UNREADABLE);
	GIT2_BASE_CONST(FILEMODE_TREE);
	GIT2_BASE_CONST(FILEMODE_BLOB);
	GIT2_BASE_CONST(FILEMODE_BLOB_EXECUTABLE);
	GIT2_BASE_CONST(FILEMODE_LINK);
	GIT2_BASE_CONST(FILEMODE_COMMIT);

	// certs
	GIT2_BASE_CONST(CERT_NONE);
	GIT2_BASE_CONST(CERT_X509);
	GIT2_BASE_CONST(CERT_HOSTKEY_LIBSSH2);
	GIT2_BASE_CONST(CERT_STRARRAY);

	git2_exception_init(TSRMLS_C);
	git2_repository_init(TSRMLS_C);
	git2_config_init(TSRMLS_C);
	git2_config_entry_init(TSRMLS_C);
	git2_reference_init(TSRMLS_C);
	git2_commit_init(TSRMLS_C);
	git2_remote_init(TSRMLS_C);
	git2_tree_init(TSRMLS_C);
	git2_tree_entry_init(TSRMLS_C);
	git2_blob_init(TSRMLS_C);
	git2_cred_init(TSRMLS_C);

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(git2) {
	git_libgit2_shutdown();
	return SUCCESS;
}

#ifdef COMPILE_DL_GIT2
ZEND_GET_MODULE(git2)
#endif

/* {{{ counter_module_entry
 */
zend_module_entry git2_module_entry = {
	STANDARD_MODULE_HEADER,
	"git2",
	NULL,
	PHP_MINIT(git2),
	PHP_MSHUTDOWN(git2),
	NULL,
	NULL,
	NULL,
	PHP_GIT2_EXTVER,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

