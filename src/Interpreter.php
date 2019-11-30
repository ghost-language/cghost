<?php

namespace Axiom\Ghost;

use Axiom\Ghost\Token;
use Axiom\Ghost\Expressions\Unary;
use Axiom\Ghost\Expressions\Binary;
use Axiom\Ghost\Expressions\Literal;
use Axiom\Ghost\Expressions\Grouping;
use Axiom\Ghost\Exceptions\RuntimeError;
use Axiom\Ghost\Contracts\VisitsStatements;
use Axiom\Ghost\Contracts\VisitsExpressions;

class Interpreter implements VisitsExpressions, VisitsStatements
{
    public function interpret($expression)
    {
        try {
            $value = $this->evaluate($expression);

            print $this->stringify($value)."\n";
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
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) > doubleval($right);
            case TOKEN_GREATER_EQUAL:
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) >= doubleval($right);
            case TOKEN_LESS:
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) < doubleval($right);
            case TOKEN_LESS_EQUAL:
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) <= doubleval($right);
            case TOKEN_MINUS:
                $this->checkNumberOperand($expression->operator, $right);

                return doubleval($left) - doubleval($right);
            case TOKEN_PLUS:
                if (is_double($left) and is_double($right)) {
                    return doubleval($left) + doubleval($right);
                }

                if (is_string($left) and is_string($right)) {
                    return $left.''.$right;
                }

                throw new RuntimeError($expression->operator, "Operands must be two numbers or two strings.");
            case TOKEN_SLASH:
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) / doubleval($right);
            case TOKEN_STAR:
                $this->checkNumberOperands($expression->operator, $left, $right);

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
     * Checks the given number operand is a valid double value.
     *
     * @param  Token  $operator
     * @param  mixed  $operand
     * @return
     * @throws \Axiom\Ghost\Exceptions\RuntimeError
     */
    protected function checkNumberOperand(Token $operator, $operand)
    {
        if (is_double($operand)) {
            return;
        }

        throw new RuntimeError("Operand must be a number.");
    }

        /**
     * Checks the given left and right operands are valid double values.
     *
     * @param  Token  $operator
     * @param  mixed  $left
     * @param  mixed  $right
     * @return
     * @throws \Axiom\Ghost\Exceptions\RuntimeError
     */
    protected function checkNumberOperands($operator, $left, $right)
    {
        if (is_double($left) and is_double($right)) {
            return;
        }

        throw new RuntimeError("Operands must be numbers.");
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

    protected function stringify($object)
    {
        if (is_null($object)) {
            return 'null';
        }

        return (string) $object;
    }
}