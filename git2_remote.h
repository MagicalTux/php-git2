#ifndef GIT2_REMOTE_H
#define GIT2_REMOTE_H

void git2_remote_init(TSRMLS_DC);

void git2_remote_spawn(zval *return_value, git_remote *remote TSRMLS_DC);

#endif /* GIT2_REMOTE_H */
