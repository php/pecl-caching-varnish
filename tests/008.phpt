--TEST--
Check for the basic VarnishLog::getLine() functionality
--SKIPIF--
<?php if (!extension_loaded("varnish") || !file_exists('tests/config.php')) print "skip"; ?>
--FILE--
<?php 

include 'tests/config.php';

$i = 3;
$vs = new VarnishLog($args_all['args_ident']);
while($i-- > 0) {
        $line = $vs->getLine();
        echo (int)is_array($line), "\n";
        echo (int)!empty($line), "\n";
};

?>
--EXPECT--
1
1
1
1
1
1
