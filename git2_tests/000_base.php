<?php

if (!extension_loaded('git2')) {
	echo "Unable to run tests without extension!\n";
	exit(1);
}

$repo = Git2\Repository::open(dirname(__DIR__));
var_dump($repo);
var_dump($repo->path());

var_dump($repo->head());
var_dump($repo->head()->name());

$config = $repo->config();
var_dump($config);
var_dump($config->export());

$last_commit = $repo->head()->target();
var_dump(bin2hex($last_commit));

//$commit = Git2\Commit::lookup_oid($repo, $last_commit);
$commit = $repo->head()->peel(Git2::OBJ_COMMIT); // this is actually equivalent to the previous command

var_dump($commit);
var_dump($commit->message());

$tree = $commit->tree();
var_dump($tree);
var_dump(bin2hex($tree->id()));
var_dump($tree->entrycount());


