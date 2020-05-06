<?php

function fib($n)
{
    if ($n < 2) return $n;
    return fib($n - 2) + fib($n - 1);
}

$start = microtime(TRUE);
echo (fib(35) == 9227465 ? 'true' : 'false')."\n";
$end = microtime(TRUE);

echo "elapsed:\n";
echo $end - $start."\n";