--TEST--
Check for vcl.use
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

foreach ($args_all as $args) {
	$r = true;

	$va = new VarnishAdmin($args);
	$va->connect();
	$va->auth();

	$list = $va->getVclList();
	$name = $list[0]['name']; // thats enough to get the first best name

	$r = $va->vclUse($name);
	var_export($r);
	echo "\n";
}

?>
--EXPECT--
true
true
true
true
