<?php

if (!extension_loaded('git2')) {
	echo "Unable to run tests without extension!\n";
	exit(1);
}

$test = Git2\Repository::open(dirname(__DIR__));
var_dump($test);
var_dump($test->head_detached());
var_dump($test->head_unborn());
var_dump($test->is_empty());
var_dump($test->path());
var_dump($test->workdir());
var_dump($test->is_bare());
var_dump($test->state());

var_dump($test->head());
var_dump($test->head()->name());
var_dump(bin2hex($test->head()->target()));

