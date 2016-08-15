<?php

$tmp = sys_get_temp_dir().'/php-git2_test';

$repo = Git2\Repository::init_ext($tmp, ['flags' => Git2\Repository::INIT_MKDIR]);
if (!$repo) {
	echo "Failed to initialize repo in $tmp\n";
	exit(1);
}

// config repo
$config = $repo->config();
$config->set_string('branch.master.remote', 'origin');
$config->set_string('branch.master.merge', 'refs/heads/master');

echo "Fetching remote...\n";
$remote = Git2\Remote::create_anonymous($repo, 'https://github.com/MTYoujou/php-git2.git');
var_dump($remote->connect(false));
var_dump($remote->fetch(['+refs/*', 'refs/*']));
var_dump($repo->checkout_head());

