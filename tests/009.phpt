--TEST--
Check for vcl.list
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists('tests/config.php')) print "skip"; ?>
--FILE--
<?php 

include 'tests/config.php';

foreach ($args_all as $args) {
	$r = true;

	$va = new VarnishAdmin($args);
	$va->connect();
	$va->auth();

	$list = $va->getVclList();
	$r = $r && (count($list) > 0);
	$one_active = false;
	while ($r && (list(, $data) = each($list)) !== false) {
		$r = $r && (3 == count($data));

		$r = $r && isset($data['status']);
		$r = $r && isset($data['locks']);
		$r = $r && isset($data['name']);

		if ('active' == $data['status']) {	
			$one_active = true;
		}
	}

	$r = $r && $one_active;

	var_export($r);
	echo "\n";
}

?>
--EXPECT--
true
true
true
true
