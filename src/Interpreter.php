<?php

namespace Axiom\Ghost;

use Axiom\Ghost\Expressions\Unary;
use Axiom\Ghost\Expressions\Binary;
use Axiom\Ghost\Expressions\Literal;
use Axiom\Ghost\Expressions\Grouping;
use Axiom\Ghost\Contracts\VisitsStatements;
use Axiom\Ghost\Contracts\VisitsExpressions;

class Interpretor implements VisitsExpressions, VisitsStatements
{
    public function interpret($expression)
    {
        try {

        } catch (RuntimeError $error) {
            // Catch error
        }
    }

    /**
     * Evaluate literal expressions.
     * 
     * @param  Literal  $expression
     */
    public function visitLiteralExpression(Literal $expression)
    {
        return $expression->value;
    }

    /**
     * Evaluate grouping expressions.
     * 
     * @param  Grouping  $expression
     */
    public function visitGroupingExpression(Grouping $expression)
    {
        return $this->evaluate($expression->expression);
    }

    /**
     * Evaluate unary expressions.
     * 
     * @param  Unary  $expression
     */
    public function visitUnaryExpression(Unary $expression)
    {
        $right = $this->evaluate($expression->right);

        switch ($expression->operator->type) {
            case TOKEN_BANG:
                return ! $this->isTruthy($right);
            case TOKEN_MINUS:
                return -doubleval($right);
        }

        // Unreachable
        return null;
    }

    /**
     * Evaluate binary expressions.
     * 
     * @param  Binary  $expression
     */
    public function visitBinaryExpression(Binary $expression)
    {
        $left  = $this->evaluate($expression->left);
        $right = $this->evaluate($expression->right);

        switch ($expression->operator->type) {
            case TOKEN_GREATER:
                return doubleval($left) > doubleval($right);
            case TOKEN_GREATER_EQUAL:
                return doubleval($left) >= doubleval($right);
            case TOKEN_LESS:
                return doubleval($left) < doubleval($right);
            case TOKEN_LESS_EQUAL:
                return doubleval($left) <= doubleval($right);
            case TOKEN_MINUS:
                return doubleval($left) - doubleval($right);
            case TOKEN_PLUS:
                if (is_double($left) and is_double($right)) {
                    return doubleval($left) + doubleval($right);
                }

                if (is_string($left) and is_string($right)) {
                    return $left.''.$right;
                }
            case TOKEN_SLASH:
                return doubleval($left) / doubleval($right);
            case TOKEN_STAR:
                return doubleval($left) * doubleval($right);
            case TOKEN_BANG_EQUAL:
                return ! $this->isEqual($left, $right);
            case TOKEN_EQUAL_EQUAL:
                return $this->isEqual($left, $right);
        }

        // Unreachable
        return null;
    }

    /**
     * Send the expression back into the interpreter's
     * visitor implementation.
     * 
     * @param  Expression  $expression
     */
    protected function evaluate($expression)
    {
        return $expression->accept($this);
    }

    /**
     * Determines if the given object is truthy.
     * 
     * @param  mixed  $object
     * @return bool
     */
    protected function isTruthy($object)
    {
        if ($object === null) {
            return false;
        }

        if (is_bool($object)) {
            return boolval($object);
        }

        return true;
    }

    /**
     * Determines if the given left and right values
     * are equal.
     * 
     * @param  mixed  $left
     * @param  mixed  $right
     * @return bool
     */
    protected function isEqual($left, $right)
    {
        if (is_null($left) and is_null($right)) {
            return true;
        }

        if (is_null($left)) {
            return false;
        }

        return $left == $right;
    }
}