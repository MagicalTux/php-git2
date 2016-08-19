# libgit2 bindings for PHP 7

Because I need libgit2 and the implementation at https://github.com/libgit2/php-git doesn't appear maintained anymore (and no PHP7 implementation currently exists) I decided to put one together quickly with simple classes close to the original libgit2 syntax.

This implementation __requires__ PHP7 and implements classes unlike the libgit2 version, because why not.

All the methods I needed are now implemented, however if you need anything missing you can open an issue. Eventually all methods will be implemented, probably.

## Requirements

PHP 7.0 or above.

## Installation

It is highly recommended to link libgit2 statically so when it is updated you will not need to build this package again. Not rebuilding would lead to strange behavior and errors if we didn't check the version at initialization and refused to load when version doesn't match. Linking statically libgit2 is trivial and prevents most issues.

By default the configure script will use the local libgit2, so you just need to build it. This can be accomplished by running:

  $ ./libgit2_build.sh

You can then build php-git2 as you would any extension:

  $ phpize
  $ ./configure
  $ make
  $ make install

To use your system's libgit2, run configure with the `--with-system-libgit2` flag.
