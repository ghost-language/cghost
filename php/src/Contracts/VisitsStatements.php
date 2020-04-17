<?php

namespace Axiom\Ghost\Contracts;

use Axiom\Ghost\Statements\PrintStatement;
use Axiom\Ghost\Statements\ExpressionStatement;

interface VisitsStatements
{
    function visitPrintStatement(PrintStatement $statement);
    function visitExpressionStatement(ExpressionStatement $statement);
}