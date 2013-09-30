--TEST--
Check for the basic VarnishLog::getLine() functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SECRET")) print "skip VARNISH_TEST_SECRET not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

$i = 3;
$vs = new VarnishLog($args_ident);
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
