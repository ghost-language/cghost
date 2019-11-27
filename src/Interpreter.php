<?php

namespace Axiom\Ghost;

use Axiom\Ghost\Expressions\Literal;
use Axiom\Ghost\Contracts\VisitsStatements;
use Axiom\Ghost\Contracts\VisitsExpressions;

class Interpretor implements VisitsExpressions, VisitsStatements
{
    public function interpret($expression)
    {
        try {

        } catch (RuntimeError $error) {
            // Catch error
        }
    }

    public function visitLiteralExpression(Literal $expression)
    {
        return $expression->value;
    }
}