<?php

namespace Axiom\Ghost\Statements;

abstract class Statement
{
    abstract public function accept($visitor);
}