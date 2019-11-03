<?php

namespace Axiom\Ghost;

class Token
{
    /**
     * @var
     */
    public $type;

    /**
     * @var
     */
    public $literal;

    /**
     * @var
     */
    public $line = 0;
    
    /**
     * Create a new Token instance.
     * 
     * @param  $type  constant
     * @param  $literal  mixed
     * @param  $line  integer
     */
    public function __construct($type, $literal, $line)
    {
        $this->type    = $type;
        $this->literal = $literal;
        $this->line    = $line;
    }

    /**
     * Return a string representation of the Token class.
     * 
     * @return string
     */
    public function __toString()
    {
        $string = "{$this->type}";

        if (! is_null($this->literal)) {
            $string .= " ({$this->literal})";
        }

        return $string;
    }
}