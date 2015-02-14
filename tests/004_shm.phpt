--TEST--
Check for varnish set/get params
--SKIPIF--
<?php if(substr(PHP_OS, 0, 3) == "WIN") die("skip now for Windows"); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$args = array();
$args['timeout'] = 1500;

$va = new VarnishAdmin($args);
$va->connect();
$va->auth();

$va->setParam("thread_pools", 20);
$vparams = $va->getParams();
echo (int)$vparams['thread_pools'] . "\n";

$va->setParam("thread_pools", 30);
$vparams = $va->getParams();
echo (int)$vparams['thread_pools'] . "\n";

$va->setParam("thread_pools", 40);
$vparams = $va->getParams();
echo (int)$vparams['thread_pools'] . "\n";

?>
--EXPECT--
20
30
40
