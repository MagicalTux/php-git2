#ifndef GIT2_CRED_H
#define GIT2_CRED_H

void git2_cred_init(TSRMLS_DC);

void git2_cred_spawn(zval *return_value, git_cred *cred TSRMLS_DC);

git_cred *git2_cred_extract_from_zval(zval *o);
git_cred *git2_cred_take_from_zval(zval *o);

#endif /* GIT2_CRED_H */
