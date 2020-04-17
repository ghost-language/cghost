<?php

namespace Axiom\Ghost;

class SyntaxError
{
    public static $hadError = false;

    public static function error($token, $message)
    {
        if ($token->type === TOKEN_EOF) {
            self::report($token->line, ' at end', $message);
        } else {
            self::report($token->line, ' at \''.$token->literal.'\'', $message);
        }
    }

    public static function report($line, $where, $message)
    {
        print("[line $line] Error$where: $message\n");
        
        self::$hadError = true;
    }
}