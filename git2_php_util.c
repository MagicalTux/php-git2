#include "php_git2.h"
#include "git2_php_util.h"

#define ARRAY_FETCH_LONG(_x) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_long(data); \
	opts->_x = Z_LVAL_P(data); \
}

#define ARRAY_FETCH_BOOL(_x) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_bool(data); \
	opts->_x = Z_BVAL_P(data); \
}

#define ARRAY_FETCH_STRING(_x) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_string(data); \
	opts->_x = Z_STRVAL_P(data); \
}

#define ARRAY_FETCH_OPTIONS(_x, _method) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_array(data); \
	_method(&opts->_x, Z_ARRVAL_P(data)); \
}

#define ARRAY_FETCH_STRARRAY(_x) ARRAY_FETCH_OPTIONS(_x, php_git2_ht_to_strarray)

#define ARRAY_FETCH_CALLBACK(_x, _payload) /* TODO */

void php_git2_ht_to_strarray(git_strarray *out, HashTable *in) {
	uint32_t count = zend_array_count(in);
	out->count = count;
	if (count == 0) return;

	out->strings = emalloc(sizeof(char*) * count);
	uint32_t cur_pos = 0;

	HashPosition position;
	zval *data = NULL;

	for (zend_hash_internal_pointer_reset_ex(in, &position);
	    data = zend_hash_get_current_data_ex(in, &position);
	    zend_hash_move_forward_ex(in, &position)) {

	    convert_to_string(data);
	    out->strings[cur_pos] = Z_STRVAL_P(data);
	    cur_pos += 1;
	}
}

void php_git2_strarray_free(git_strarray *a) {
	if (a->count > 0) {
		efree(a->strings);
	}
}

void git2_parse_repository_init_options(git_repository_init_options *opts, HashTable *ht) {
	zval *data;

	if (ht == NULL) return; // skip if null

	ARRAY_FETCH_LONG(flags);
	ARRAY_FETCH_LONG(mode);
	ARRAY_FETCH_STRING(workdir_path); // check open_basedir?
	ARRAY_FETCH_STRING(description);
	ARRAY_FETCH_STRING(template_path); // check open_basedir?
	ARRAY_FETCH_STRING(initial_head);
	ARRAY_FETCH_STRING(origin_url);
}

void git2_parse_clone_options(git_clone_options *opts, HashTable *ht) {
	zval *data;

	if (ht == NULL) return; // skip if null

	ARRAY_FETCH_OPTIONS(checkout_opts, git2_parse_checkout_options);
	ARRAY_FETCH_OPTIONS(fetch_opts, git2_parse_fetch_options);
	ARRAY_FETCH_BOOL(bare);
	ARRAY_FETCH_LONG(local); // one of GIT_CLONE_LOCAL_AUTO, GIT_CLONE_LOCAL, GIT_CLONE_NO_LOCAL, GIT_CLONE_LOCAL_NO_LINKS
	ARRAY_FETCH_STRING(checkout_branch);
	ARRAY_FETCH_CALLBACK(repository_cb, repository_cb_payload); // TODO + _payload
	ARRAY_FETCH_CALLBACK(remote_cb, remote_cb_payload); // TODO + _payload
}

void git2_parse_checkout_options(git_checkout_options *opts, HashTable *ht) {
	zval *data;

	if (ht == NULL) return; // skip if null

	ARRAY_FETCH_LONG(checkout_strategy);
	ARRAY_FETCH_BOOL(disable_filters);
	ARRAY_FETCH_LONG(dir_mode);
	ARRAY_FETCH_LONG(file_mode);
	ARRAY_FETCH_LONG(file_open_flags);
	ARRAY_FETCH_LONG(notify_flags);
	ARRAY_FETCH_CALLBACK(notify_cb, notify_payload);
	ARRAY_FETCH_CALLBACK(progress_cb, progress_payload);
	ARRAY_FETCH_STRARRAY(paths);
	// git_tree *baseline;
	// git_index *baseline_index;
	ARRAY_FETCH_STRING(target_directory);
	ARRAY_FETCH_STRING(ancestor_label);
	ARRAY_FETCH_STRING(our_label);
	ARRAY_FETCH_STRING(their_label);
	ARRAY_FETCH_CALLBACK(perfdata_cb, perfdata_payload);
}

