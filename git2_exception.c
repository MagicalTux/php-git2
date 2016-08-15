#include "php_git2.h"
#include "git2_exception.h"
#include <ext/spl/spl_exceptions.h>

zend_class_entry *php_git2_exception_ce;

void git2_throw_exception(zend_long code TSRMLS_DC, const char *format, ...) {
	va_list args;
	char *message;
	int message_len;

	va_start(args, format);
	message_len = vspprintf(&message, 0, format, args);
	zend_throw_exception(php_git2_exception_ce, message, code TSRMLS_CC);
	efree(message);
	va_end(args);
}

static zend_function_entry php_git2_exception_me[] = {
	PHP_FE_END
};

void git2_exception_init(TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "Git2", "Exception", php_git2_exception_me);
	php_git2_exception_ce = zend_register_internal_class_ex(&ce, spl_ce_RuntimeException);
}

