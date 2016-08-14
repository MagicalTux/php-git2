#ifndef GIT2_CONFIG_ENTRY_H
#define GIT2_CONFIG_ENTRY_H

void git2_config_entry_init(TSRMLS_DC);

void git2_config_entry_spawn(zval **return_value, git_config_entry *e TSRMLS_DC);

#endif /* GIT2_CONFIG_ENTRY_H */
