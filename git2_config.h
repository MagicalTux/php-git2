#ifndef GIT2_CONFIG_H
#define GIT2_CONFIG_H

void git2_config_init(TSRMLS_DC);

void git2_config_spawn(zval *return_value, git_config *config TSRMLS_DC);

#endif /* GIT2_CONFIG_H */
