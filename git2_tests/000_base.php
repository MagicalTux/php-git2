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

$cb = function($root, $entry) use ($repo) {
	$fullname = $root.$entry->name();
	echo " + $fullname (".bin2hex($entry->id()).")\n";
	if ($fullname == 'README.md') {
		// fetch contents, compare with actual README.md (will fail if uncommitted changes exist in file)
		$blob = Git2\Blob::lookup_oid($repo, $entry->id());
		$data = $blob->rawcontent();
		$actual_data = file_get_contents(dirname(__DIR__).'/README.md');
		if ($data == $actual_data) {
			echo " `- TEST OK\n";
		} else {
			echo " `- TEST FAILED ! (data different, uncommitted changes?)\n";
			exit(1);
		}
	}
	return 0;
};

$tree->walk(Git2\Tree::WALK_POST, $cb);


