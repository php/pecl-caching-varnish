--TEST--
Check for varnish connection close
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists('tests/config.php')) print "skip"; ?>
--FILE--
<?php 

include 'tests/config.php';

foreach ($args_all as $args) {
	$r = true;

	$va = new VarnishAdmin($args);
	var_export($va->disconnect());
	echo "\n";
	$va->connect();
	var_export($va->disconnect());
	echo "\n";
}

?>
--EXPECT--
false
true
false
true
false
true
false
true
