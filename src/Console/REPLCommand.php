<?php

namespace Axiom\Ghost\Console;

use Axiom\Ghost\Ghost;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;

class REPLCommand extends Command
{
    protected $ghost;

    public function __construct(Ghost $ghost)
    {
        $this->ghost = $ghost;

        parent::__construct();
    }

    public function configure()
    {
        $this->setName('repl')
            ->setDescription('Ghost\'s interactive shell');
    }

    public function execute(InputInterface $input, OutputInterface $output)
    {
        $output->writeln('Ghost Interpreter (PHP) -- Interactive Console v1.0');
        $output->writeln('===================================================');
        $output->writeln('// Type "<comment>help</comment>" or "<comment>license</comment>" for more information.');
        $output->writeln('// Type "<comment>exit</comment>" to leave the interactive console.');
        $output->writeln('');

        $this->waitForUserInput($input, $output);
    }

    protected function waitForUserInput(InputInterface $input, OutputInterface $output)
    {
        $helper = $this->getHelper('question');
        $question = new Question('<info>>>> </info>');

        $statement = $helper->ask($input, $output, $question);

        $this->listenForConsoleCommands($input, $output, $statement);

        $result   = $this->ghost->execute($statement);

        dd($result);

        // Temporary output during development
        $messages = ['Scanned tokens:'];

        foreach ($result as $token) {
            $messages[] = "<info>$token</info>";
        }

        $this->writeOutput($messages, $output);
        
        $this->waitForUserInput($input, $output);
    }

    protected function listenForConsoleCommands(InputInterface $input, OutputInterface $output, $statement)
    {
        if ($statement === 'exit') {
            $output->writeln('Exiting...');
            die();
        }

        if ($statement === 'help') {
            $output->writeln('Help...');
            $output->writeln('');

            $this->waitForUserInput($input, $output);
        }

        if ($statement === 'license') {
            $output->writeln('Copyright (c) 2019 Shea Lewis. All rights reserved.');
            $output->writeln('');
            $output->writeln('This work is licensed under the terms of the MIT license.');
            $output->writeln('For a copy, see <https://opensource.org/licenses/MIT>.');
            $output->writeln('');

            $this->waitForUserInput($input, $output);
        }
    }

    protected function writeOutput($messages, $output)
    {
        if (! is_array($messages)) {
            $messages = [$messages];
        }

        $output->writeln('');

        foreach ($messages as $message) {
            $output->writeln("    $message");
        }
        
        $output->writeln('');
    }
}