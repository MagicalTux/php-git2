#ifndef GIT2_EXCEPTION_H
#define GIT2_EXCEPTION_H

void git2_exception_init(TSRMLS_D);
void git2_throw_exception(zend_long code TSRMLS_DC, const char *format, ...);
zend_bool git2_throw_last_error(TSRMLS_D);

extern zend_class_entry *php_git2_exception_ce;

#endif /* GIT2_EXCEPTION_H */
