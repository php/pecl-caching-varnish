--TEST--
Check for VarnishStat::getSnapshot functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

foreach (array_slice($args_all, 0, 2) as $args) {
	$vs = new VarnishStat($args);

	$stat = $vs->getSnapshot();

	echo (int)is_array($stat), "\n";
	echo (int)!empty($stat), "\n";
}

?>
--EXPECT--
1
1
1
1
