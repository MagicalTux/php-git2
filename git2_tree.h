#ifndef GIT2_TREE_H
#define GIT2_TREE_H

void git2_tree_init(TSRMLS_DC);

void git2_tree_spawn(zval **return_value, git_tree *tree TSRMLS_DC);

#endif /* GIT2_TREE_H */
