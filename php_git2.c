#include "php_git2.h"

#include "git2_repository.h"

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(git2) {
	git2_repository_init();

	return SUCCESS;
}
/* }}} */

