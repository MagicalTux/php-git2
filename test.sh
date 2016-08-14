#!/bin/sh

PHP=`which php`
PHP_OPTS="-n -dextension_dir=modules -dextension=git2.so"

if [ "$#" -gt 0 ]; then
	exec "$PHP" $PHP_OPTS "$@"
	exit $?
fi

for foo in git2_tests/*.php; do
	echo "Running $foo"
	"$PHP" $PHP_OPTS "$foo"
	if [ $? != 0 ]; then
		echo "Tests have failed"
		exit 1
	fi
done

