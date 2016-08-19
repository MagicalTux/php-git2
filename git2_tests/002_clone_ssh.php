<?php

$tmp = sys_get_temp_dir().'/php-git2_test_ssh';

if (file_exists($tmp)) {
	echo "Skipping test - Remove $tmp to run test - rm -fr ".escapeshellarg($tmp)."\n";
	exit(0); // skip test
}

$options = [
	'fetch_opts' => [
		'callbacks' => [
			'credentials' => function($url, $username, $types) {
				return Git2\Cred::create_ssh_key($username, __DIR__.'/test_key.pub', __DIR__.'/test_key');
			},
		],
	],
];

echo "Cloning repository...\n";
$repo = Git2\Repository::clone('git@github.com:MagicalTux/php-git2.git', $tmp, $options);

#var_dump($repo->checkout_head());
var_dump($repo->head());

