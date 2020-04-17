<?php

namespace Axiom\Ghost\Expressions;

abstract class Expression
{
    /**
     * @var
     */
    public $left;

    /**
     * @var
     */
    public $operator;

    /**
     * @var
     */
    public $right;

    /**
     * Create a new Expression instance.
     * 
     * @param  $left  mixed
     * @param  $operator  mixed
     * @param  $right  mixed
     */
    public function __construct($left, $operator, $right)
    {
        $this->left     = $left;
        $this->operator = $operator;
        $this->right    = $right;
    }
}