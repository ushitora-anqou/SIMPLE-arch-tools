#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

typedef struct Token_tag Token;
typedef struct CodeLocation CodeLocation;
_Noreturn void failwith(CodeLocation *loc, const char *msg, ...);

typedef struct Vector Vector;
struct Vector {
    void **data;
    int size, rsved_size;
};

Vector *new_vector()
{
    Vector *ret = (Vector *)malloc(sizeof(Vector));
    ret->size = 0;
    ret->rsved_size = 0;
    ret->data = NULL;
    return ret;
}

void vector_push_back(Vector *vec, void *item)
{
    assert(vec != NULL);

    if (vec->size == vec->rsved_size) {
        vec->rsved_size = vec->rsved_size > 0 ? vec->rsved_size * 2 : 2;
        void **ndata = (void **)malloc(sizeof(void *) * vec->rsved_size);
        if (vec->data) memcpy(ndata, vec->data, vec->size * sizeof(void *));
        vec->data = ndata;
    }

    vec->data[vec->size++] = item;
}

void *vector_pop_back(Vector *vec)
{
    if (vec->size == 0) return NULL;
    return vec->data[--vec->size];
}

void *vector_get(Vector *vec, int i)
{
    if (i >= vec->size) return NULL;
    return vec->data[i];
}

int vector_size(Vector *vec)
{
    return vec->size;
}

void *vector_set(Vector *vec, int i, void *item)
{
    assert(vec != NULL && i < vector_size(vec));
    vec->data[i] = item;
    return item;
}

void vector_push_back_vector(Vector *vec, Vector *src)
{
    for (int i = 0; i < vector_size(src); i++)
        vector_push_back(vec, vector_get(src, i));
}

typedef struct Pair Pair;
struct Pair {
    void *first, *second;
};

Pair *new_pair(void *first, void *second)
{
    Pair *pair = (Pair *)malloc(sizeof(Pair));
    pair->first = first;
    pair->second = second;
    return pair;
}

typedef struct KeyValue KeyValue;
struct KeyValue {
    const char *key;
    void *value;
};

typedef struct Map Map;
struct Map {
    Vector *data;
};

Map *new_map()
{
    Map *map = malloc(sizeof(Map));
    map->data = new_vector();
    return map;
}

int map_size(Map *map)
{
    return vector_size(map->data);
}

KeyValue *map_insert(Map *map, const char *key, void *item)
{
    KeyValue *kv = malloc(sizeof(KeyValue));
    kv->key = key;
    kv->value = item;
    vector_push_back(map->data, kv);
    return kv;
}

KeyValue *map_lookup(Map *map, const char *key)
{
    for (int i = 0; i < vector_size(map->data); i++) {
        KeyValue *kv = (KeyValue *)vector_get(map->data, i);
        if (kv != NULL && strcmp(kv->key, key) == 0) return kv;
    }

    return NULL;
}

int map_erase(Map *map, const char *key)
{
    int erased_cnt = 0;

    for (int i = 0; i < vector_size(map->data); i++) {
        KeyValue *kv = (KeyValue *)vector_get(map->data, i);
        if (kv != NULL && strcmp(kv->key, key) == 0) {
            // found
            vector_set(map->data, i, NULL);
            erased_cnt++;
        }
    }

    return erased_cnt;
}

static int line_row = 0, line_column = 0;
static Vector *input_lines = NULL;
static char *read_line = NULL;

void read_all_lines(FILE *fh)
{
    assert(input_lines == NULL);

    input_lines = new_vector();

    char buf[512];
    while (fgets(buf, sizeof(buf), fh) != NULL)
        vector_push_back(input_lines, new_string(buf));

    read_line = (char *)vector_get(input_lines, 0);
}

int get_char(void)
{
    assert(input_lines != NULL);

    if (line_row >= vector_size(input_lines)) return EOF;
    assert(read_line == NULL || line_column <= strlen(read_line));

    int ch = read_line[line_column++];
    while (ch == '\\' && read_line[line_column] == '\n') {
        // continuous lines
        line_row++;
        read_line = (char *)vector_get(input_lines, line_row);
        if (read_line == NULL) failwith(NULL, "Unexpected EOF");
        ch = read_line[0];
        line_column = 1;
    }
    if (ch == '\n') {
        line_row++;
        line_column = 0;
        read_line = (char *)vector_get(input_lines, line_row);
    }
    else if (ch == '\0') {
        ch = EOF;
    }

    return ch;
}

char *get_prev_read_line()
{
    return (char *)vector_get(input_lines, line_row - 1);
}

int get_prev_max_line_column()
{
    return strlen(get_prev_read_line());
}

void unget_char(void)
{
    if (--line_column < 0) {
        assert(line_row >= 1 && line_column == -1);

        line_row--;
        read_line = (char *)vector_get(input_lines, line_row);
        line_column = strlen(read_line) - 1;
    }

    while (line_column == 0 && line_row >= 1) {
        char *prev_line = get_prev_read_line();
        int size = get_prev_max_line_column();
        if (size >= 2 && prev_line[size - 1] == '\n' &&
            prev_line[size - 2] == '\\') {
            // continuous lines
            line_row--;
            read_line = prev_line;
            line_column = size - 2;
        }
    }
}

struct CodeLocation {
    int line_row, line_column;
    char *read_line;
};

typedef struct Token_tag Token;
struct Token_tag {
    CodeLocation loc;

    enum {
        T_IDENT,
        T_INTEGER,
        T_COMMA,
        T_LBRACKET,
        T_RBRACKET,
        T_LBRACE,
        T_RBRACE,
        T_LPAREN,
        T_RPAREN,
        T_PLUS,
        T_MINUS,
        T_COLON,
        T_EQ,
        T_EQEQ,
        T_NEQ,
        T_LT,
        T_GT,
        T_LTEQ,
        T_GTEQ,
        T_LTLT,
        T_GTGT,
        T_SEMICOLON,
        T_EXCLAM,
        K_IF,
        K_RETURN,
        K_INT,
        K_WHILE,
        K_FOR,
    } kind;

    union {
        char *sval;
        int ival;
    };
};

const char *token2str(Token *token)
{
    switch (token->kind) {
    case T_IDENT:
        return token->sval;
    case T_INTEGER:
        return format("%d", token->ival);
    case T_COMMA:
        return ",";
    case T_LBRACKET:
        return "[";
    case T_RBRACKET:
        return "]";
    case T_LBRACE:
        return "{";
    case T_RBRACE:
        return "}";
    case T_LPAREN:
        return "(";
    case T_RPAREN:
        return ")";
    case T_PLUS:
        return "+";
    case T_NEQ:
        return "!=";
    case T_LT:
        return "<";
    case T_LTEQ:
        return "<=";
    case T_GT:
        return ">";
    case T_GTEQ:
        return ">=";
    case T_MINUS:
        return "-";
    case T_COLON:
        return ":";
    case T_EQ:
        return "=";
    case T_EQEQ:
        return "==";
    case T_LTLT:
        return "<<";
    case T_GTGT:
        return ">>";
    case T_SEMICOLON:
        return ";";
    case T_EXCLAM:
        return "!";
    case K_IF:
        return "if";
    case K_RETURN:
        return "return";
    case K_INT:
        return "int";
    case K_WHILE:
        return "while";
    case K_FOR:
        return "for";
    }
    assert(0);
}

const char *tokenkind2str(int kind)
{
    switch (kind) {
    case T_IDENT:
        return "identifier";
    case T_INTEGER:
        return "integer";

    default: {
        Token token = {.kind = kind};
        return token2str(&token);
    }
    }
    assert(0);
}

_Noreturn void failwith(CodeLocation *loc, const char *msg, ...)
{
    char buf[512];
    va_list args;
    va_start(args, msg);
    vsnprintf(buf, 512, msg, args);
    va_end(args);

    if (loc)
        error_at(loc->line_row + 1, loc->line_column + 1, buf);
    else
        error_at(line_row + 1, line_column + 1, buf);
}

#define HL_IDENT "\e[1m'%s'\e[m"
#define HL_REG "\e[1m'R%d'\e[m"

_Noreturn void failwith_unexpected_token(Token *token, const char *got,
                                         const char *expected)
{
    failwith(&token->loc,
             "Unexpected token: got \e[1m'%s'\e[m but expected \e[1m'%s'\e[m",
             got, expected);
}

Token *next_token()
{
    Token *token = (Token *)malloc(sizeof(Token));
    char sval[128];

    int ch;
    while ((ch = get_char()) != EOF) {
        // line_row and line_column represent the NEXT position to be read.
        token->loc.line_row = line_row;
        token->loc.line_column = line_column - 1;
        token->loc.read_line = read_line;

        if (isspace(ch)) continue;

        if (isalpha(ch) || ch == '.' || ch == '_') {  // read an identifier
            int i = 0;
            sval[i++] = ch;

            while ((ch = get_char()) != EOF) {
                if (!(isalnum(ch) || ch == '_')) break;
                sval[i++] = ch;
            }
            unget_char();
            sval[i++] = '\0';

            if (streql(sval, "if")) {
                token->kind = K_IF;
                return token;
            }
            if (streql(sval, "return")) {
                token->kind = K_RETURN;
                return token;
            }
            if (streql(sval, "int")) {
                token->kind = K_INT;
                return token;
            }
            if (streql(sval, "while")) {
                token->kind = K_WHILE;
                return token;
            }
            if (streql(sval, "for")) {
                token->kind = K_FOR;
                return token;
            }

            token->kind = T_IDENT;
            token->sval = new_string(sval);
            return token;
        }

        if (isdigit(ch)) {  // read an integer
            token->kind = T_INTEGER;

            if (ch == '0') {
                ch = get_char();
                if (ch != 'x') {  // just 0
                    unget_char();
                    token->ival = 0;
                    return token;
                }

                // hex number
                int ival = 0;
                while (1) {
                    ch = get_char();
                    if (isdigit(ch))
                        ival = ival * 16 + ch - '0';
                    else if ('A' <= ch && ch <= 'F')
                        ival = ival * 16 + ch - 'A' + 10;
                    else if ('a' <= ch && ch <= 'f')
                        ival = ival * 16 + ch - 'a' + 10;
                    else
                        break;
                }
                unget_char();
                token->ival = ival;
                return token;
            }

            // decimal number
            int ival = ch - '0';
            while (isdigit(ch = get_char())) ival = ival * 10 + ch - '0';
            unget_char();
            token->ival = ival;
            return token;
        }

        if (ch == '/') {  // skip comment until endline
            while ((ch = get_char()) != '\n')
                ;
            continue;
        }

        switch (ch) {
        case ',':
            token->kind = T_COMMA;
            break;

        case '[':
            token->kind = T_LBRACKET;
            break;

        case ']':
            token->kind = T_RBRACKET;
            break;

        case '{':
            token->kind = T_LBRACE;
            break;

        case '}':
            token->kind = T_RBRACE;
            break;

        case '(':
            token->kind = T_LPAREN;
            break;

        case ')':
            token->kind = T_RPAREN;
            break;

        case '+': {
            token->kind = T_PLUS;
        } break;

        case '-': {
            token->kind = T_MINUS;
        } break;

        case '<': {
            ch = get_char();
            if (ch == '<') {
                token->kind = T_LTLT;
                break;
            }
            if (ch == '=') {
                token->kind = T_LTEQ;
                break;
            }
            unget_char();
            token->kind = T_LT;
        } break;

        case '>': {
            ch = get_char();
            if (ch == '>') {
                token->kind = T_GTGT;
                break;
            }
            if (ch == '=') {
                token->kind = T_GTEQ;
                break;
            }
            unget_char();
            token->kind = T_GT;
        } break;

        case ':':
            token->kind = T_COLON;
            break;

        case ';':
            token->kind = T_SEMICOLON;
            break;

        case '!':
            ch = get_char();
            if (ch == '=') {
                token->kind = T_NEQ;
                break;
            }
            unget_char();
            token->kind = T_EXCLAM;
            break;

        case '=':
            ch = get_char();
            if (ch == '=') {
                token->kind = T_EQEQ;
                break;
            }
            unget_char();
            token->kind = T_EQ;
            break;

        default:
            goto unrecognized_character;
        }

        return token;
    }

    free(token);
    return NULL;

unrecognized_character:
    error_at(line_row + 1, line_column,
             format("Unrecognized character: \e[1m'%c'\e[m", ch));
}

static Vector *input_tokens;
static int input_tokens_npos;

#define SAVE_CURRENT_INPUT_TOKENS                   \
    Vector *tmp__saved_input_tokens = input_tokens; \
    int tmp__saved_input_tokens_npos = input_tokens_npos;
#define RESTORE_CURRENT_INPUT_TOKENS        \
    input_tokens = tmp__saved_input_tokens; \
    input_tokens_npos = tmp__saved_input_tokens_npos;

void read_all_tokens(FILE *fh)
{
    read_all_lines(fh);

    input_tokens = new_vector();

    Token *token;
    while ((token = next_token()) != NULL)
        vector_push_back(input_tokens, token);
}

Token *pop_token()
{
    Token *token = vector_get(input_tokens, input_tokens_npos++);
    if (token == NULL) failwith(NULL, "Unexpected EOF");
    return token;
}

Token *peek_token()
{
    return vector_get(input_tokens, input_tokens_npos);
}

Token *expect_token(int kind)
{
    Token *token = pop_token();
    if (token->kind != kind)
        failwith_unexpected_token(token, token2str(token), tokenkind2str(kind));

    return token;
}

Token *match_token(int kind)
{
    Token *token = peek_token();
    if (token != NULL && token->kind == kind) return token;
    return NULL;
}

int match_token2(int kind0, int kind1)
{
    Token *token0 = (Token *)vector_get(input_tokens, input_tokens_npos),
          *token1 = (Token *)vector_get(input_tokens, input_tokens_npos + 1);

    if (token0 == NULL || token1 == NULL) return 0;
    return token0->kind == kind0 && token1->kind == kind1;
}

Token *pop_token_if(int kind)
{
    if (match_token(kind)) return pop_token();
    return NULL;
}

typedef struct AST AST;
typedef enum AST_KIND AST_KIND;
struct AST {
    CodeLocation loc;

    enum AST_KIND {
        AST_INTEGER,
        AST_ADD,
        AST_SUB,
        AST_FIXLSHIFT,
        AST_FIXRSHIFT,
        AST_LT,
        AST_LTE,
        AST_GT,
        AST_GTE,
        AST_EQ,
        AST_NEQ,
        AST_EXPR_STMT,
        AST_RETURN,
        AST_IF,
        AST_LNOT,
        AST_COMPOUND,
        AST_VAR,
        AST_VAR_DECL,
        AST_ASSIGN,
        AST_WHILE,
        AST_FOR,
        AST_MEM_READ,
        AST_MEM_WRITE,
    } kind;

    union {
        int ival;
        char *sval;
        Vector *exprs, *stmts;
        AST *ast;

        struct {
            AST *lhs, *rhs;
        };

        struct {
            AST *if_cond, *if_body;
        };

        struct {
            char *varname;
            AST *rhs;
        } assign;

        struct {
            AST *while_cond, *while_body;
        };

        struct {
            AST *for_init, *for_cond, *for_iter, *for_body;
        };

        struct {
            AST *addr, *rhs;
        } mem_write;
    };
};

AST *new_ast(AST_KIND kind, CodeLocation loc)
{
    AST *ast = (AST *)malloc(sizeof(AST));
    ast->kind = kind;
    ast->loc = loc;
    return ast;
}

AST *new_ast_wo_loc(AST_KIND kind)
{
    AST *ast = (AST *)malloc(sizeof(AST));
    ast->kind = kind;
    return ast;
}

AST *new_unop_ast(AST_KIND kind, AST *lhs)
{
    assert(lhs != NULL);

    AST *ast = new_ast(kind, lhs->loc);
    ast->ast = lhs;
    return ast;
}

AST *new_binop_ast(AST_KIND kind, AST *lhs, AST *rhs)
{
    assert(lhs != NULL && rhs != NULL);

    AST *ast = new_ast(kind, lhs->loc);
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

AST *parse_expr(void);
AST *parse_stmt(void);
int match_declaration(void);
Vector *parse_var_decls(void);

AST *parse_unsigned_integer(void)
{
    Token *token = expect_token(T_INTEGER);
    AST *ast = new_ast(AST_INTEGER, token->loc);
    ast->ival = token->ival;
    return ast;
}

AST *parse_primary(void)
{
    if (pop_token_if(T_LPAREN)) {
        AST *ast = parse_expr();
        expect_token(T_RPAREN);
        return ast;
    }

    if (match_token2(T_IDENT, T_LBRACKET)) {
        pop_token();  // discard T_IDENT
        pop_token();  // discard T_LBRACKET
        AST *ast = parse_expr();
        expect_token(T_RBRACKET);
        return new_unop_ast(AST_MEM_READ, ast);
    }

    if (match_token(T_IDENT)) {
        Token *token = pop_token();
        AST *ast = new_ast(AST_VAR, token->loc);
        ast->sval = token->sval;
        return ast;
    }

    return parse_unsigned_integer();
}

AST *parse_unary(void)
{
    if (pop_token_if(T_EXCLAM)) {
        AST *ast = new_unop_ast(AST_LNOT, parse_primary());
        return ast;
    }

    return parse_primary();
}

AST *parse_additive(void)
{
    AST *lhs = parse_unary();
    while (match_token(T_PLUS) || match_token(T_MINUS)) {
        Token *token = pop_token();
        lhs = new_binop_ast(token->kind == T_PLUS ? AST_ADD : AST_SUB, lhs,
                            parse_unary());
    }
    return lhs;
}

AST *parse_shift(void)
{
    AST *lhs = parse_additive();
    while (match_token(T_LTLT) || match_token(T_GTGT)) {
        Token *token = pop_token();
        lhs =
            new_binop_ast(token->kind == T_LTLT ? AST_FIXLSHIFT : AST_FIXRSHIFT,
                          lhs, parse_unsigned_integer());
    }
    return lhs;
}

AST *parse_relational(void)
{
    AST *lhs = parse_shift();
    while (match_token(T_LT) || match_token(T_GT) || match_token(T_LTEQ) ||
           match_token(T_GTEQ)) {
        Token *token = pop_token();
        switch (token->kind) {
        case T_LT:
            lhs = new_binop_ast(AST_LT, lhs, parse_shift());
            break;
        case T_GT:
            lhs = new_binop_ast(AST_GT, lhs, parse_shift());
            break;
        case T_LTEQ:
            lhs = new_binop_ast(AST_LTE, lhs, parse_shift());
            break;
        case T_GTEQ:
            lhs = new_binop_ast(AST_GTE, lhs, parse_shift());
            break;
        default:
            assert(0);
        }
    }
    return lhs;
}

AST *parse_equality(void)
{
    AST *lhs = parse_relational();
    while (match_token(T_EQEQ) || match_token(T_NEQ)) {
        Token *token = pop_token();
        lhs = new_binop_ast(token->kind == T_EQEQ ? AST_EQ : AST_NEQ, lhs,
                            parse_relational());
    }
    return lhs;
}

AST *parse_assignment(void)
{
    if (match_token2(T_IDENT, T_EQ)) {
        Token *ident_token = pop_token();
        char *varname = ident_token->sval;
        pop_token();
        AST *rhs = parse_assignment();
        AST *ast = new_ast(AST_ASSIGN, ident_token->loc);
        ast->assign.varname = varname;
        ast->assign.rhs = rhs;
        return ast;
    }

    if (match_token2(T_IDENT, T_LBRACKET)) {
        SAVE_CURRENT_INPUT_TOKENS;

        Token *ident_token = pop_token();
        pop_token();  // discard T_LBRACKET
        AST *addr = parse_expr();
        expect_token(T_RBRACKET);
        if (pop_token_if(T_EQ)) {
            AST *rhs = parse_assignment();
            AST *ast = new_ast(AST_MEM_WRITE, ident_token->loc);
            ast->mem_write.addr = addr;
            ast->mem_write.rhs = rhs;
            return ast;
        }

        RESTORE_CURRENT_INPUT_TOKENS;
    }

    return parse_equality();
}

AST *parse_expr(void)
{
    return parse_assignment();
}

AST *parse_expr_stmt(void)
{
    AST *expr = parse_expr();
    expect_token(T_SEMICOLON);
    return new_unop_ast(AST_EXPR_STMT, expr);
}

AST *parse_jump_stmt(void)
{
    expect_token(K_RETURN);
    AST *expr = parse_expr();
    expect_token(T_SEMICOLON);

    AST *ast = new_unop_ast(AST_RETURN, expr);
    return ast;
}

AST *parse_selection_stmt(void)
{
    expect_token(K_IF);
    expect_token(T_LPAREN);
    AST *cond_expr = parse_expr();
    expect_token(T_RPAREN);
    AST *body = parse_stmt();
    AST *ast = new_ast_wo_loc(AST_IF);
    ast->if_cond = cond_expr;
    ast->if_body = body;
    return ast;
}

AST *parse_compound_stmt(void)
{
    Vector *stmts = new_vector();

    expect_token(T_LBRACE);
    while (!pop_token_if(T_RBRACE)) {
        if (match_declaration())
            vector_push_back_vector(stmts, parse_var_decls());
        else
            vector_push_back(stmts, parse_stmt());
    }

    AST *ast = new_ast_wo_loc(AST_COMPOUND);
    ast->stmts = stmts;
    return ast;
}

AST *parse_iteration_while_stmt(void)
{
    Token *while_token = expect_token(K_WHILE);
    expect_token(T_LPAREN);
    AST *cond = parse_expr();
    expect_token(T_RPAREN);
    AST *body = parse_stmt();

    AST *ast = new_ast(AST_WHILE, while_token->loc);
    ast->while_cond = cond;
    ast->while_body = body;
    return ast;
}

AST *parse_iteration_for_stmt(void)
{
    Token *for_token = expect_token(K_FOR);
    expect_token(T_LPAREN);
    AST *init = NULL;
    if (!pop_token_if(T_SEMICOLON)) {
        init = parse_expr();
        expect_token(T_SEMICOLON);
    }
    AST *cond = NULL;
    if (!pop_token_if(T_SEMICOLON)) {
        cond = parse_expr();
        expect_token(T_SEMICOLON);
    }
    AST *iter = NULL;
    if (!pop_token_if(T_RPAREN)) {
        iter = parse_expr();
        expect_token(T_RPAREN);
    }
    AST *body = parse_stmt();

    AST *ast = new_ast(AST_FOR, for_token->loc);
    ast->for_init = init;
    ast->for_cond = cond;
    ast->for_iter = iter;
    ast->for_body = body;
    return ast;
}

AST *parse_stmt(void)
{
    if (match_token(K_RETURN)) return parse_jump_stmt();
    if (match_token(K_IF)) return parse_selection_stmt();
    if (match_token(T_LBRACE)) return parse_compound_stmt();
    if (match_token(K_WHILE)) return parse_iteration_while_stmt();
    if (match_token(K_FOR)) return parse_iteration_for_stmt();
    return parse_expr_stmt();
}

int match_declaration(void)
{
    return !!match_token(K_INT);
}

Vector *parse_var_decls(void)
{
    Vector *decls = new_vector();
    Token *int_token = expect_token(K_INT);
    do {
        char *varname = expect_token(T_IDENT)->sval;
        AST *ast = new_ast(AST_VAR_DECL, int_token->loc);
        ast->sval = varname;
        vector_push_back(decls, ast);
    } while (pop_token_if(T_COMMA));
    expect_token(T_SEMICOLON);
    return decls;
}

Vector *parse(void)
{
    Vector *stmts = new_vector();
    while (peek_token()) {
        if (match_declaration())
            vector_push_back_vector(stmts, parse_var_decls());
        else
            vector_push_back(stmts, parse_stmt());
    }
    return stmts;
}

typedef struct AnalysisEnv AnalysisEnv;
struct AnalysisEnv {
    AnalysisEnv *parent;
    Map *alphatbl;
};

AnalysisEnv *new_analysis_env(AnalysisEnv *parent)
{
    AnalysisEnv *env = (AnalysisEnv *)malloc(sizeof(AnalysisEnv));
    env->parent = parent;
    env->alphatbl = new_map();
    return env;
}

char *ae_lookup_alphatbl(AnalysisEnv *env, char *key)
{
    assert(env != NULL);

    KeyValue *kv = map_lookup(env->alphatbl, key);
    if (kv != NULL) return kv->value;
    if (env->parent != NULL) return ae_lookup_alphatbl(env->parent, key);
    return NULL;
}

int count(void)
{
    static int index = 0;
    return index++;
}

AST *analyze_detail(AST *ast, AnalysisEnv *env)
{
    switch (ast->kind) {
    case AST_INTEGER:
        break;

    case AST_ADD:
    case AST_SUB:
    case AST_FIXLSHIFT:
    case AST_FIXRSHIFT:
    case AST_LT:
    case AST_LTE:
    case AST_EQ:
    case AST_NEQ:
        ast->lhs = analyze_detail(ast->lhs, env);
        ast->rhs = analyze_detail(ast->rhs, env);
        break;

    case AST_GT:
        ast = new_binop_ast(AST_LT, analyze_detail(ast->rhs, env),
                            analyze_detail(ast->lhs, env));
        break;

    case AST_GTE:
        ast = new_binop_ast(AST_LTE, analyze_detail(ast->rhs, env),
                            analyze_detail(ast->lhs, env));
        break;

    case AST_EXPR_STMT:
    case AST_RETURN:
    case AST_MEM_READ:
        ast->ast = analyze_detail(ast->ast, env);
        break;

    case AST_MEM_WRITE:
        ast->mem_write.addr = analyze_detail(ast->mem_write.addr, env);
        ast->mem_write.rhs = analyze_detail(ast->mem_write.rhs, env);
        break;

    case AST_LNOT:
        assert(ast->ast != NULL);
        switch (ast->ast->kind) {
        case AST_LT:
            ast->ast->kind = AST_GTE;
            ast = analyze_detail(ast->ast, env);
            break;
        case AST_LTE:
            ast->ast->kind = AST_GT;
            ast = analyze_detail(ast->ast, env);
            break;
        case AST_GT:
            ast->ast->kind = AST_LTE;
            ast = analyze_detail(ast->ast, env);
            break;
        case AST_GTE:
            ast->ast->kind = AST_LT;
            ast = analyze_detail(ast->ast, env);
            break;
        default:
            ast->ast = analyze_detail(ast->ast, env);
            break;
        }
        break;

    case AST_IF:
        ast->if_cond = analyze_detail(ast->if_cond, env);
        ast->if_body = analyze_detail(ast->if_body, env);
        break;

    case AST_COMPOUND: {
        AnalysisEnv *newenv = new_analysis_env(env);
        for (int i = 0; i < vector_size(ast->stmts); i++)
            vector_set(
                ast->stmts, i,
                analyze_detail((AST *)vector_get(ast->stmts, i), newenv));
    } break;

    case AST_VAR: {
        // alpha conversion
        char *new_varname = ae_lookup_alphatbl(env, ast->sval);
        if (new_varname == NULL)
            failwith(&ast->loc, "Not declared variable: " HL_IDENT, ast->sval);
        ast->sval = new_varname;
    } break;

    case AST_VAR_DECL: {
        // alpha conversion
        if (map_lookup(env->alphatbl, ast->sval) != NULL)
            failwith(&ast->loc, "Duplicate variable declaration: " HL_IDENT,
                     ast->sval);
        char *newname = format("%s.%d", ast->sval, count());
        map_insert(env->alphatbl, ast->sval, newname);
        ast->sval = newname;
    } break;

    case AST_ASSIGN: {
        // alpha conversion
        char *new_varname = ae_lookup_alphatbl(env, ast->assign.varname);
        if (new_varname == NULL)
            failwith(&ast->loc, "Not declared variable: " HL_IDENT,
                     ast->assign.varname);
        ast->assign.varname = new_varname;
        ast->assign.rhs = analyze_detail(ast->assign.rhs, env);
    } break;

    case AST_WHILE: {
        ast->while_cond = analyze_detail(ast->while_cond, env);
        ast->while_body = analyze_detail(ast->while_body, env);
    } break;

    case AST_FOR: {
        if (ast->for_init) ast->for_init = analyze_detail(ast->for_init, env);
        if (ast->for_cond) ast->for_cond = analyze_detail(ast->for_cond, env);
        if (ast->for_iter) ast->for_iter = analyze_detail(ast->for_iter, env);
        ast->for_body = analyze_detail(ast->for_body, env);
    } break;
    }

    return ast;
}

Vector *analyze(Vector *src)
{
    Vector *dst = new_vector();
    AnalysisEnv *env = new_analysis_env(NULL);

    for (int i = 0; i < vector_size(src); i++) {
        AST *ast = (AST *)vector_get(src, i);
        assert(ast != NULL);
        vector_push_back(dst, analyze_detail(ast, env));
    }

    free(env);

    return dst;
}

typedef struct GenEnv GenEnv;
struct GenEnv {
    FILE *fh;
    int var_disp_index, reg_used_flag[8];
    Map *var2disp;
};

GenEnv *new_gen_env(FILE *fh)
{
    GenEnv *env = (GenEnv *)malloc(sizeof(GenEnv));
    env->fh = fh;
    env->var_disp_index = 0;
    for (int i = 0; i < 8; i++) env->reg_used_flag[i] = 0;
    env->var2disp = new_map();

    return env;
}

static GenEnv *env;

int get_reg(void)
{
    assert(env);

    for (int i = 0; i < 8; i++) {
        if (env->reg_used_flag[i]) continue;
        env->reg_used_flag[i] = 1;
        return i;
    }

    assert(0);
}

void give_reg_back(int reg_index)
{
    assert(env->reg_used_flag[reg_index]);

    env->reg_used_flag[reg_index] = 0;
}

char *make_label(void)
{
    return format(".L%d", count());
}

void emit(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *buf = vformat(fmt, ap);
    fprintf(env->fh, buf[0] == '.' ? "%s\n" : "\t%s\n", buf);
    free(buf);
    va_end(ap);
}

int generate_code_detail(AST *ast);

char *generate_cond(AST *ast)
{
    assert(ast != NULL);

    switch (ast->kind) {
    case AST_LT: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        give_reg_back(lhs_reg_index);
        give_reg_back(rhs_reg_index);
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        return "BLT";
    }
    case AST_LTE: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        give_reg_back(lhs_reg_index);
        give_reg_back(rhs_reg_index);
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        return "BLE";
    }
    case AST_EQ: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        give_reg_back(lhs_reg_index);
        give_reg_back(rhs_reg_index);
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        return "BE";
    }
    case AST_NEQ: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        give_reg_back(lhs_reg_index);
        give_reg_back(rhs_reg_index);
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        return "BNE";
    }
    default: {
        int reg_index = generate_code_detail(ast);
        give_reg_back(reg_index);
        emit("CMP R%d, 0", reg_index);
        return "BNE";
    }
    }
}

int generate_code_detail(AST *ast)
{
    switch (ast->kind) {
    case AST_INTEGER: {
        int reg_index = get_reg();
        emit("LI R%d, %d", reg_index, ast->ival);
        return reg_index;
    }

    case AST_LNOT: {
        int reg_index = generate_code_detail(ast->ast);
        char *exit_label = make_label(), *tobetrue_label = make_label();
        emit("CMP R%d, 0", reg_index);
        emit("BE %s", tobetrue_label);
        emit("LI R%d, 0", reg_index);
        emit("B %s", exit_label);
        emit("%s:", tobetrue_label);
        emit("LI R%d, 1", reg_index);
        emit("%s:", exit_label);
        return reg_index;
    }

    case AST_ADD: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        emit("ADD R%d, R%d", lhs_reg_index, rhs_reg_index);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_SUB: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        emit("SUB R%d, R%d", lhs_reg_index, rhs_reg_index);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_FIXLSHIFT: {
        int lhs_reg_index = generate_code_detail(ast->lhs);
        emit("SLL R%d, %d", lhs_reg_index, ast->rhs->ival);
        return lhs_reg_index;
    }

    case AST_FIXRSHIFT: {
        int lhs_reg_index = generate_code_detail(ast->lhs);
        emit("SRL R%d, %d", lhs_reg_index, ast->rhs->ival);
        return lhs_reg_index;
    }

    case AST_LT: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        char *lt_label = make_label(), *exit_label = make_label();
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        emit("BLT %s", lt_label);
        emit("LI R%d, 0", lhs_reg_index);
        emit("B %s", exit_label);
        emit("%s:", lt_label);
        emit("LI R%d, 1", lhs_reg_index);
        emit("%s:", exit_label);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_LTE: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        char *lte_label = make_label(), *exit_label = make_label();
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        emit("BLE %s", lte_label);
        emit("LI R%d, 0", lhs_reg_index);
        emit("B %s", exit_label);
        emit("%s:", lte_label);
        emit("LI R%d, 1", lhs_reg_index);
        emit("%s:", exit_label);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_EQ: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        char *eq_label = make_label(), *exit_label = make_label();
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        emit("BE %s", eq_label);
        emit("LI R%d, 0", lhs_reg_index);
        emit("B %s", exit_label);
        emit("%s:", eq_label);
        emit("LI R%d, 1", lhs_reg_index);
        emit("%s:", exit_label);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_NEQ: {
        int lhs_reg_index = generate_code_detail(ast->lhs),
            rhs_reg_index = generate_code_detail(ast->rhs);
        char *neq_label = make_label(), *exit_label = make_label();
        emit("CMP R%d, R%d", lhs_reg_index, rhs_reg_index);
        emit("BNE %s", neq_label);
        emit("LI R%d, 0", lhs_reg_index);
        emit("B %s", exit_label);
        emit("%s:", neq_label);
        emit("LI R%d, 1", lhs_reg_index);
        emit("%s:", exit_label);
        give_reg_back(rhs_reg_index);
        return lhs_reg_index;
    }

    case AST_EXPR_STMT: {
        give_reg_back(generate_code_detail(ast->ast));
        return -1;
    }

    case AST_RETURN: {
        int reg_index = generate_code_detail(ast->ast);
        assert(reg_index != -1);
        emit("MOV R0, R%d", reg_index);
        emit("HLT");
        give_reg_back(reg_index);
        return -1;
    }

    case AST_IF: {
        char *next_label = make_label(), *exit_label = make_label();
        char *cond = generate_cond(ast->if_cond);
        emit("%s %s", cond, next_label);
        emit("B %s", exit_label);
        emit("%s:", next_label);
        assert(generate_code_detail(ast->if_body) == -1);
        emit("%s:", exit_label);
        return -1;
    }

    case AST_COMPOUND: {
        for (int i = 0; i < vector_size(ast->stmts); i++) {
            AST *stmt = (AST *)vector_get(ast->stmts, i);
            int reg_index = generate_code_detail(stmt);
            assert(reg_index == -1);
        }
        return -1;
    }

    case AST_VAR: {
        KeyValue *kv = map_lookup(env->var2disp, ast->sval);
        assert(kv != NULL);
        int disp = (int)kv->value;
        int reg_index = get_reg();
        emit("LD R%d, %d(SP)", reg_index, disp);
        return reg_index;
    }

    case AST_VAR_DECL: {
        assert(map_lookup(env->var2disp, ast->sval) == NULL);
        map_insert(env->var2disp, ast->sval, (void *)(env->var_disp_index++));
        return -1;
    }

    case AST_ASSIGN: {
        KeyValue *kv = map_lookup(env->var2disp, ast->assign.varname);
        assert(kv != NULL);
        int disp = (int)kv->value;
        int reg_index = generate_code_detail(ast->assign.rhs);
        emit("ST R%d, %d(SP)", reg_index, disp);
        return reg_index;
    }

    case AST_WHILE: {
        char *next_label = make_label(), *exit_label = make_label(),
             *loop_label = make_label();
        emit("%s:", loop_label);
        char *cond = generate_cond(ast->while_cond);
        emit("%s %s", cond, next_label);
        emit("B %s", exit_label);
        emit("%s:", next_label);
        assert(generate_code_detail(ast->while_body) == -1);
        emit("B %s", loop_label);
        emit("%s:", exit_label);

        return -1;
    }

    case AST_FOR: {
        int reg_index;
        char *exit_label = make_label(), *loop_label = make_label();

        if (ast->for_init) {
            reg_index = generate_code_detail(ast->for_init);
            assert(reg_index != -1);
            give_reg_back(reg_index);
        }

        emit("%s:", loop_label);
        if (ast->for_cond) {
            char *next_label = make_label();
            char *cond = generate_cond(ast->for_cond);
            emit("%s %s", cond, next_label);
            emit("B %s", exit_label);
            emit("%s:", next_label);
        }
        assert(generate_code_detail(ast->for_body) == -1);
        if (ast->for_iter) {
            reg_index = generate_code_detail(ast->for_iter);
            assert(reg_index != -1);
            give_reg_back(reg_index);
        }
        emit("B %s", loop_label);
        emit("%s:", exit_label);

        return -1;
    }

    case AST_MEM_READ: {
        int reg_index = generate_code_detail(ast->ast);
        assert(reg_index != -1);
        emit("LD R%d, (R%d)", reg_index, reg_index);
        return reg_index;
    }

    case AST_MEM_WRITE: {
        int addr_reg_index = generate_code_detail(ast->mem_write.addr),
            rhs_reg_index = generate_code_detail(ast->mem_write.rhs);
        assert(addr_reg_index != -1);
        assert(rhs_reg_index != -1);
        give_reg_back(addr_reg_index);
        emit("ST R%d, (R%d)", rhs_reg_index, addr_reg_index);
        return rhs_reg_index;
    }

    case AST_GT:
    case AST_GTE:
        assert(0);
    }
    assert(0);
}

void generate_code(FILE *fh, Vector *ast)
{
    assert(fh && ast);

    env = new_gen_env(fh);

    int sp_reg_index = get_reg();
    emit("define SP R%d", sp_reg_index);
    emit("LI SP, 0");

    for (int i = 0; i < vector_size(ast); i++) {
        AST *stmt = (AST *)vector_get(ast, i);
        int reg_index = generate_code_detail(stmt);
        assert(reg_index == -1);
    }

    give_reg_back(sp_reg_index);
}

int main()
{
    read_all_tokens(stdin);
    Vector *ast = parse();
    ast = analyze(ast);
    generate_code(stdout, ast);

    free(ast);

    return 0;
}
