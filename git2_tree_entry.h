#ifndef GIT2_TREE_ENTRY_H
#define GIT2_TREE_ENTRY_H

void git2_tree_entry_init(TSRMLS_DC);

void git2_tree_entry_spawn(zval *return_value, git_tree_entry *e TSRMLS_DC);

#endif /* GIT2_TREE_ENTRY_H */
