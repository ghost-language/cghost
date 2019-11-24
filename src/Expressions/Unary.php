<?php

namespace Axiom\Ghost\Expressions;

use Axiom\Ghost\Token;
use Axiom\Ghost\Expressions\Expression;

class Unary extends Expression
{
    /**
	 * @var Token
	 */
	public $operator;

	/**
	 * @var Expression
	 */
	public $right;

    /**
     * Create a new Unary expression instance.
     * 
     * param  Token  $operator
	 * param  Expression  $right
     */
    public function __construct(Token $operator, Expression $right)
    {
        $this->operator = $operator;
		$this->right = $right;
    }

    /**
	 * Accept a visitor instance.
	 *
	 * @param  Visitor  $visitor
	 * @return Visitor
	*/
	public function accept($visitor)
	{
		return $visitor->visitUnary($this);
	}
}