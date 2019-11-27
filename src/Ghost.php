<?php

namespace Axiom\Ghost;

class Ghost
{
    protected $scanner;

    public function __construct()
    {
        $this->scanner = new Scanner;
    }

    /**
     * Execute the passed source code.
     * 
     * @param  string  $source
     * @return mixed
     */
    public function execute($source)
    {
        $this->scanner->setSource($source);

        $tokens     = $this->scanner->scanTokens();
        $parser     = new Parser($tokens);
        $expression = $parser->parse();

        if (SyntaxError::$hadError) {
            return;
        }

        return $expression;
    }
}