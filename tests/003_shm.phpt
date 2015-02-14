--TEST--
Check for varnish authentication
--SKIPIF--
<?php if(substr(PHP_OS, 0, 3) == "WIN") die("skip now for Windows"); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

/* XXX test exceptions */

$va = new VarnishAdmin;
$va->connect();
var_export($va->auth());
echo "\n";

?>
--EXPECT--
true
