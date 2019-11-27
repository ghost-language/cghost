<?php

namespace Axiom\Ghost\Contracts;

use Axiom\Ghost\Expressions\Unary;
use Axiom\Ghost\Expressions\Binary;
use Axiom\Ghost\Expressions\Literal;
use Axiom\Ghost\Expressions\Grouping;


interface VisitsExpressions
{
    public function visitBinaryExpression(Binary $expression);
    public function visitGroupingExpression(Grouping $expression);
    public function visitLiteralExpression(Literal $expression);
    public function visitUnaryExpression(Unary $expression);
}