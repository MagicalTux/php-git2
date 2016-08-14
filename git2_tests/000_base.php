<?php

if (!extension_loaded('git2')) {
	echo "Unable to run tests without extension!\n";
	exit(1);
}

$test = Git2\Repository::open(dirname(__DIR__));
var_dump($test);
var_dump($test->head_detached());

