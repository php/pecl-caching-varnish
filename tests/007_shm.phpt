--TEST--
Check for VarnishStat::getSnapshot functionality
--SKIPIF--
<?php print "skip VarnishStat and VarnishLog are not supported anymore"; ?>
<?php if(substr(PHP_OS, 0, 3) == "WIN") die("skip now for Windows"); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

$vs = new VarnishStat;

$stat = $vs->getSnapshot();

echo (int)is_array($stat), "\n";
echo (int)!empty($stat), "\n";

?>
--EXPECT--
1
1
