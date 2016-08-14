<?php

if (!extension_loaded('git2')) {
	echo "Unable to run tests without extension!\n";
	exit(1);
}

$test = Git2\Repository::open(dirname(__DIR__));
var_dump($test);
var_dump($test->path());

var_dump($test->head());
var_dump($test->head()->name());

$last_commit = $test->head()->target();
var_dump(bin2hex($last_commit));

$commit = Git2\Commit::lookup_oid($test, $last_commit);
var_dump($commit);
var_dump($commit->message());

