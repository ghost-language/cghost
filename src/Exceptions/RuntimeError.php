<?php

namespace Axiom\Ghost\Exceptions;

use Exception;
use Axiom\Ghost\Token;

class RuntimeError extends Exception
{
    /**
     * @var \Axiom\Ghost\Token
     */
    public $token;

    public function __construct(Token $token, $message)
    {
        parent::__construct($message);

        $this->token = $token;
    }
}