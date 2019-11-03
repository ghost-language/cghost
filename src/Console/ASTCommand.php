<?php

namespace Axiom\Ghost\Console;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

class ASTCommand extends Command
{
    public function configure()
    {
        $this->setName('ast')
            ->setDescription('Generate Ghost\'s abstract syntax tree');
    }

    public function execute(InputInterface $input, OutputInterface $output)
    {
        $output->writeln('Ghost Interpreter (PHP) -- Abstract Syntax Tree Generator');

        $expressions = [
            'Binary'   => ['Expression $left', 'Token $operator', 'Expression $right'],
            'Grouping' => ['Expression $expression'],
            'Literal'  => ['$value'],
            'Unary'    => ['Token $operator', 'Expression $right'],
        ];

        $stub = file_get_contents('stubs/expression.stub');

        foreach ($expressions as $class => $properties) {
            file_put_contents("src/Expressions/{$class}.php", $stub);
        }
    }
}