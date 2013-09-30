--TEST--
Check for varnish stop/start
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$args = array();
$args['timeout'] = 2000;
$va = new VarnishAdmin($args);
$va->connect();
$va->auth();

echo $va->stop() . "\n";
sleep(2);
echo $va->start() . "\n";
sleep(2);

?>
--EXPECT--
200
200
