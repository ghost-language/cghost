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