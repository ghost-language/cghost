<?php

namespace Axiom\Ghost;

class Scanner
{
    /**
     * @var string
     */
    protected $source;

    /**
     * @var array
     */
    protected $tokens = [];

    /**
     * @var integer
     */
    protected $start = 0;

    /**
     * @var integer
     */
    protected $current = 0;

    /**
     * @var integer
     */
    protected $line = 1;

    /**
     * Set the source.
     * 
     * @param  string  $source
     * @return self
     */
    public function setSource($source)
    {
        $this->reset();

        $this->source = $source;

        return $this;
    }

    public function scanTokens()
    {
        while (! $this->isAtEndOfSource()) {
            $this->start = $this->current;

            $this->scanToken();
        }

        $this->addToken(TOKEN_EOF);

        return $this->tokens;
    }

    /**
     * Determine if we've reached the end of the source.
     * 
     * @return Boolean
     */
    protected function isAtEndOfSource()
    {
        return $this->current >= strlen($this->source);
    }

    /**
     * Scan the current character and extract the token type.
     * 
     * @return void
     */
    protected function scanToken()
    {
        $character = $this->advance();

        switch ($character) {
            case '(':
                $this->addToken(TOKEN_LEFT_PARENTHESIS);
                break;
            case ')':
                $this->addToken(TOKEN_RIGHT_PARENTHESIS);
                break;
            case '{':
                $this->addToken(TOKEN_LEFT_BRACE);
                break;
            case '}':
                $this->addToken(TOKEN_RIGHT_BRACE);
                break;
            case ',':
                $this->addToken(TOKEN_COMMA);
                break;
            case '.':
                $this->addToken(TOKEN_DOT);
                break;
            case '-':
                $this->addToken(TOKEN_MINUS);
                break;
            case '+':
                $this->addToken(TOKEN_PLUS);
                break;
            case ';':
                $this->addToken(TOKEN_SEMICOLON);
                break;
            case '*':
                $this->addToken(TOKEN_STAR);
                break;
            case '!':
                $this->addToken($this->match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
                break;
            case '=':
                $this->addToken($this->match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
                break;
            case '<':
                $this->addToken($this->match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
                break;
            case '>':
                $this->addToken($this->match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
                break;
            case '/':
                if ($this->match('/')) {
                    while ($this->peek() != '\n' and ! $this->isAtEndOfSource()) {
                        $this->advance();
                    }
                } else {
                    $this->addToken(TOKEN_SLASH);
                }

                break;
            case ' ';
            case '\r':
            case '\t':
                // Ignore whitespace
                break;
            case '\n':
                $this->line++;
                break;
            case '"':
                $this->string();
                break;
            default:
                //
        }
    }

    /**
     * Advance and return the next line of source code.
     * 
     * @return string
     */
    protected function advance()
    {
        $next = $this->source[$this->current];
        
        $this->current++;
        
        return $next;
    }

    /**
     * Add a new token type.
     * 
     * @param  constant  $type
     * @return void
     */
    protected function addToken($type, $literal = null)
    {
        $this->tokens[] = new Token($type, $literal, $this->line);
    }

    /**
     * Looks at the second character for an expected match.
     * 
     * @param  string  $expected
     * @return boolean
     */
    protected function match($expected)
    {
        if ($this->isAtEndOfSource()) {
            return false;
        }

        if ($this->source[$this->current] !== $expected) {
            return false;
        }

        $this->current++;
        
        return true;
    }

    /**
     * Peek at and return the next character.
     * 
     * @return string
     */
    protected function peek()
    {
        if ($this->isAtEndOfSource()) {
            return;
        }

        return $this->source[$this->current];
    }

    protected function string()
    {
        while ($this->peek() != '"' and ! $this->isAtEndOfSource()) {
            if ($this->peek() == '\n') {
                $this->line++;
            }
            
            $this->advance();
        }

        if ($this->isAtEndOfSource()) {
            // Log error, "Unterminated string"
            return;
        }

        $this->advance();

        $value = substr($this->source, $this->start + 1, $this->current - $this->start - 2);

        $this->addToken(TOKEN_STRING, $value);
    }

    /**
     * Reset the scanner.
     * 
     * @return void
     */
    protected function reset()
    {
        $this->source  = '';
        $this->tokens  = [];
        $this->start   = 0;
        $this->current = 0;
        $this->line    = 1;
    }
}