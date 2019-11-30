<?php

namespace Axiom\Ghost;

class Ghost
{
    protected $scanner;
    protected $interpreter;

    public function __construct()
    {
        $this->scanner     = new Scanner;
        $this->interpreter = new Interpreter;
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

        $this->interpreter->interpret($expression);
    }
}