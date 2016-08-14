#ifndef GIT2_REPOSITORY_H
#define GIT2_REPOSITORY_H

void git2_repository_init(TSRMLS_DC);

git_repository *git2_repository_fetch_from_zval(zval *);
zend_class_entry *git2_reference_class_entry();

#endif /* GIT2_REPOSITORY_H */
