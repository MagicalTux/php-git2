<?php

$tmp = sys_get_temp_dir().'/php-git2_test';

if (file_exists($tmp)) {
	echo "Skipping test - Remove $tmp to run test - rm -fr ".escapeshellarg($tmp)."\n";
	exit(0); // skip test
}

echo "Cloning repository...\n";
$repo = Git2\Repository::clone('https://github.com/MagicalTux/php-git2.git', $tmp);

#var_dump($repo->checkout_head());
var_dump($repo->head());

