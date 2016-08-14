#ifndef PHP_GIT2_H
#define PHP_GIT2_H

#define PHP_GIT2_EXTNAME "git2"
#define PHP_GIT2_EXTVER "0.1.0"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>
#include <SAPI.h>
#include <dlfcn.h>
#include <string.h>
#include "zend_exceptions.h"
#include "zend_interfaces.h"

#include "git2.h"
#include "git2/odb.h"
#include "git2/odb_backend.h"
#include "git2/trace.h"
#include "git2/sys/filter.h"
#include "git2/sys/odb_backend.h"

extern zend_module_entry git2_module_entry;
#define phpext_git2_ptr &git2_module_entry

//PHPAPI zend_class_entry *php_git2_get_repository(void);


#endif /* PHP_GIT2_H */
