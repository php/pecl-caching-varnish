--TEST--
Check for vcl.list
--SKIPIF--
<?php if(substr(PHP_OS, 0, 3) == "WIN") die("skip now for Windows"); ?>
<?php if (!extension_loaded("varnish") || !file_exists(dirname(__FILE__) . '/config.php')) print "skip"; ?>
<?php if (!getenv("VARNISH_TEST_SHM")) print "skip VARNISH_TEST_SHM not set"; ?>
--FILE--
<?php 

include dirname(__FILE__) . '/config.php';

$r = true;

$va = new VarnishAdmin;
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

?>
--EXPECT--
true
