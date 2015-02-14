--TEST--
Check for varnish connection
--SKIPIF--
<?php putenv('VARNISH_TEST_IPV4=1'); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_IPV4")) print "skip VARNISH_TEST_IPV4 not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$va = new VarnishAdmin($args_ipv4);
var_export($va->connect());
echo "\n";

?>
--EXPECT--
true
