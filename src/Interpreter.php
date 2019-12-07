<?php

namespace Axiom\Ghost;

use Axiom\Ghost\Token;
use Axiom\Ghost\Expressions\Expression;
use Axiom\Ghost\Exceptions\RuntimeError;
use Axiom\Ghost\Statements\PrintStatement;
use Axiom\Ghost\Contracts\VisitsStatements;
use Axiom\Ghost\Contracts\VisitsExpressions;
use Axiom\Ghost\Expressions\UnaryExpression;
use Axiom\Ghost\Expressions\BinaryExpression;
use Axiom\Ghost\Expressions\LiteralExpression;
use Axiom\Ghost\Expressions\GroupingExpression;
use Axiom\Ghost\Statements\ExpressionStatement;

class Interpreter implements VisitsExpressions, VisitsStatements
{
    public function interpret(array $statements)
    {
        try {
            foreach ($statements as $statement) {
                $this->execute($statement);
            }
        } catch (RuntimeError $error) {
            // Catch error
        }
    }

    /**
     * Evaluate literal expressions.
     * 
     * @param  LiteralExpression  $expression
     */
    public function visitLiteralExpression(LiteralExpression $expression)
    {
        return $expression->value;
    }

    /**
     * Evaluate grouping expressions.
     * 
     * @param  GroupingExpression  $expression
     */
    public function visitGroupingExpression(GroupingExpression $expression)
    {
        return $this->evaluate($expression->expression);
    }

    /**
     * Evaluate unary expressions.
     * 
     * @param  UnaryExpression  $expression
     */
    public function visitUnaryExpression(UnaryExpression $expression)
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
     * @param  BinaryExpression  $expression
     */
    public function visitBinaryExpression(BinaryExpression $expression)
    {
        $left  = $this->evaluate($expression->left);
        $right = $this->evaluate($expression->right);

        switch ($expression->operator->type) {
            case TOKEN_GREATER:
                // $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) > doubleval($right);
            case TOKEN_GREATER_EQUAL:
                // $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) >= doubleval($right);
            case TOKEN_LESS:
                // $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) < doubleval($right);
            case TOKEN_LESS_EQUAL:
                $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) <= doubleval($right);
            case TOKEN_MINUS:
                // $this->checkNumberOperand($expression->operator, $right);

                return doubleval($left) - doubleval($right);
            case TOKEN_PLUS:
                // if (is_double($left) and is_double($right)) {
                    return doubleval($left) + doubleval($right);
                // }

                // if (is_string($left) and is_string($right)) {
                //     return $left.''.$right;
                // }

                throw new RuntimeError($expression->operator, "Operands must be two numbers or two strings.");
            case TOKEN_SLASH:
                // $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) / doubleval($right);
            case TOKEN_STAR:
                // $this->checkNumberOperands($expression->operator, $left, $right);

                return doubleval($left) * doubleval($right);
            case TOKEN_BANG_EQUAL:
                return ! $this->isEqual($left, $right);
            case TOKEN_EQUAL_EQUAL:
                return $this->isEqual($left, $right);
        }

        // Unreachable
        return null;
    }

    public function visitPrintStatement(PrintStatement $statement)
    {
        $value = $this->evaluate($statement->expression);

        print($this->stringify($value)."\n");

        return null;
    }

    public function visitExpressionStatement(ExpressionStatement $statement)
    {
        $this->evaluate($statement->expression);
        
        return null;
    }

    /**
     * Send the expression back into the interpreter's
     * visitor implementation.
     * 
     * @param  Expression  $expression
     */
    protected function evaluate(Expression $expression)
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

        throw new RuntimeError($operator, "Operand must be a number.");
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

        throw new RuntimeError($operator, "Operands must be numbers.");
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

        if ($object === true) {
            return 'true';
        }

        if ($object === false) {
            return 'false';
        }

        return (string) $object;
    }

    protected function execute($statement)
    {
        $statement->accept($this);
    }
}