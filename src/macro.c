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

typedef struct Token_tag Token;
struct Token_tag {
    int line_row, line_column;
    char *read_line;

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
        T_REGISTER,
        T_NEWLINE,
        T_EQ,
        T_EQEQ,
        T_NEQ,
        T_LT,
        T_LTEQ,
        T_PLUSEQ,
        T_MINUSEQ,
        T_LTLTEQ,
        T_GTGTEQ,
        T_ANDEQ,
        T_OREQ,
        K_DEFINE,
        K_UNDEF,
        K_IF,
        K_THEN,
        K_GOTO,
        K_BEGIN,
        K_END,
        K_INLINE,
        K_HALT,
        K_ALLOC,
        K_FREE,
        P_LABELNS_BEGIN,
        P_LABELNS_END,
    } kind;

    union {
        char *sval;
        int ival;
    };
};

Token *source_token_replaced = NULL;

void set_source_token_replaced(Token *src)
{
    source_token_replaced = src;
}

Token *new_token(int kind)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->kind = kind;
    if (source_token_replaced) {
        token->read_line = source_token_replaced->read_line;
        token->line_row = source_token_replaced->line_row;
        token->line_column = source_token_replaced->line_column;
    }
    else {
        token->read_line = NULL;
        token->line_row = token->line_column = 0;
    }
    return token;
}

Token *new_ident(char *sval)
{
    Token *token = new_token(T_IDENT);
    token->sval = sval;
    return token;
}

Token *dup_token(Token *token)
{
    Token *newtoken = (Token *)malloc(sizeof(Token));
    memcpy(newtoken, token, sizeof(Token));
    return newtoken;
}

void copy_tokens_updating_source(Vector *dst, Vector *src)
{
    assert(source_token_replaced != NULL);

    for (int i = 0; i < vector_size(src); i++) {
        Token *token = dup_token((Token *)vector_get(src, i));

        token->read_line = source_token_replaced->read_line;
        token->line_row = source_token_replaced->line_row;
        token->line_column = source_token_replaced->line_column;

        vector_push_back(dst, token);
    }
}

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
    case T_PLUSEQ:
        return "+=";
    case T_NEQ:
        return "!=";
    case T_LT:
        return "<";
    case T_LTEQ:
        return "<=";
    case T_MINUSEQ:
        return "-=";
    case T_LTLTEQ:
        return "<<=";
    case T_GTGTEQ:
        return ">>=";
    case T_ANDEQ:
        return "&=";
    case T_OREQ:
        return "|=";
    case T_MINUS:
        return "-";
    case T_COLON:
        return ":";
    case T_REGISTER:
        return format("R%d", token->ival);
    case T_NEWLINE:
        return "newline";
    case T_EQ:
        return "=";
    case T_EQEQ:
        return "==";
    case K_DEFINE:
        return "define";
    case K_UNDEF:
        return "undef";
    case K_IF:
        return "if";
    case K_THEN:
        return "then";
    case K_GOTO:
        return "goto";
    case K_BEGIN:
        return "begin";
    case K_END:
        return "end";
    case K_INLINE:
        return "inline";
    case K_HALT:
        return "halt";
    case K_ALLOC:
        return "alloc";
    case K_FREE:
        return "free";
    case P_LABELNS_BEGIN:
        return format("P_LABELNS_BEGIN(%s)", token->sval);
    case P_LABELNS_END:
        return format("P_LABELNS_END(%s)", token->sval);
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
    case T_REGISTER:
        return "register";
    case P_LABELNS_BEGIN:
        return "P_LABELNS_BEGIN";
    case P_LABELNS_END:
        return "P_LABELNS_END";

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
    case T_NEWLINE:
    case T_EQ:
    case T_EQEQ:
    case T_PLUSEQ:
    case T_NEQ:
    case T_LT:
    case T_LTEQ:
    case T_MINUSEQ:
    case T_LTLTEQ:
    case T_GTGTEQ:
    case T_ANDEQ:
    case T_OREQ:
    case K_DEFINE:
    case K_UNDEF:
    case K_IF:
    case K_THEN:
    case K_GOTO:
    case K_BEGIN:
    case K_END:
    case K_INLINE:
    case K_HALT:
    case K_ALLOC:
    case K_FREE: {
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
        error_at(cause->line_row + 1, cause->line_column + 1, buf);
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
        token->line_row = line_row;
        token->line_column = line_column - 1;
        token->read_line = read_line;

        if (ch == '\n') {
            token->kind = T_NEWLINE;

            // '\n' is the last letter of the previous line.
            token->line_row--;
            token->line_column = get_prev_max_line_column();
            token->read_line = get_prev_read_line();

            return token;
        }

        if (isspace(ch)) continue;

        if (isalpha(ch) || ch == '.' || ch == '_') {  // read an identifier
            int i = 0;
            sval[i++] = ch;

            // when register
            if (ch == 'R') {
                if (isdigit(ch = get_char())) {
                    token->kind = T_REGISTER;
                    token->ival = ch - '0';
                    return token;
                }
                unget_char();
            }

            while ((ch = get_char()) != EOF) {
                if (!(isalnum(ch) || ch == '_')) break;
                sval[i++] = ch;
            }
            unget_char();
            sval[i++] = '\0';

            if (streql(sval, "define")) {
                token->kind = K_DEFINE;
                return token;
            }
            if (streql(sval, "undef")) {
                token->kind = K_UNDEF;
                return token;
            }
            if (streql(sval, "if")) {
                token->kind = K_IF;
                return token;
            }
            if (streql(sval, "then")) {
                token->kind = K_THEN;
                return token;
            }
            if (streql(sval, "goto")) {
                token->kind = K_GOTO;
                return token;
            }
            if (streql(sval, "begin")) {
                token->kind = K_BEGIN;
                return token;
            }
            if (streql(sval, "end")) {
                token->kind = K_END;
                return token;
            }
            if (streql(sval, "inline")) {
                token->kind = K_INLINE;
                return token;
            }
            if (streql(sval, "halt")) {
                token->kind = K_HALT;
                return token;
            }
            if (streql(sval, "alloc")) {
                token->kind = K_ALLOC;
                return token;
            }
            if (streql(sval, "free")) {
                token->kind = K_FREE;
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

        if (ch == '#' || ch == '/') {  // skip comment until endline
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
            ch = get_char();
            if (ch == '=') {
                token->kind = T_PLUSEQ;
                break;
            }

            unget_char();
            token->kind = T_PLUS;
        } break;

        case '-': {
            ch = get_char();
            if (ch == '=') {
                token->kind = T_MINUSEQ;
                break;
            }

            unget_char();
            token->kind = T_MINUS;
        } break;

        case '<': {
            ch = get_char();
            if (ch == '<') {
                ch = get_char();
                if (ch == '=') {
                    token->kind = T_LTLTEQ;
                    break;
                }
                goto unrecognized_character;
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
                ch = get_char();
                if (ch == '=') {
                    token->kind = T_GTGTEQ;
                    break;
                }
                goto unrecognized_character;
            }
            goto unrecognized_character;
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

        case '&':
            ch = get_char();
            if (ch == '=') {
                token->kind = T_ANDEQ;
                break;
            }
            goto unrecognized_character;

        case '|':
            ch = get_char();
            if (ch == '=') {
                token->kind = T_OREQ;
                break;
            }
            goto unrecognized_character;

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

char *expect_ident()
{
    Token *token = expect_token(T_IDENT);
    return token->sval;
}

int expect_integer(int sign, int nbits)
{
    assert(sign == 0 || sign == 1);
    assert(1 <= nbits && nbits <= 32);

    int mul = 1;
    if (pop_token_if(T_PLUS))
        mul = 1;
    else if (sign && pop_token_if(T_MINUS))
        mul = -1;
    Token *token = expect_token(T_INTEGER);
    int num = mul * token->ival;

    int max = (1u << (nbits - sign)) - 1, min = sign ? -(1u << (nbits - 1)) : 0;
    if (num < min || max < num)
        failwith_unexpected_token(token, format("%d", num),
                                  format("number in [%d, %d]", min, max));

    return num;
}

int match_integer()
{
    return match_token(T_PLUS) || match_token(T_MINUS) ||
           match_token(T_INTEGER);
}

void expect_mem(int *base_reg, int *disp)
{
    // [Rbase (+ disp)?]
    expect_token(T_LBRACKET);
    *base_reg = expect_token(T_REGISTER)->ival;
    *disp = 0;
    if (match_token(T_PLUS) || match_token(T_MINUS))
        *disp = expect_integer(1, 8);
    expect_token(T_RBRACKET);
}

Token *new_labelns_begin(char *ns_name)
{
    Token *token = new_token(P_LABELNS_BEGIN);
    token->sval = ns_name;
    return token;
}

Token *new_labelns_end(char *ns_name)
{
    Token *token = new_token(P_LABELNS_END);
    token->sval = ns_name;
    return token;
}

char *create_temporary_ns_name(void)
{
    // TODO: use more random way
    static char src_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char buf[2 + 20 + 1] = "::";
    for (int i = 0; i < sizeof(buf) - (2 + 1); i++)
        buf[i + 2] = src_chars[rand() % (sizeof(src_chars) - 1)];
    buf[sizeof(buf) - 1] = '\0';
    return new_string(buf);
}

typedef struct {
    Vector *params, *body;
} Inline;

static Map *inlines;

typedef struct {
    Map *name2reg;
    char *reg2name[8];
} AllocTable;

AllocTable *new_alloc_table()
{
    AllocTable *tbl = (AllocTable *)malloc(sizeof(AllocTable));
    tbl->name2reg = new_map();
    for (int i = 0; i < 8; i++) tbl->reg2name[i] = NULL;
    return tbl;
}

static AllocTable *global_alloc;

void preprocess_phase2(Vector *src, Vector *dst, Map *param2arg);
void preprocess_phase2_detail(Vector *dst, AllocTable *local_alloc,
                              Map *param2arg)
{
    // match inline definition
    if (pop_token_if(K_INLINE)) {
        char *inline_name = expect_ident();
        expect_token(T_LPAREN);
        Vector *params = new_vector();
        if (!pop_token_if(T_RPAREN)) {
            vector_push_back(params, expect_ident());
            while (pop_token_if(T_COMMA))
                vector_push_back(params, expect_ident());
            expect_token(T_RPAREN);
        }
        expect_token(T_LBRACE);
        Vector *body = new_vector();
        while (!pop_token_if(T_RBRACE)) vector_push_back(body, pop_token());

        Inline *inl = (Inline *)malloc(sizeof(Inline));
        inl->params = params;
        inl->body = body;
        map_insert(inlines, inline_name, inl);
        return;
    }

    // match register allocation
    if (match_token(K_ALLOC)) {
        Token *alloc_token = pop_token();
        char *ident = expect_ident();
        int reg_index = -1;
        if (match_token(T_REGISTER)) reg_index = expect_token(T_REGISTER)->ival;
        expect_token(T_NEWLINE);

        if (map_lookup(local_alloc->name2reg, ident) ||
            map_lookup(global_alloc->name2reg, ident))
            failwith(alloc_token, "Already allocated name " HL_IDENT, ident);

        if (reg_index == -1) {
            // automatic register allocation
            for (int i = 0; i < 8; i++) {
                if (local_alloc->reg2name[i] != NULL) continue;
                reg_index = i;
            }
            if (reg_index == -1)
                failwith(alloc_token, "No free register for " HL_IDENT, ident);
        }
        else {
            if (local_alloc->reg2name[reg_index] != NULL)
                failwith(alloc_token,
                         "You're allocating register " HL_REG " for " HL_IDENT
                         ", which has already been allocated for " HL_IDENT,
                         reg_index, ident, local_alloc->reg2name[reg_index]);
            if (global_alloc->reg2name[reg_index] != NULL)
                failwith(alloc_token,
                         "You're allocating register " HL_REG " for " HL_IDENT
                         ", which has already been allocated for " HL_IDENT,
                         reg_index, ident, global_alloc->reg2name[reg_index]);
        }

        map_insert(local_alloc->name2reg, ident, (void *)reg_index);
        local_alloc->reg2name[reg_index] = ident;
        return;
    }

    // match register free
    if (match_token(K_FREE)) {
        Token *free_token = pop_token();
        char *ident = expect_ident();
        expect_token(T_NEWLINE);

        KeyValue *kv = map_lookup(local_alloc->name2reg, ident);
        if (kv == NULL)
            failwith(free_token,
                     "No such register allocation to be freed in this "
                     "scope: " HL_IDENT,
                     ident);
        map_erase(local_alloc->name2reg, ident);
        local_alloc->reg2name[(int)kv->value] = NULL;
        return;
    }

    if (!match_token(T_IDENT)) {
        vector_push_back(dst, pop_token());
        return;
    }

    Token *ident_token = pop_token();  // expect T_IDENT
    char *ident = ident_token->sval;
    KeyValue *kv = NULL;

    // check if the identifier is a name to which a register has been
    // allocated to locally or globally
    kv = map_lookup(local_alloc->name2reg, ident);
    if (kv == NULL) kv = map_lookup(global_alloc->name2reg, ident);
    if (kv) {  // if found
        set_source_token_replaced(ident_token);
        Token *token = new_token(T_REGISTER);
        token->ival = (int)kv->value;
        vector_push_back(dst, token);
        set_source_token_replaced(NULL);
        return;
    }

    // match argument expansion
    if (param2arg) {
        kv = map_lookup(param2arg, ident);
        if (kv) {  // if found
            // expand arguments
            Vector *tokens = (Vector *)(kv->value);
            for (int i = 0; i < vector_size(tokens); i++)
                vector_push_back(dst, (Token *)vector_get(tokens, i));

            return;
        }
    }

    // match inline expansion
    kv = map_lookup(inlines, ident);
    if (kv) {  // if found
        // parse inline's arguments
        Vector *args = new_vector();
        expect_token(T_LPAREN);
        if (!pop_token_if(T_RPAREN)) {
            do {
                Vector *arg = new_vector();
                while (!match_token(T_RPAREN) && !match_token(T_COMMA))
                    preprocess_phase2_detail(arg, local_alloc, param2arg);
                vector_push_back(args, arg);
            } while (pop_token_if(T_COMMA));
            expect_token(T_RPAREN);
        }

        // check if #args is correct
        Inline *inl = (Inline *)(kv->value);
        if (vector_size(args) != vector_size(inl->params))
            failwith(ident_token, "Wrong number of arguments for inline %s",
                     ident);

        // create a map to correspond params with args
        Map *param2arg = new_map();
        for (int i = 0; i < vector_size(args); i++) {
            Vector *arg = (Vector *)vector_get(args, i);
            char *param = (char *)vector_get(inl->params, i);
            map_insert(param2arg, param, arg);
        }

        free(args);

        // expand inline
        char *tmp_ns_name = create_temporary_ns_name();
        vector_push_back(dst, new_labelns_begin(tmp_ns_name));
        preprocess_phase2(inl->body, dst, param2arg);
        vector_push_back(dst, new_labelns_end(tmp_ns_name));

        free(param2arg);

        return;
    }

    // normal identifier
    vector_push_back(dst, ident_token);
}

// copy tokens from src to dst, expanding inlines if any
void preprocess_phase2(Vector *src, Vector *dst, Map *param2arg)
{
    // save input_tokens
    Vector *org_input_tokens = input_tokens;
    int org_input_tokens_nnpos = input_tokens_npos;
    input_tokens = src;
    input_tokens_npos = 0;

    AllocTable *local_alloc = new_alloc_table();
    if (global_alloc == NULL) global_alloc = local_alloc;

    // start to copy tokens
    while (peek_token() != NULL)
        preprocess_phase2_detail(dst, local_alloc, param2arg);

    // restore input_tokens
    input_tokens = org_input_tokens;
    input_tokens_npos = org_input_tokens_nnpos;
}

void preprocess()
{
    Vector *dst = NULL;

    // Phase 1: define and expand macros
    dst = new_vector();
    Map *macros = new_map();
    while (peek_token() != NULL) {
        if (pop_token_if(K_DEFINE)) {
            Token *ident_token = expect_token(T_IDENT);
            char *name = ident_token->sval;
            Vector *code = new_vector();
            while (peek_token() != NULL && !match_token(T_NEWLINE))
                vector_push_back(code, pop_token());
            if (peek_token() != NULL) pop_token();  // pop T_NEWLINE

            if (map_lookup(macros, name) != NULL)
                failwith(ident_token,
                         "Can't define macro with the same name: %s", name);

            map_insert(macros, name, code);
            continue;
        }

        if (pop_token_if(K_UNDEF)) {
            char *name = expect_ident();
            expect_token(T_NEWLINE);
            map_erase(macros, name);
            continue;
        }

        // maybe macro expansion
        if (match_token(T_IDENT)) {
            Token *macro_ident = peek_token();

            KeyValue *kv = map_lookup(macros, macro_ident->sval);
            if (kv != NULL) {
                pop_token();  // discard the macro identifier
                Vector *code = (Vector *)(kv->value);

                set_source_token_replaced(macro_ident);
                copy_tokens_updating_source(dst, code);
                set_source_token_replaced(NULL);

                continue;
            }
        }

        vector_push_back(dst, pop_token());
    }
    free(macros);

    // Phase 2: define and expand inlines
    Vector *src = dst;
    dst = new_vector();
    inlines = new_map();
    preprocess_phase2(src, dst, NULL);
    free(inlines);

    // set new tokens
    input_tokens = dst;
    input_tokens_npos = 0;

    // Phase 3: expand syntax sugars
    dst = new_vector();
    while (peek_token() != NULL) {
        if (match_token(T_REGISTER) || match_token(T_LBRACKET)) {
            Vector *lhs = new_vector(), *rhs = new_vector();

            // set the source token to the left-most one
            // TODO: more precise position?
            set_source_token_replaced(peek_token());

            // left hand side
            while (!(match_token(T_EQ) || match_token(T_PLUSEQ) ||
                     match_token(T_MINUSEQ) || match_token(T_LTLTEQ) ||
                     match_token(T_GTGTEQ) || match_token(T_ANDEQ) ||
                     match_token(T_OREQ)))
                vector_push_back(lhs, pop_token());

            // operator
            Token *token = pop_token();
            int op_kind = token->kind;

            // right hand side
            if (match_token(T_LBRACKET)) {
                while (!match_token(T_RBRACKET))
                    vector_push_back(rhs, pop_token());
            }
            vector_push_back(rhs, pop_token());

            // generate code
            switch (op_kind) {
            case T_EQ:
                vector_push_back(dst, new_ident("MOV"));
                break;
            case T_PLUSEQ:
                vector_push_back(dst, new_ident("ADD"));
                break;
            case T_MINUSEQ:
                vector_push_back(dst, new_ident("SUB"));
                break;
            case T_LTLTEQ:
                vector_push_back(dst, new_ident("SLL"));
                break;
            case T_GTGTEQ:
                vector_push_back(dst, new_ident("SRL"));
                break;
            case T_ANDEQ:
                vector_push_back(dst, new_ident("AND"));
                break;
            case T_OREQ:
                vector_push_back(dst, new_ident("OR"));
                break;
            default:
                assert(0);
            }
            vector_push_back_vector(dst, lhs);
            vector_push_back(dst, new_token(T_COMMA));
            vector_push_back_vector(dst, rhs);

            set_source_token_replaced(NULL);

            continue;
        }

        if (pop_token_if(K_IF)) {
            Token *lhs = pop_token(), *op = pop_token(), *rhs = pop_token();
            expect_token(K_THEN);
            expect_token(K_GOTO);
            Token *label = expect_token(T_IDENT);  // label

            if (lhs->kind != T_REGISTER && lhs->kind != T_INTEGER)
                failwith_unexpected_token(lhs, token2str(lhs),
                                          "register or integer");
            if (rhs->kind != T_REGISTER && rhs->kind != T_INTEGER)
                failwith_unexpected_token(rhs, token2str(rhs),
                                          "register or integer");

            // set the source token to the left-most one
            // TODO: more precise position?
            set_source_token_replaced(lhs);

            vector_push_back(dst, new_ident("CMP"));
            vector_push_back(dst, lhs);
            vector_push_back(dst, new_token(T_COMMA));
            vector_push_back(dst, rhs);

            switch (op->kind) {
            case T_EQEQ:
                vector_push_back(dst, new_ident("JE"));
                break;
            case T_NEQ:
                vector_push_back(dst, new_ident("JNE"));
                break;
            case T_LT:
                vector_push_back(dst, new_ident("JL"));
                break;
            case T_LTEQ:
                vector_push_back(dst, new_ident("JLE"));
                break;
            default:
                failwith_unexpected_token(op, token2str(op), "==, !=, <, <=");
            }
            vector_push_back(dst, label);

            set_source_token_replaced(NULL);

            continue;
        }

        if (match_token(K_GOTO)) {
            set_source_token_replaced(pop_token());

            Token *label = expect_token(T_IDENT);  // label
            vector_push_back(dst, new_ident("JMP"));
            vector_push_back(dst, label);

            set_source_token_replaced(NULL);
            continue;
        }

        if (match_token(K_HALT)) {
            set_source_token_replaced(pop_token());
            vector_push_back(dst, new_ident("HLT"));
            set_source_token_replaced(NULL);
        }

        if (match_token(K_BEGIN)) {
            set_source_token_replaced(pop_token());

            expect_token(T_LPAREN);
            char *ns_name = expect_ident();
            expect_token(T_RPAREN);

            vector_push_back(dst, new_labelns_begin(ns_name));

            set_source_token_replaced(NULL);
            continue;
        }

        if (match_token(K_END)) {
            set_source_token_replaced(pop_token());

            expect_token(T_LPAREN);
            char *ns_name = expect_ident();
            expect_token(T_RPAREN);

            vector_push_back(dst, new_labelns_end(ns_name));

            set_source_token_replaced(NULL);
            continue;
        }

        while (peek_token() != NULL && !pop_token_if(T_NEWLINE))
            vector_push_back(dst, pop_token());
    }

    input_tokens = dst;
    input_tokens_npos = 0;
}

Vector *emits = NULL;

typedef struct {
    char *code, *comment;
} EmitedLine;

void emit(Token *src, char *str, ...)
{
    EmitedLine *line = (EmitedLine *)malloc(sizeof(EmitedLine));

    va_list args;
    va_start(args, str);
    line->code = vformat(str, args);
    va_end(args);

    line->comment = format("# %04d %s", src->line_row + 1, src->read_line);
    if (line->comment[strlen(line->comment) - 1] == '\n')
        line->comment[strlen(line->comment) - 1] = '\0';

    vector_push_back(emits, line);
}

int emitted_size()
{
    return vector_size(emits);
}

int main()
{
    srand((unsigned int)time(NULL));

    Map *labels = new_map();
    Vector *pending = new_vector(), *labelns_stack = new_vector();

    emits = new_vector();
    input_tokens = new_vector();

    read_all_tokens(stdin);
    preprocess();

    while (peek_token() != NULL) {
        if (match_token(P_LABELNS_BEGIN)) {
            Token *token = pop_token();
            vector_push_back(labelns_stack, token->sval);
            continue;
        }
        if (match_token(P_LABELNS_END)) {
            Token *token = pop_token();
            char *ns_name = vector_pop_back(labelns_stack);
            if (ns_name == NULL || !streql(ns_name, token->sval))
                failwith(token, "Invalid end of label namespace: %s",
                         token->sval);
            continue;
        }

        Token *op_token = expect_token(T_IDENT);
        char *ident = op_token->sval;

        if (streql(ident, "MOV")) {
            if (match_token(T_LBRACKET)) {
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                expect_token(T_COMMA);
                int src_reg = expect_token(T_REGISTER)->ival;

                emit(op_token, "ST R%d, %d(R%d)", src_reg, disp, base_reg);
                continue;
            }

            int dst_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);

            if (match_integer()) {
                // MOV Rn, imm
                int src_imm = expect_integer(1, 8);
                emit(op_token, "LI R%d, %d", dst_reg, src_imm);
                continue;
            }

            if (match_token(T_LBRACKET)) {
                // MOV Rn, [Rbase (+ disp)?]
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                emit(op_token, "LD R%d, %d(R%d)", dst_reg, disp, base_reg);
                continue;
            }

            // MOV Rn, Rm
            int src_reg = expect_token(T_REGISTER)->ival;
            emit(op_token, "MOV R%d, R%d", dst_reg, src_reg);
            continue;
        }

        if (streql(ident, "ADD")) {
            int dst_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);

            if (match_integer()) {
                // ADD Rn, imm -> ADDI Rn, imm
                int src_imm = expect_integer(1, 4);
                emit(op_token, "ADDI R%d, %d", dst_reg, src_imm);
                continue;
            }

            // ADD Rn, Rm
            int src_reg = expect_token(T_REGISTER)->ival;
            emit(op_token, "ADD R%d, R%d", dst_reg, src_reg);
            continue;
        }

        if (streql(ident, "CMP")) {
            int dst_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);

            if (match_integer()) {
                // CMP Rn, imm -> CMPI Rn, imm
                int src_imm = expect_integer(1, 4);
                emit(op_token, "CMPI R%d, %d", dst_reg, src_imm);
                continue;
            }

            // CMP Rn, Rm
            int src_reg = expect_token(T_REGISTER)->ival;
            emit(op_token, "CMP R%d, R%d", dst_reg, src_reg);
            continue;
        }

        {
            char *simple_ops_reg_reg[] = {"SUB", "AND", "OR", "XOR"};
            int i = 0, size = sizeof(simple_ops_reg_reg) / sizeof(char *);
            for (; i < size; i++)
                if (streql(ident, simple_ops_reg_reg[i])) break;
            if (i < size) {
                char *op = simple_ops_reg_reg[i];
                int dst_reg = expect_token(T_REGISTER)->ival;
                expect_token(T_COMMA);
                int src_reg = expect_token(T_REGISTER)->ival;
                emit(op_token, "%s R%d, R%d", op, dst_reg, src_reg);
                continue;
            }
        }

        {
            char *simple_ops_reg_imm[] = {"SLL", "SLR", "SRL", "SRA"};
            int i = 0, size = sizeof(simple_ops_reg_imm) / sizeof(char *);
            for (; i < size; i++)
                if (streql(ident, simple_ops_reg_imm[i])) break;
            if (i < size) {
                char *op = simple_ops_reg_imm[i];
                int dst_reg = expect_token(T_REGISTER)->ival;
                expect_token(T_COMMA);
                int src_imm = expect_integer(0, 4);
                emit(op_token, "%s R%d, %d", op, dst_reg, src_imm);
                continue;
            }
        }

        {
            char *jump_ops_src[] = {"JMP", "JE",  "JNE", "JL",  "JLE", "B",
                                    "BE",  "BNE", "BLT", "BLE", "CALL"},
                 *jump_ops_dst[] = {"B",  "BE",  "BNE", "BLT", "BLE", "B",
                                    "BE", "BNE", "BLT", "BLE", "BAL"};
            int i = 0, size = sizeof(jump_ops_src) / sizeof(char *);
            for (; i < size; i++)
                if (streql(ident, jump_ops_src[i])) break;
            if (i < size) {
                if (match_integer()) {
                    int d = expect_integer(1, 8);
                    emit(op_token, "%s %d", jump_ops_dst[i], d);
                    continue;
                }

                char *label_name = expect_ident();
                int labelns_stack_size = vector_size(labelns_stack);
                if (labelns_stack_size >= 1)
                    label_name =
                        format("%s::%s",
                               (char *)vector_get(labelns_stack,
                                                  labelns_stack_size - 1),
                               label_name);
                vector_push_back(pending,
                                 new_pair(label_name, (void *)emitted_size()));
                emit(op_token, "%s", jump_ops_dst[i]);
                continue;
            }
        }

        if (streql(ident, "LD")) {
            int dst_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);
            int disp = 0;
            if (match_integer()) disp = expect_integer(1, 8);
            expect_token(T_LPAREN);
            int src_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_RPAREN);

            emit(op_token, "LD R%d, %d(R%d)", dst_reg, disp, src_reg);
            continue;
        }

        if (streql(ident, "ST")) {
            int src_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);
            int disp = 0;
            if (match_integer()) disp = expect_integer(1, 8);
            expect_token(T_LPAREN);
            int dst_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_RPAREN);

            emit(op_token, "ST R%d, %d(R%d)", src_reg, disp, dst_reg);
            continue;
        }

        if (streql(ident, "LI")) {
            int src_reg = expect_token(T_REGISTER)->ival;
            expect_token(T_COMMA);
            int imm = expect_integer(1, 8);

            emit(op_token, "LI R%d, %d", src_reg, imm);
            continue;
        }

        if (streql(ident, "IN")) {
            int src_reg = expect_token(T_REGISTER)->ival;
            emit(op_token, "IN R%d", src_reg);
            continue;
        }

        if (streql(ident, "OUT")) {
            int src_reg = expect_token(T_REGISTER)->ival;
            emit(op_token, "OUT R%d", src_reg);
            continue;
        }

        if (streql(ident, "RET")) {
            emit(op_token, "BR");
            continue;
        }

        if (streql(ident, "HLT")) {
            emit(op_token, "HLT");
            continue;
        }

        // label
        char *label_name = ident;
        int labelns_stack_size = vector_size(labelns_stack);
        if (labelns_stack_size >= 1)
            label_name = format(
                "%s::%s",
                (char *)vector_get(labelns_stack, labelns_stack_size - 1),
                ident);
        expect_token(T_COLON);
        map_insert(labels, label_name, (void *)emitted_size());
    }

    int npend = vector_size(pending);
    for (int i = 0; i < npend; i++) {
        Pair *pair = (Pair *)vector_get(pending, i);
        char *label_name = pair->first;
        int emit_index = (int)(pair->second);

        KeyValue *kv = map_lookup(labels, label_name);
        if (kv == NULL)
            // TODO: position the label was used?
            failwith(NULL, "Undeclared label: \e[1m%s\e[m", label_name);
        int d = (int)(kv->value) - emit_index - 1;

        EmitedLine *line = vector_get(emits, emit_index);
        line->code = format("%s %d", line->code, d);
        vector_set(emits, emit_index, line);
    }

    int nemits = emitted_size();
    for (int i = 0; i < nemits; i++) {
        EmitedLine *line = (EmitedLine *)vector_get(emits, i);
        // pretty print
        printf("%s", line->code);
        int nspace = 20 - strlen(line->code);
        for (int i = 0; i < nspace; i++) printf(" ");
        printf("%s\n", line->comment);
    }
}
