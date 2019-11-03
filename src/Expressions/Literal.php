<?php

namespace Axiom\Ghost\Expressions;

use Axiom\Ghost\Token;
use Axiom\Ghost\Expressions\Expression;

class Literal extends Expression
{
    /**
	 * @var Object
	 */
	public $value;

    /**
     * Create a new Literal expression instance.
     * 
     * param  Object  $value
     */
    public function __construct(Object $value)
    {
        $this->value = $value;
    }
}