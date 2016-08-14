PHP_ARG_ENABLE(git2, Whether to enable the "git2" extension,
	[  --enable-git2           Enable "php-git2" extension support])

PHP_ARG_ENABLE(git2-static, to build with static libgit2,
	[  --enable-git2-static	  Build php-git2 with static libgit2], no, no)

if test $PHP_GIT2 != "no"; then
	PHP_SUBST(GIT2_SHARED_LIBADD)
	
	if test "$PHP_GIT2_STATIC" != "no"; then
		CFLAGS="$CFLAGS -Wl,libgit2/build/libgit2.a"
		PHP_ADD_INCLUDE([$ext_srcdir/libgit2/include])
		PHP_SUBST([CFLAGS])
	else
		PHP_ADD_LIBRARY(git2,, GIT2_SHARED_LIBADD)
	fi

	PHP_NEW_EXTENSION(git2, php_git2.c git2_repository.c git2_config.c git2_config_entry.c git2_reference.c git2_commit.c, $ext_shared)
fi
