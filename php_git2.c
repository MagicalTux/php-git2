#include "php_git2.h"

#include "git2_repository.h"

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(git2) {
	git2_repository_init();

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(git2) {
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

