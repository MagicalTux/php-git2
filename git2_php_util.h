#ifndef GIT2_PHP_UTIL_H
#define GIT2_PHP_UTIL_H

void php_git2_ht_to_strarray(git_strarray *out, HashTable *in);
void php_git2_strarray_free(git_strarray *a);

void git2_parse_repository_init_options(git_repository_init_options *opts, HashTable *ht);
void git2_parse_clone_options(git_clone_options *opts, HashTable *ht);
void git2_parse_checkout_options(git_checkout_options *opts, HashTable *ht);
void git2_parse_fetch_options(git_fetch_options *opts, HashTable *ht);
void git2_parse_remote_callbacks(git_remote_callbacks *opts, HashTable *ht);
void git2_parse_push_options(git_push_options *opts, HashTable *ht);

#endif /* GIT2_PHP_UTIL_H */
