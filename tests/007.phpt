--TEST--
Check for varnish status functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists('tests/config.php')) print "skip"; ?>
--FILE--
<?php 

include 'tests/config.php';

$vs = new VarnishStat($args_all['args_ident']);

$stat = $vs->getSnapshot();

echo (int)is_array($stat), "\n";
echo (int)!empty($stat);

?>
--EXPECT--
1
1
