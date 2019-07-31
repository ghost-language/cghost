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

    protected $start = 0;
    protected $current = 0;
    protected $line = 1;


    /**
     * Set the source.
     * 
     * @param  string  $source
     * @return self
     */
    public function setSource($source)
    {
        $this->source = $source;

        return $this;
    }

    public function scanTokens()
    {
        while (! $this->isAtEndOfSource()) {
            $this->start = $this->current;

            $this->scanToken();
        }

        $this->tokens[] = new Token(EOF, null, $this->line);

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

    protected function scanToken()
    {
        $character = $this->advance();

        switch ($character) {

        }
    }

    protected function advance()
    {
        $next = $this->source[$this->current];
        
        $this->current++;
        
        return $next;
    }
}