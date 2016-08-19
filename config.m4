PHP_ARG_ENABLE(git2, whether to enable the "git2" extension,
	[  --enable-git2           Enable "php-git2" extension support])

PHP_ARG_WITH(system-libgit2, whether to build with system libgit2,
	[  --with-system-libgit2   Build php-git2 with system libgit2], no, no)

if test $PHP_GIT2 != "no"; then
	if test "$PHP_SYSTEM_LIBGIT2" != "no"; then
		PHP_ADD_LIBRARY(git2,, GIT2_SHARED_LIBADD)
	else
		if ! test -f libgit2/build/libgit2.a; then
			echo "Please run ./libgit2_build.sh or configure with --with-system-libgit2"
			exit 1
		fi
		PHP_ADD_INCLUDE([libgit2/include])
		PHP_ADD_LIBRARY(curl,, GIT2_SHARED_LIBADD)
		PHP_ADD_LIBRARY(rt,, GIT2_SHARED_LIBADD)
		PHP_ADD_LIBRARY(http_parser,, GIT2_SHARED_LIBADD)
		PHP_ADD_LIBRARY(ssh2,, GIT2_SHARED_LIBADD)
		PHP_ADD_LIBRARY_WITH_PATH(git2, libgit2/build, GIT2_SHARED_LIBADD)
	fi

	PHP_NEW_EXTENSION(git2, php_git2.c git2_php_util.c git2_exception.c git2_repository.c git2_config.c git2_config_entry.c git2_reference.c git2_commit.c git2_remote.c git2_tree.c git2_tree_entry.c git2_blob.c git2_cred.c, $ext_shared)
	PHP_SUBST(GIT2_SHARED_LIBADD)
fi
