--TEST--
Check for varnish status functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_IPV6")) print "skip VARNISH_TEST_IPV6 not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$args_ipv6['timeout'] = 3000;

$va = new VarnishAdmin($args_ipv6);
$va->connect();
$va->auth();

$va->stop();
echo (int)$va->isRunning(), "\n";
sleep(2);

$va->start();
echo (int)$va->isRunning(), "\n";
sleep(2);

?>
--EXPECT--
0
1
