<?php

namespace Axiom\Ghost;

use ParseError;
use Axiom\Ghost\Expressions\Unary;
use Axiom\Ghost\Expressions\Binary;
use Axiom\Ghost\Expressions\Literal;
use Axiom\Ghost\Expressions\Grouping;

class Parser
{
    /**
     * @var array
     */
    protected $tokens = [];

    /**
     * @var integer
     */
    protected $current = 0;

    /**
     * Create a new Parser instance.
     * 
     * @param  array  $tokens
     */
    public function __construct($tokens)
    {
        $this->tokens = $tokens;
    }

    public function parse()
    {
        try {
            return $this->expression();
        } catch (ParseError $error) {
            return null;
        }
    }

    protected function expression()
    {
        return $this->equality();
    }

    protected function equality()
    {
        $expression = $this->comparison();

        while ($this->match(TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL)) {
            $operator   = $this->previous();
            $right      = $this->comparison();
            $expression = new Binary($expression, $operator, $right);
        }

        return $expression;
    }

    protected function comparison()
    {
        $expression = $this->addition();

        while ($this->match(TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LESS, TOKEN_LESS_EQUAL)) {
            $operator   = $this->previous();
            $right      = $this->addition();
            $expression = new Binary($expression, $operator, $right);
        }

        return $expression;
    }

    protected function addition()
    {
        $expression = $this->multiplication();

        while ($this->match(TOKEN_MINUS, TOKEN_PLUS)) {
            $operator   = $this->previous();
            $right      = $this->multiplication();
            $expression = new Binary($expression, $operator, $right);
        }

        return $expression;
    }

    protected function multiplication()
    {
        $expression = $this->unary();

        while ($this->match(TOKEN_SLASH, TOKEN_STAR)) {
            $operator   = $this->previous();
            $right      = $this->unary();
            $expression = new Binary($expression, $operator, $right);
        }

        return $expression;
    }

    protected function unary()
    {
        if ($this->match(TOKEN_BANG, TOKEN_MINUS)) {
            $operator   = $this->previous();
            $right      = $this->unary();
            $expression = new Unary($operator, $right);
        }

        return $this->primary();
    }

    protected function primary()
    {
        if ($this->match(TOKEN_FALSE)) {
            return new Literal(false);
        }

        if ($this->match(TOKEN_TRUE)) {
            return new Literal(true);
        }

        if ($this->match(TOKEN_NULL)) {
            return new Literal(null);
        }

        if ($this->match(TOKEN_NUMBER, TOKEN_STRING)) {
            return new Literal($this->previous()->literal);
        }

        if ($this->match(TOKEN_LEFT_PARENTHESIS)) {
            $expression = $this->expression();
            
            $this->consume(TOKEN_RIGHT_PARENTHESIS, "Expect ')' after expression.");

            return new Grouping($expression);
        }

        throw $this->error($this->peek(), 'Expect expression.');
    }

    protected function consume($tokenType, $message)
    {
        if ($this->check($tokenType)) {
            return $this->advance();
        }

        throw $this->error($this->peek(), $message);
    }

    protected function match(...$tokenTypes)
    {
        foreach ($tokenTypes as $type) {
            if ($this->check($type)) {
                $this->advance();

                return true;
            }
        }

        return false;
    }

    protected function check($tokenType)
    {
        if ($this->isAtEnd()) {
            return false;
        }

        return $this->peek()->type == $tokenType;
    }

    protected function advance()
    {
        if (! $this->isAtEnd()) {
            $this->current++;
        }

        return $this->previous();
    }

    protected function isAtEnd()
    {
        return $this->peek()->type === TOKEN_EOF;
    }

    protected function peek()
    {
        return $this->tokens[$this->current];
    }

    protected function previous()
    {
        return $this->tokens[$this->current - 1];
    }

    protected function synchronize()
    {
        $this->advance();

        while (! $this->isAtEnd()) {
            if ($this->previous()->type === TOKEN_SEMICOLON) {
                return;
            }

            switch ($this->peek()) {
                case TOKEN_CLASS:
                case TOKEN_FUNCTION:
                case TOKEN_LET:
                case TOKEN_FOR:
                case TOKEN_IF:
                case TOKEN_WHILE:
                case TOKEN_PRINT:
                case TOKEN_RETURN:
                    return;
            }

            $this->advance();
        }
    }

    protected function error($token, $message)
    {
        SyntaxError::error($token, $message);

        return new ParseError;
    }
}