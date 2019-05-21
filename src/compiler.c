#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utility.h"

typedef struct Token_tag Token;
_Noreturn void failwith(Token *cause, const char *msg, ...);

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

typedef struct CodeLocation CodeLocation;
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
        T_LTEQ,
        K_IF,
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
        return "<";
    case T_MINUS:
        return "-";
    case T_COLON:
        return ":";
    case T_EQ:
        return "=";
    case T_EQEQ:
        return "==";
    case K_IF:
        return "if";
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

    case T_COMMA:
    case T_LBRACKET:
    case T_RBRACKET:
    case T_LBRACE:
    case T_RBRACE:
    case T_LPAREN:
    case T_RPAREN:
    case T_PLUS:
    case T_MINUS:
    case T_COLON:
    case T_EQ:
    case T_EQEQ:
    case T_NEQ:
    case T_LT:
    case T_LTEQ:
    case K_IF: {
        Token token = {.kind = kind};
        return token2str(&token);
    }
    }
    assert(0);
}

_Noreturn void failwith(Token *cause, const char *msg, ...)
{
    char buf[512];
    va_list args;
    va_start(args, msg);
    vsnprintf(buf, 512, msg, args);
    va_end(args);

    if (cause)
        error_at(cause->loc.line_row + 1, cause->loc.line_column + 1, buf);
    else
        error_at(line_row + 1, line_column + 1, buf);
}

#define HL_IDENT "\e[1m'%s'\e[m"
#define HL_REG "\e[1m'R%d'\e[m"

_Noreturn void failwith_unexpected_token(Token *token, const char *got,
                                         const char *expected)
{
    failwith(token,
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
            if (ch == '=') {
                token->kind = T_LTEQ;
                break;
            }
            unget_char();
            token->kind = T_LT;
        } break;

        case ':':
            token->kind = T_COLON;
            break;

        case '!':
            ch = get_char();
            if (ch == '=') {
                token->kind = T_NEQ;
                break;
            }
            unget_char();
            goto unrecognized_character;

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
    } kind;

    union {
        int ival;
    };
};

AST *new_ast(AST_KIND kind, CodeLocation loc)
{
    AST *ast = (AST *)malloc(sizeof(AST));
    ast->kind = kind;
    ast->loc = loc;
    return ast;
}

AST *parse()
{
    Token *token = expect_token(T_INTEGER);
    AST *ast = new_ast(AST_INTEGER, token->loc);
    ast->ival = token->ival;
    return ast;
}

void generate_code(FILE *fh, AST *ast)
{
    fprintf(fh, "LI R0, %d\n", ast->ival);
    fprintf(fh, "HLT\n");
}

int main()
{
    read_all_tokens(stdin);
    AST *ast = parse();
    generate_code(stdout, ast);

    free(ast);

    return 0;
}
