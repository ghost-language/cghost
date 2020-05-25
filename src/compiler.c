#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/ghost.h"
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "scanner.h"

#if DEBUG_PRINT_CODE
    #include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY,
} Precedence;

typedef void (*ParseFn)(GhostVM *vm, bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
    bool isCaptured;
} Local;

typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

typedef enum {
    TYPE_FUNCTION,
    TYPE_CONSTRUCTOR,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    struct Compiler* enclosing;
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    Upvalue upvalues[UINT8_COUNT];
    int scopeDepth;
} Compiler;

typedef struct ClassCompiler {
    struct ClassCompiler* enclosing;
    Token name;
    bool hasSuperclass;
} ClassCompiler;

Parser parser;

Compiler* current = NULL;

ClassCompiler* currentClass = NULL;

static Chunk* currentChunk() {
    return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;

    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);

    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();

        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();

        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;

    advance();

    return true;
}

static void emitByte(GhostVM *vm, uint8_t byte) {
    writeChunk(vm, currentChunk(), byte, parser.previous.line);
}

static void emitBytes(GhostVM *vm, uint8_t byte1, uint8_t byte2) {
    emitByte(vm, byte1);
    emitByte(vm, byte2);
}

static void emitLoop(GhostVM *vm, int loopStart) {
    emitByte(vm, OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large.");

    emitByte(vm, (offset >> 8) & 0xff);
    emitByte(vm, offset & 0xff);
}

static int emitJump(GhostVM *vm, uint8_t instruction) {
    emitByte(vm, instruction);
    emitByte(vm, 0xff);
    emitByte(vm, 0xff);

    return currentChunk()->count - 2;
}

static void emitReturn(GhostVM *vm)
{
    if (current->type == TYPE_CONSTRUCTOR) {
        emitBytes(vm, OP_GET_LOCAL, 0);
    } else {
        emitByte(vm, OP_NULL);
    }

    emitByte(vm, OP_RETURN);
}

static uint8_t makeConstant(GhostVM *vm, Value value) {
    int constant = addConstant(vm, currentChunk(), value);

    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");

        return 0;
    }

    return (uint8_t) constant;
}

static void emitConstant(GhostVM *vm, Value value) {
    emitBytes(vm, OP_CONSTANT, makeConstant(vm, value));
}

static void patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(GhostVM *vm, Compiler* compiler, FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(vm);
    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(vm, parser.previous.start, parser.previous.length);
    }

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;

    if (type != TYPE_FUNCTION) {
        local->name.start = "this";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

static ObjFunction* endCompiler(GhostVM *vm) {
    emitReturn(vm);
    ObjFunction* function = current->function;

    #if DEBUG_PRINT_CODE
        if (!parser.hadError) {
            disassembleChunk(currentChunk(),
            function->name != NULL ? function->name->chars : "<script>");
        }
    #endif

    current = current->enclosing;
    return function;
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope(GhostVM *vm) {
    current->scopeDepth--;

    // When multiple local variables go out of scope at once, you
    // get a series of OP_POP instructions which get interpreted
    // one at a time. A simple optimization to make is to add a
    // specialized OP_POPN instruction that takes an operand for
    // the number of slots to pop and pop them all at once.
    while (current->localCount > 0 && current->locals[current->localCount - 1].depth > current->scopeDepth) {
        if (current->locals[current->localCount - 1].isCaptured) {
            emitByte(vm, OP_CLOSE_UPVALUE);
        } else {
            emitByte(vm, OP_POP);
        }
        current->localCount--;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(GhostVM *vm, Precedence precedence);

static uint8_t identifierConstant(GhostVM *vm, Token* name) {
    return makeConstant(vm, OBJ_VAL(copyString(vm, name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;

    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];

        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Cannot read local variable it ints own initializer.");
            }

            return i;
        }
    }

    return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal) {
    int upvalueCount = compiler->function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT8_COUNT) {
        error("Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolveLocal(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

static void declareVariable() {
    // Global variables are implicitly declared
    if (current->scopeDepth == 0) return;

    Token* name = &parser.previous;

    for (int i = current->localCount - 1; i >= 0; i--) {
        Local* local = &current->locals[i];

        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Variable with this name already declared in this scope.");
        }
    }

    addLocal(*name);
}

static uint8_t parseVariable(GhostVM *vm, const char *errorMessage)
{
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(vm, &parser.previous);
}

static void markInitialized() {
    if (current->scopeDepth == 0) return;

    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(GhostVM *vm, uint8_t global)
{
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(vm, OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList(GhostVM *vm) {
    uint8_t argCount = 0;

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression(vm);

            if (argCount == 255) {
                error("Cannot have more than 255 arguments.");
            }

            argCount++;
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

static void and_(GhostVM *vm, bool canAssign) {
    int endJump = emitJump(vm, OP_JUMP_IF_FALSE);

    emitByte(vm, OP_POP);
    parsePrecedence(vm, PREC_AND);

    patchJump(endJump);
}

static void binary(GhostVM *vm, bool canAssign) {
    // Remember the operator
    TokenType operatorType = parser.previous.type;

    // Compile the right operand
    ParseRule* rule = getRule(operatorType);
    parsePrecedence(vm, (Precedence)(rule->precedence + 1));

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(vm, OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(vm, OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(vm, OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(vm, OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(vm, OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(vm, OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(vm, OP_ADD); break;
        case TOKEN_MINUS:         emitByte(vm, OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(vm, OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(vm, OP_DIVIDE); break;
        case TOKEN_PERCENT:       emitByte(vm, OP_MODULO); break;
        default:
            // Unreachable
            return;
    }
}

static void call(GhostVM *vm, bool canAssign) {
    uint8_t argCount = argumentList(vm);
    emitBytes(vm, OP_CALL, argCount);
}

static void list(GhostVM *vm, bool canAssign) {
    emitByte(vm, OP_NEW_LIST);

    do {
        if (check(TOKEN_RIGHT_BRACKET)) {
            break;
        }

        expression(vm);
        emitByte(vm, OP_ADD_LIST);
    } while (match(TOKEN_COMMA));

    consume(TOKEN_RIGHT_BRACKET, "Expected closing ']'");
}

static void subscript(GhostVM *vm, bool canAssign) {
    expression(vm);
    consume(TOKEN_RIGHT_BRACKET, "Expected closing ']'");

    if (match(TOKEN_EQUAL)) {
        expression(vm);
        emitByte(vm, OP_SUBSCRIPT_ASSIGN);
    } else {
        emitByte(vm, OP_SUBSCRIPT);
    }
}

static void dot(GhostVM *vm, bool canAssign) {
    consume(TOKEN_IDENTIFIER, "Expect property name after '.'.");
    uint8_t name = identifierConstant(vm, &parser.previous);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression(vm);
        emitBytes(vm, OP_SET_PROPERTY, name);
    } else if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList(vm);
        emitBytes(vm, OP_INVOKE, name);
        emitByte(vm, argCount);
    } else {
        emitBytes(vm, OP_GET_PROPERTY, name);
    }
}

static void literal(GhostVM *vm, bool canAssign) {
    switch (parser.previous.type) {
        case TOKEN_FALSE: emitByte(vm, OP_FALSE); break;
        case TOKEN_NULL: emitByte(vm, OP_NULL); break;
        case TOKEN_TRUE: emitByte(vm, OP_TRUE); break;
        default:
            // Unreachable
            return;
    }
}

static void grouping(GhostVM *vm, bool canAssign) {
    expression(vm);

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(GhostVM *vm, bool canAssign) {
    double value = strtod(parser.previous.start, NULL);

    emitConstant(vm, NUMBER_VAL(value));
}

static void or_(GhostVM *vm, bool canAssign) {
    int elseJump = emitJump(vm, OP_JUMP_IF_FALSE);
    int endJump = emitJump(vm, OP_JUMP);

    patchJump(elseJump);
    emitByte(vm, OP_POP);

    parsePrecedence(vm, PREC_OR);
    patchJump(endJump);
}

static void string(GhostVM *vm, bool canAssign) {
    // We could support string escape sequences like
    // \n here.
    emitConstant(vm, OBJ_VAL(copyString(
        vm,
        parser.previous.start + 1,
        parser.previous.length - 2
    )));
}

static void namedVariable(GhostVM *vm, Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);

    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(vm, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression(vm);
        emitBytes(vm, setOp, (uint8_t)arg);
    } else {
        emitBytes(vm, getOp, (uint8_t)arg);
    }
}

static void variable(GhostVM *vm, bool canAssign) {
    namedVariable(vm, parser.previous, canAssign);
}

static Token syntheticToken(const char* text) {
    Token token;
    token.start = text;
    token.length = (int)strlen(text);

    return token;
}

static void super_(GhostVM *vm, bool canAssign) {
    if (currentClass == NULL) {
        error("Cannot use 'super' outside of a class.");
    } else if (!currentClass->hasSuperclass) {
        error("Cannot use 'super' in a class with no superclass.");
    }

    consume(TOKEN_DOT, "Expect '.' after 'super'.");
    consume(TOKEN_IDENTIFIER, "Expect superclass method name.");
    uint8_t name = identifierConstant(vm, &parser.previous);

    namedVariable(vm, syntheticToken("this"), false);

    if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList(vm);
        namedVariable(vm, syntheticToken("super"), false);
        emitBytes(vm, OP_SUPER_INVOKE, name);
        emitByte(vm, argCount);
    } else {
        namedVariable(vm, syntheticToken("super"), false);
        emitBytes(vm, OP_GET_SUPER, name);
    }
}

static void this_(GhostVM *vm, bool canAssign) {
    if (currentClass == NULL) {
        error("Cannot use 'this' outside of a class.");
        return;
    }

    variable(vm, false);
}

static void unary(GhostVM *vm, bool canAssign) {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(vm, PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType) {
        case TOKEN_BANG: emitByte(vm, OP_NOT); break;
        case TOKEN_MINUS: emitByte(vm, OP_NEGATE); break;
        default:
            // Unreachable
            return;
    }
}

ParseRule rules[] = {
    {grouping, call, PREC_CALL},     // TOKEN_LEFT_PAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_PAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_LEFT_BRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_BRACE
    {list, subscript, PREC_CALL},    // TOKEN_LEFT_BRACKET
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_BRACKET
    {NULL, NULL, PREC_NONE},         // TOKEN_COMMA
    {NULL, dot, PREC_CALL},          // TOKEN_DOT
    {unary, binary, PREC_TERM},      // TOKEN_MINUS
    {NULL, binary, PREC_TERM},       // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},         // TOKEN_SEMICOLON
    {NULL, binary, PREC_FACTOR},     // TOKEN_SLASH
    {NULL, binary, PREC_FACTOR},     // TOKEN_STAR
    {NULL, binary, PREC_TERM},       // TOKEN_PERCENT
    {unary, NULL, PREC_NONE},        // TOKEN_BANG
    {NULL, binary, PREC_EQUALITY},   // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_EQUAL
    {NULL, binary, PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
    {variable, NULL, PREC_NONE},     // TOKEN_IDENTIFIER
    {string, NULL, PREC_NONE},       // TOKEN_STRING
    {number, NULL, PREC_NONE},       // TOKEN_NUMBER
    {NULL, and_, PREC_AND},          // TOKEN_AND
    {NULL, NULL, PREC_NONE},         // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},         // TOKEN_ELSE
    {literal, NULL, PREC_NONE},      // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},         // TOKEN_FOR
    {NULL, NULL, PREC_NONE},         // TOKEN_FUNCTION
    {NULL, NULL, PREC_NONE},         // TOKEN_IF
    {literal, NULL, PREC_NONE},      // TOKEN_NULL
    {NULL, or_, PREC_OR},            // TOKEN_OR
    {NULL, NULL, PREC_NONE},         // TOKEN_RETURN
    {super_, NULL, PREC_NONE},       // TOKEN_SUPER
    {this_, NULL, PREC_NONE},        // TOKEN_THIS
    {literal, NULL, PREC_NONE},      // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},         // TOKEN_LET
    {NULL, NULL, PREC_NONE},         // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},         // TOKEN_EXTENDS
    {NULL, NULL, PREC_NONE},         // TOKEN_INCLUDE
    {NULL, NULL, PREC_NONE},         // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},         // TOKEN_EOF
};

static void parsePrecedence(GhostVM *vm, Precedence precedence) {
    advance();

    ParseFn prefixRule = getRule(parser.previous.type)->prefix;

    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(vm, canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();

        ParseFn infixRule = getRule(parser.previous.type)->infix;

        infixRule(vm, canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression(GhostVM *vm) {
    parsePrecedence(vm, PREC_ASSIGNMENT);
}

static void block(GhostVM *vm) {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration(vm);
    }

    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

static void function(GhostVM *vm, FunctionType type) {
    Compiler compiler;
    initCompiler(vm, &compiler, type);
    beginScope();

    // Compile the parameter list
    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");

    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;

            if (current->function->arity > 255) {
                errorAtCurrent("Cannot have more than 255 parameters.");
            }

            uint8_t paramConstant = parseVariable(vm, "Expect parameter name.");
            defineVariable(vm, paramConstant);
        } while (match(TOKEN_COMMA));
    }

    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");

    // The body
    consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    block(vm);

    // Create the function object
    ObjFunction *function = endCompiler(vm);
    emitBytes(vm, OP_CLOSURE, makeConstant(vm, OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(vm, compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(vm, compiler.upvalues[i].index);
    }
}

static void method(GhostVM *vm) {
    consume(TOKEN_IDENTIFIER, "Expect method name.");
    uint8_t constant = identifierConstant(vm, &parser.previous);

    FunctionType type = TYPE_METHOD;

    if (parser.previous.length == 11 && memcmp(parser.previous.start, "constructor", 11) == 0) {
        type = TYPE_CONSTRUCTOR;
    }

    function(vm, type);

    emitBytes(vm, OP_METHOD, constant);
}

static void classDeclaration(GhostVM *vm) {
    consume(TOKEN_IDENTIFIER, "Expect class name.");
    Token className = parser.previous;
    uint8_t nameConstant = identifierConstant(vm, &parser.previous);
    declareVariable();

    emitBytes(vm, OP_CLASS, nameConstant);
    defineVariable(vm, nameConstant);

    ClassCompiler classCompiler;
    classCompiler.name = parser.previous;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    if (match(TOKEN_EXTENDS)) {
        consume(TOKEN_IDENTIFIER, "Expect superclass name.");
        variable(vm, false);

        if (identifiersEqual(&className, &parser.previous)) {
            error("A class cannot inherit from itself.");
        }

        beginScope();
        addLocal(syntheticToken("super"));
        defineVariable(vm, 0);

        namedVariable(vm, className, false);
        emitByte(vm, OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }

    namedVariable(vm, className, false);
    consume(TOKEN_LEFT_BRACE, "Except '{' before class body.");

    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        method(vm);
    }

    consume(TOKEN_RIGHT_BRACE, "Except '}' after class body.");
    emitByte(vm, OP_POP);

    if (classCompiler.hasSuperclass) {
        endScope(vm);
    }

    currentClass = currentClass->enclosing;
}

static void functionDeclaration(GhostVM *vm) {
    uint8_t global = parseVariable(vm, "Expect function name.");
    markInitialized();
    function(vm, TYPE_FUNCTION);
    defineVariable(vm, global);
}

static void letDeclaration(GhostVM *vm) {
    uint32_t global = parseVariable(vm, "Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression(vm);
    } else {
        emitByte(vm, OP_NULL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(vm, global);
}

static void expressionStatement(GhostVM *vm) {
    expression(vm);
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(vm, OP_POP);
}

static void forStatement(GhostVM *vm) {
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");

    if (match(TOKEN_SEMICOLON)) {
        // No initializer
    } else if (match(TOKEN_LET)) {
        letDeclaration(vm);
    } else {
        expressionStatement(vm);
    }

    int loopStart = currentChunk()->count;

    int exitJump = -1;

    if (!match(TOKEN_SEMICOLON)) {
        expression(vm);
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false
        exitJump = emitJump(vm, OP_JUMP_IF_FALSE);
        emitByte(vm, OP_POP);
    }

    if (!match(TOKEN_RIGHT_PAREN)) {
        int bodyJump = emitJump(vm, OP_JUMP);

        int incrementStart = currentChunk()->count;
        expression(vm);
        emitByte(vm, OP_POP);
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(vm, loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement(vm);

    emitLoop(vm, loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(vm, OP_POP);
    }

    endScope(vm);
}

static void ifStatement(GhostVM *vm) {
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(vm);
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(vm, OP_JUMP_IF_FALSE);
    emitByte(vm, OP_POP);
    statement(vm);

    int elseJump = emitJump(vm, OP_JUMP);

    patchJump(thenJump);
    emitByte(vm, OP_POP);

    if (match(TOKEN_ELSE)) statement(vm);
    patchJump(elseJump);
}

static void includeStatement(GhostVM *vm) {
    consume(TOKEN_STRING, "Expect a string after include");
    emitConstant(vm, OBJ_VAL(copyString(vm, parser.previous.start + 1, parser.previous.length - 2)));
    consume(TOKEN_SEMICOLON, "Expect ';' after include.");

    emitByte(vm, OP_INCLUDE);
}

static void returnStatement(GhostVM *vm) {
    if (current->type == TYPE_SCRIPT) {
        error("Cannot return from top-level code.");
    }

    if (match(TOKEN_SEMICOLON)) {
        emitReturn(vm);
    } else {
        if (current->type == TYPE_CONSTRUCTOR) {
            error("Cannot return a value from a constructor.");
        }

        expression(vm);
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(vm, OP_RETURN);
    }
}

static void whileStatement(GhostVM *vm) {
    int loopStart = currentChunk()->count;

    consume(TOKEN_LEFT_PAREN, "Except '(' after 'while'.");
    expression(vm);
    consume(TOKEN_RIGHT_PAREN, "Except ')' after condition.");

    int exitJump = emitJump(vm, OP_JUMP_IF_FALSE);

    emitByte(vm, OP_POP);
    statement(vm);

    emitLoop(vm, loopStart);

    patchJump(exitJump);
    emitByte(vm, OP_POP);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUNCTION:
            case TOKEN_LET:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_EXTENDS:
            case TOKEN_INCLUDE:
            case TOKEN_RETURN:
                return;

            default:
                // Do nothing
                ;
        }

        advance();
    }
}

static void declaration(GhostVM *vm) {
    if (match(TOKEN_CLASS)) {
        classDeclaration(vm);
    } else if (match(TOKEN_FUNCTION)) {
        functionDeclaration(vm);
    } else if (match(TOKEN_LET)) {
        letDeclaration(vm);
    } else {
        statement(vm);
    }

    if (parser.panicMode) synchronize();
}

static void statement(GhostVM *vm) {
    if (match(TOKEN_FOR)) {
        forStatement(vm);
    } else if (match(TOKEN_IF)) {
        ifStatement(vm);
    } else if (match(TOKEN_RETURN)) {
        returnStatement(vm);
    } else if (match(TOKEN_INCLUDE)) {
        includeStatement(vm);
    } else if (match(TOKEN_WHILE)) {
        whileStatement(vm);
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block(vm);
        endScope(vm);
    } else {
        expressionStatement(vm);
    }
}

ObjFunction* ghostCompile(GhostVM *vm, const char* source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(vm, &compiler, TYPE_SCRIPT);

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (! match(TOKEN_EOF)) {
        declaration(vm);
    }

    ObjFunction *function = endCompiler(vm);

    return parser.hadError ? NULL : function;
}

void markCompilerRoots(GhostVM *vm) {
    Compiler* compiler = current;

    while (compiler != NULL) {
        markObject(vm, (Obj *)compiler->function);
        compiler = compiler->enclosing;
    }
}