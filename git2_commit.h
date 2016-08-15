#ifndef GIT2_COMMIT_H
#define GIT2_COMMIT_H

void git2_commit_init(TSRMLS_DC);
void git2_commit_spawn(zval **return_value, git_commit *commit TSRMLS_DC);

#endif /* GIT2_COMMIT_H */
