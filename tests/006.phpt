--TEST--
Check for varnish status functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

foreach ($args_all as $args) {
	$args['timeout'] = 3000;

	$va = new VarnishAdmin($args);
	$va->connect();
	$va->auth();

	$va->stop();
	echo (int)$va->isRunning(), "\n";
	sleep(2);

	$va->start();
	echo (int)$va->isRunning(), "\n";
	sleep(2);
}

?>
--EXPECT--
0
1
0
1
0
1
0
1
