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
            'Binary' => [
                ['Expression', '$left'],
                ['Token', '$operator'],
                ['Expression', '$right'],
            ],
            'Grouping' => [
                ['Expression', '$expression'],
            ],
            'Literal' => [
                ['Object', '$value'],
            ],
            'Unary' => [
                ['Token', '$operator'],
                ['Expression', '$right'],
            ],
        ];

        $stub = file_get_contents('stubs/expression.stub');

        foreach ($expressions as $class => $properties) {
            $content = $stub;

            $replace = [
                '{class}'       => $class,
                '{properties}'  => $this->generateProperties($properties),
                '{docblock}'    => $this->generateDocBlock($properties),
                '{parameters}'  => $this->generateParameters($properties),
                '{definitions}' => $this->generateDefinitions($properties),
                '{accept}'      => $this->generateAcceptFunction($class),
            ];

            foreach ($replace as $find => $replace) {
                $content = str_replace($find, $replace, $content);
            }

            file_put_contents("src/Expressions/{$class}.php", $content);
        }

        $output->writeln('<comment>Complete.</comment>');
    }

    private function generateProperties($properties)
    {
        $string = '';

        foreach ($properties as list($type, $variable)) {
            $string .= "\t/**\n";
            $string .= "\t * @var {$type}\n";
            $string .= "\t */\n";
            $string .= "\tpublic {$variable};\n\n";
        }

        return trim($string);
    }

    private function generateDocBlock($properties)
    {
        $string = '';

        foreach ($properties as list($type, $variable)) {
            $string .= "\t * param  {$type}  {$variable}\n";
        }

        return trim($string);
    }

    private function generateParameters($properties)
    {
        $string = '';

        foreach ($properties as list($type, $variable)) {
            $string .= "{$type} {$variable}, ";
        }

        return rtrim(trim($string), ',');
    }

    private function generateDefinitions($properties)
    {
        $string = '';

        foreach ($properties as list($type, $variable)) {
            $reference = str_replace('$', '', $variable);

            $string .= "\t\t\$this->{$reference} = {$variable};\n";
        }

        return trim($string);
    }

    private function generateAcceptFunction($class)
    {
        $string = '';

        $string .= "\t/**\n";
        $string .= "\t * Accept a visitor instance.\n";
        $string .= "\t *\n";
        $string .= "\t * @param  Visitor  \$visitor\n";
        $string .= "\t * @return Visitor\n";
        $string .= "\t*/\n";
        $string .= "\tpublic function accept(\$visitor)\n";
        $string .= "\t{\n";
        $string .= "\t\treturn \$visitor->visit{$class}(\$this);\n";
        $string .= "\t}";

        return trim($string);
    }
}