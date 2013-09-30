--TEST--
Check for varnish set/get params
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SECRET")) print "skip VARNISH_TEST_SECRET not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$args_ident['timeout'] = 1500;

$va = new VarnishAdmin($args_ident);
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
