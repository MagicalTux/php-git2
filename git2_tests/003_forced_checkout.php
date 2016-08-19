<?php

$tmp = sys_get_temp_dir().'/php-git2_test_ssh2';

$repo = Git2\Repository::init($tmp, false);

try {
	$remote = Git2\Remote::lookup_name($repo, 'origin');
} catch(\Exception $e) {
	$remote = Git2\Remote::create_with_fetchspec($repo, 'origin', 'git@github.com:MagicalTux/php-git2.git', '+refs/*:refs/*');
}

$fetch_opts = [
	'callbacks' => [
		'credentials' => function($url, $username, $types) {
			return Git2\Cred::create_ssh_key($username, __DIR__.'/test_key.pub', __DIR__.'/test_key');
		},
	],
	'update_fetchhead' => false,
	'download_tags' => Git2\Remote::DOWNLOAD_TAGS_ALL,
];

echo "Fetching repository...\n";
var_dump($remote->fetch(NULL, $fetch_opts));

$cfg = $repo->config();
$cfg->set_string('branch.master.remote', 'origin');
$cfg->set_string('branch.master.merge', 'refs/heads/master');

$repo->set_head('refs/heads/master');

echo "Checking out...\n";
var_dump($repo->checkout_head(['checkout_strategy' => \Git2::CHECKOUT_FORCE]));

