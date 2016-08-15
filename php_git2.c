#include "php_git2.h"

#include "git2_exception.h"
#include "git2_repository.h"
#include "git2_config.h"
#include "git2_config_entry.h"
#include "git2_reference.h"
#include "git2_commit.h"
#include "git2_remote.h"

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

/* {{{ PHP_MINIT_FUNCTION */
PHP_MINIT_FUNCTION(git2) {
	// check version
	int git2_major, git2_minor, git2_rev;
	git_libgit2_version(&git2_major, &git2_minor, &git2_rev);
	if ((git2_major != LIBGIT2_VER_MAJOR) || (git2_minor != LIBGIT2_VER_MINOR) || (git2_rev != LIBGIT2_VER_REVISION)) {
		zend_error(E_WARNING, "Compiled libgit2 and linked libgit2 do not match");
		return FAILURE;
	}

	git_libgit2_init();

	git2_exception_init(TSRMLS_CC);
	git2_repository_init(TSRMLS_CC);
	git2_config_init(TSRMLS_CC);
	git2_config_entry_init(TSRMLS_CC);
	git2_reference_init(TSRMLS_CC);
	git2_commit_init(TSRMLS_CC);
	git2_remote_init(TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

PHP_MSHUTDOWN_FUNCTION(git2) {
	git_libgit2_shutdown();
	return SUCCESS;
}

#ifdef COMPILE_DL_GIT2
ZEND_GET_MODULE(git2)
#endif

/* {{{ counter_module_entry
 */
zend_module_entry git2_module_entry = {
	STANDARD_MODULE_HEADER,
	"git2",
	NULL,
	PHP_MINIT(git2),
	PHP_MSHUTDOWN(git2),
	NULL,
	NULL,
	NULL,
	PHP_GIT2_EXTVER,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

