#ifndef GIT2_REFERENCE_H
#define GIT2_REFERENCE_H

void git2_reference_init(TSRMLS_DC);

void git2_reference_spawn(zval **return_value, git_reference *ref TSRMLS_DC);

#endif /* GIT2_REFERENCE_H */
