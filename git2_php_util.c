#include "php_git2.h"
#include "git2_php_util.h"
#include "git2_cred.h"

#define ARRAY_FETCH_LONG(_x) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_long(data); \
	opts->_x = Z_LVAL_P(data); \
}

#define ARRAY_FETCH_BOOL(_x) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	convert_to_boolean(data); \
	opts->_x = Z_LVAL_P(data); \
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

#define ARRAY_FETCH_CALLBACK(_x, _payload) if ((data = zend_hash_str_find(ht, ZEND_STRL(#_x))) != NULL) { \
	git2_callback_ ## _x ## _set(&opts->_x, &opts->_payload, data); \
}

// all remote callbacks have the same payload, so this makes a large structure
struct git2_remote_callbacks_payload {
	zval credentials_callback;
	zend_fcall_info_cache fci_cache;
};

int git2_callback_credentials_call(git_cred **cred, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload) {
	struct git2_remote_callbacks_payload *p = (struct git2_remote_callbacks_payload*)payload;
	if (Z_ISUNDEF(p->credentials_callback)) return 1; // ??

	zend_fcall_info fci;
	zval argv[3];
	zval retval;
	int error;

	ZVAL_STRING(&argv[0], url);
	ZVAL_STRING(&argv[1], username_from_url);
	ZVAL_LONG(&argv[2], allowed_types);

	fci.size = sizeof(fci);
	fci.function_table = EG(function_table);
	fci.object = NULL;
	ZVAL_COPY_VALUE(&fci.function_name, &p->credentials_callback);
	fci.retval = &retval;
	fci.param_count = 3;
	fci.params = argv;
	fci.no_separation = 0;
	fci.symbol_table = NULL;

	error = zend_call_function(&fci, &p->fci_cache);

	zval_ptr_dtor(&argv[0]);
	zval_ptr_dtor(&argv[1]);
	zval_ptr_dtor(&argv[2]);

	if (error == FAILURE) {
		php_error_docref(NULL, E_WARNING, "Callback for git credentials failed");
		return -1;
	} else if (!Z_ISUNDEF(retval)) {
		git_cred *c = git2_cred_take_from_zval(&retval); // will return NULL if not a cred object or used more than once
		if (c) {
			*cred = c;
			zval_ptr_dtor(&retval);
			return 0;
		}
		zval_ptr_dtor(&retval);
		return 1;
	} else {
		return 1;
	}
}

static void git2_callback_credentials_set(git_cred_acquire_cb *cb, void **payload, zval *callback) {
	struct git2_remote_callbacks_payload *p;
	if (*payload == NULL) {
		*payload = emalloc(sizeof(struct git2_remote_callbacks_payload));
		memset(*payload, 0, sizeof(struct git2_remote_callbacks_payload));
		p = (struct git2_remote_callbacks_payload*)*payload;
		p->fci_cache = empty_fcall_info_cache;
	} else {
		p = (struct git2_remote_callbacks_payload*)*payload;
	}

	if (!Z_ISUNDEF(p->credentials_callback)) {
		zval_ptr_dtor(&p->credentials_callback);
		p->fci_cache = empty_fcall_info_cache;
	}

	ZVAL_COPY(&p->credentials_callback, callback);
	*cb = git2_callback_credentials_call;
}

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

void git2_parse_fetch_options(git_fetch_options *opts, HashTable *ht) {
	zval *data;
	if (ht == NULL) return; // skip if null

	ARRAY_FETCH_OPTIONS(callbacks, git2_parse_remote_callbacks);
	ARRAY_FETCH_LONG(prune);
	ARRAY_FETCH_BOOL(update_fetchhead);
	ARRAY_FETCH_LONG(download_tags);
	ARRAY_FETCH_STRARRAY(custom_headers);
}

void git2_parse_remote_callbacks(git_remote_callbacks *opts, HashTable *ht) {
	zval *data;

	if (ht == NULL) return; // skip if null

	ARRAY_FETCH_CALLBACK(sideband_progress, payload);
	ARRAY_FETCH_CALLBACK(completion, payload);
	ARRAY_FETCH_CALLBACK(credentials, payload);
	ARRAY_FETCH_CALLBACK(certificate_check, payload);
	ARRAY_FETCH_CALLBACK(transfer_progress, payload);
	ARRAY_FETCH_CALLBACK(update_tips, payload);
	ARRAY_FETCH_CALLBACK(pack_progress, payload);
	ARRAY_FETCH_CALLBACK(push_transfer_progress, payload);
	ARRAY_FETCH_CALLBACK(push_update_reference, payload);
	ARRAY_FETCH_CALLBACK(push_negotiation, payload);
	ARRAY_FETCH_CALLBACK(transport, payload);
}

void git2_parse_push_options(git_push_options *opts, HashTable *ht) {
	zval *data;
	if (ht == NULL) return;
	
	ARRAY_FETCH_LONG(pb_parallelism);
	ARRAY_FETCH_OPTIONS(callbacks, git2_parse_remote_callbacks);
	ARRAY_FETCH_STRARRAY(custom_headers);
}

