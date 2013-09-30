--TEST--
Check for the basic VarnishLog::getLine() functionality
--SKIPIF--
<?php if(substr(PHP_OS, 0, 3) == "WIN") die("skip now for Windows"); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

$i = 3;
$vs = new VarnishLog;
while($i-- > 0) {
		$line = $vs->getLine();
		echo (int)is_array($line), "\n";
		echo (int)!empty($line), "\n";
}

?>
--EXPECT--
1
1
1
1
1
1
