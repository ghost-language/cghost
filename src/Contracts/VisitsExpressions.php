<?php

namespace Axiom\Ghost\Contracts;

use Axiom\Ghost\Expressions\UnaryExpression;
use Axiom\Ghost\Expressions\BinaryExpression;
use Axiom\Ghost\Expressions\LiteralExpression;
use Axiom\Ghost\Expressions\GroupingExpression;

interface VisitsExpressions
{
    public function visitBinaryExpression(BinaryExpression $expression);
    public function visitGroupingExpression(GroupingExpression $expression);
    public function visitLiteralExpression(LiteralExpression $expression);
    public function visitUnaryExpression(UnaryExpression $expression);
}