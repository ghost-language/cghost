<?php

function dd(...$args)
{
    print_r($args);
    die();
}