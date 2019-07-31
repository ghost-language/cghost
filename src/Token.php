<?php

namespace Axiom\Ghost;

class Token
{
    public $type;
    public $literal;
    public $line = 0;
    
    public function __construct($type, $literal, $line)
    {
        $this->type = $type;
        $this->literal = $literal;
        $this->line = $line;
    }
}