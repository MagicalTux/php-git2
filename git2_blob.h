#ifndef GIT2_BLOB_H
#define GIT2_BLOB_H

void git2_blob_init(TSRMLS_DC);

void git2_blob_spawn(zval **return_value, git_blob *blob TSRMLS_DC);

#endif /* GIT2_BLOB_H */
