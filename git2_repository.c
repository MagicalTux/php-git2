#include "php_git2.h"
#include "git2_repository.h"

zend_class_entry *php_git2_repository_ce;

static zend_function_entry git2_repository_methods[] = {
/*	PHP_ME(Repository, __construct, arginfo___construct, ZEND_ACC_PUBLIC) */
	{ NULL, NULL, NULL }
};

void git2_repository_init() {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Repository", git2_repository_methods);
	php_git2_repository_ce = zend_register_internal_class(&ce TSRMLS_CC);
}

