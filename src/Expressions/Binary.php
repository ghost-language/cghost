<?php

namespace Axiom\Ghost\Expressions;

use Axiom\Ghost\Token;
use Axiom\Ghost\Expressions\Expression;

class Binary extends Expression
{
    /**
	 * @var Expression
	 */
	public $left;

	/**
	 * @var Token
	 */
	public $operator;

	/**
	 * @var Expression
	 */
	public $right;

    /**
     * Create a new Binary expression instance.
     * 
     * param  Expression  $left
	 * param  Token  $operator
	 * param  Expression  $right
     */
    public function __construct(Expression $left, Token $operator, Expression $right)
    {
        $this->left = $left;
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
		return $visitor->visitBinary($this);
	}
}