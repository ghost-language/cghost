<?php

namespace Axiom\Ghost;

class Scanner
{
    /**
     * @var string
     */
    protected $source;

    /**
     * @var array
     */
    protected $tokens = [];

    /**
     * @var integer
     */
    protected $start = 0;

    /**
     * @var integer
     */
    protected $current = 0;

    /**
     * @var integer
     */
    protected $line = 1;

    /**
     * @var array
     */
    protected $keywords = [
        'and'      => TOKEN_AND,
        'class'    => TOKEN_CLASS,
        'else'     => TOKEN_ELSE,
        'false'    => TOKEN_FALSE,
        'for'      => TOKEN_FOR,
        'function' => TOKEN_FUNCTION,
        'if'       => TOKEN_IF,
        'let'      => TOKEN_LET,
        'null'     => TOKEN_NULL,
        'or'       => TOKEN_OR,
        'print'    => TOKEN_PRINT,
        'return'   => TOKEN_RETURN,
        'parent'   => TOKEN_PARENT,
        'this'     => TOKEN_THIS,
        'true'     => TOKEN_TRUE,
        'while'    => TOKEN_WHILE,
    ];

    /**
     * Set the source.
     * 
     * @param  string  $source
     * @return self
     */
    public function setSource($source)
    {
        $this->reset();

        $this->source = $source;

        return $this;
    }

    /**
     * Scan all tokens in source code.
     * 
     * @return array
     */
    public function scanTokens()
    {
        while (! $this->isAtEndOfSource()) {
            $this->start = $this->current;

            $this->scanToken();
        }

        $this->addToken(TOKEN_EOF);

        return $this->tokens;
    }

    /**
     * Determine if we've reached the end of the source.
     * 
     * @return Boolean
     */
    protected function isAtEndOfSource()
    {
        return $this->current >= strlen($this->source);
    }

    /**
     * Scan the current character and extract the token type.
     * 
     * @return void
     */
    protected function scanToken()
    {
        $character = $this->advance();

        switch ($character) {
            case '(':
                $this->addToken(TOKEN_LEFT_PARENTHESIS);
                break;
            case ')':
                $this->addToken(TOKEN_RIGHT_PARENTHESIS);
                break;
            case '{':
                $this->addToken(TOKEN_LEFT_BRACE);
                break;
            case '}':
                $this->addToken(TOKEN_RIGHT_BRACE);
                break;
            case ',':
                $this->addToken(TOKEN_COMMA);
                break;
            case '.':
                $this->addToken(TOKEN_DOT);
                break;
            case '-':
                $this->addToken(TOKEN_MINUS);
                break;
            case '+':
                $this->addToken(TOKEN_PLUS);
                break;
            case ';':
                $this->addToken(TOKEN_SEMICOLON);
                break;
            case '*':
                $this->addToken(TOKEN_STAR);
                break;
            case '!':
                $this->addToken($this->match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
                break;
            case '=':
                $this->addToken($this->match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
                break;
            case '<':
                $this->addToken($this->match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
                break;
            case '>':
                $this->addToken($this->match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
                break;
            case '/':
                if ($this->match('/')) {
                    while ($this->peek() != '\n' and ! $this->isAtEndOfSource()) {
                        $this->advance();
                    }
                } else {
                    $this->addToken(TOKEN_SLASH);
                }

                break;
            case ' ';
            case '\r':
            case '\t':
                // Ignore whitespace
                break;
            case '\n':
                $this->line++;
                break;
            case '"':
                $this->string();
                break;
            case ($this->isNumeric($character)):
                $this->number();
                break;
            case ($this->isAlpha($character)):
                $this->identifier();
                break;
            default:
                // Report error
        }
    }

    /**
     * Advance and return the next line of source code.
     * 
     * @return string
     */
    protected function advance()
    {
        $next = $this->source[$this->current];
        
        $this->current++;
        
        return $next;
    }

    /**
     * Add a new token type.
     * 
     * @param  constant  $type
     * @return void
     */
    protected function addToken($type, $literal = null)
    {
        $this->tokens[] = new Token($type, $literal, $this->line);
    }

    /**
     * Looks at the second character for an expected match.
     * 
     * @param  string  $expected
     * @return boolean
     */
    protected function match($expected)
    {
        if ($this->isAtEndOfSource()) {
            return false;
        }

        if ($this->source[$this->current] !== $expected) {
            return false;
        }

        $this->current++;
        
        return true;
    }

    /**
     * Peek at and return the current character.
     * 
     * @return string
     */
    protected function peek()
    {
        if ($this->isAtEndOfSource()) {
            return;
        }

        return $this->source[$this->current];
    }

    /**
     * Peek at and return the next character.
     * 
     * @return string
     */
    protected function peekNext()
    {
        $next = $this->current + 1;
        
        if ($next >= strlen($this->source)) {
            return;
        }

        return $this->source[$next];
    }

    /**
     * Identify and extract string token type value.
     * 
     * @return void
     */
    protected function string()
    {
        while ($this->peek() != '"' and ! $this->isAtEndOfSource()) {
            if ($this->peek() == '\n') {
                $this->line++;
            }

            $this->advance();
        }

        if ($this->isAtEndOfSource()) {
            // Log error, "Unterminated string"
            return;
        }

        $this->advance();

        $value = substr($this->source, $this->start + 1, $this->current - $this->start - 2);

        $this->addToken(TOKEN_STRING, $value);
    }

    /**
     * Determine if the referenced character is numberic.
     * (0 - 9)
     * 
     * @param  string  $character
     * @return boolean
     */
    protected function isNumeric($character)
    {
        $isNumeric = is_numeric($character);
        
        return $isNumeric;
    }

    /**
     * Determine if the referenced character is alphabetic.
     * (a - z, A - Z, _)
     * 
     * @param  string  $character
     * @return boolean
     */
    protected function isAlpha($character)
    {
        return ($character >= 'a' and $character <= 'z') or
            ($character >= 'A' and $character <= 'Z') or
            $character == '_';
    }

    /**
     * Determine if the referenced character is alphanumeric.
     * (a - z, A - Z, _, 0 - 9)
     * 
     * @param  string  $character
     * @return boolean
     */
    protected function isAlphaNumeric($character)
    {
        return $this->isAlpha($character) or $this->isNumeric($character);
    }

    /**
     * Identify and extract number token type value.
     * 
     * @return void
     */
    protected function number()
    {
        while ($this->isNumeric($this->peek())) {
            $this->advance();
        }
        
        if ($this->peek() == '.' and $this->isNumeric($this->peekNext())) {
            // Consume the "."
            $this->advance();

            while ($this->isNumeric($this->peek())) {
                $this->advance();
            }
        }

        $value = substr($this->source, $this->start, $this->current - $this->start);

        $this->addToken(TOKEN_NUMBER, $value);
    }

    /**
     * Identify and extract identifiers and reserved keywords.
     * 
     * @return void
     */
    protected function identifier()
    {
        while ($this->isAlphaNumeric($this->peek())) {
            $this->advance();
        }

        $type = TOKEN_IDENTIFIER;
        $text = substr($this->source, $this->start, $this->current - $this->start);

        if (isset($this->keywords[$text])) {
            $type = $this->keywords[$text];
        }

        $this->addToken($type);
    }

    /**
     * Reset the scanner.
     * 
     * @return void
     */
    protected function reset()
    {
        $this->source  = '';
        $this->tokens  = [];
        $this->start   = 0;
        $this->current = 0;
        $this->line    = 1;
    }
}