#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

static int line_row = 1, line_column = 1, prev_line_column;

int get_char()
{
    line_column++;

    int ch = getchar();
    if (ch == '\n') {
        prev_line_column = line_column;
        line_row++;
        line_column = 1;
    }

    return ch;
}

void unget_char(int ch)
{
    line_column--;
    if (ch == '\n') {
        line_row--;
        line_column = prev_line_column;
    }
    ungetc(ch, stdin);
}

typedef struct Token Token;
struct Token {
    int line_row, line_column;

    enum {
        T_IDENT,
        T_INTEGER,
        T_COMMA,
        T_LBRACKET,
        T_RBRACKET,
        T_PLUS,
        T_MINUS,
        T_COLON,
    } kind;

    union {
        char *sval;
        int ival;
    };
};

const char *token2str(Token *token)
{
    static char buf[128];
    switch (token->kind) {
    case T_IDENT:
        return token->sval;
    case T_INTEGER:
        // TODO: returned pointer is not malloc-ed.
        sprintf(buf, "%d", token->ival);
        return buf;
    case T_COMMA:
        return ",";
    case T_LBRACKET:
        return "[";
    case T_RBRACKET:
        return "]";
    case T_PLUS:
        return "+";
    case T_MINUS:
        return "-";
    case T_COLON:
        return ":";
    default:
        assert(0);
    }
}

const char *tokenkind2str(int kind)
{
    switch (kind) {
    case T_IDENT:
        return "identifier";
    case T_INTEGER:
        return "integer";
    case T_COMMA:
        return "comma";
    case T_LBRACKET:
        return "left bracket";
    case T_RBRACKET:
        return "right bracket";
    case T_PLUS:
        return "plus";
    case T_MINUS:
        return "minus";
    case T_COLON:
        return "colon";
    default:
        assert(0);
    }
}

_Noreturn void failwith_unexpected_token(int line_row, int line_column,
                                         const char *got, const char *expected)
{
    failwith(line_row, line_column,
             "Unexpected token: got \e[1m'%s'\e[m but expected \e[1m'%s'\e[m",
             got, expected);
}

Token *next_token()
{
    static Token token;
    static char sval[128];

    int ch;
    while ((ch = get_char()) != EOF) {
        if (isspace(ch)) continue;

        token.line_row = line_row;
        token.line_column = line_column;

        if (isalpha(ch) || ch == '.' || ch == '_') {  // read an identifier
            int i = 0;
            sval[i++] = ch;
            while (isalnum(ch = get_char())) sval[i++] = ch;
            unget_char(ch);
            sval[i++] = '\0';

            token.kind = T_IDENT;
            token.sval = sval;
            return &token;
        }

        if (isdigit(ch)) {  // read an integer
            token.kind = T_INTEGER;

            if (ch == '0') {
                ch = get_char();
                if (ch != 'x') {  // just 0
                    unget_char(ch);
                    token.ival = 0;
                    return &token;
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
                unget_char(ch);
                token.ival = ival;
                return &token;
            }

            // decimal number
            int ival = ch - '0';
            while (isdigit(ch = get_char())) ival = ival * 10 + ch - '0';
            unget_char(ch);
            token.ival = ival;
            return &token;
        }

        if (ch == '#' || ch == '/') {  // skip comment until endline
            while ((ch = get_char()) != '\n')
                ;
            continue;
        }

        switch (ch) {
        case ',':
            token.kind = T_COMMA;
            break;
        case '[':
            token.kind = T_LBRACKET;
            break;
        case ']':
            token.kind = T_RBRACKET;
            break;
        case '+':
            token.kind = T_PLUS;
            break;
        case '-':
            token.kind = T_MINUS;
            break;
        case ':':
            token.kind = T_COLON;
            break;
        default:
            failwith(line_row, line_column - 1,
                     "Unrecognized character: \e[1m'%c'\e[m", ch);
        }

        return &token;
    }

    return NULL;
}

Token *pending_token = NULL;

Token *pop_token()
{
    Token *token = pending_token;
    if (token == NULL) token = next_token();
    pending_token = NULL;

    return token;
}

Token *expect_token(int kind)
{
    Token *token = pop_token();
    if (token == NULL) failwith(line_row, line_column, "Unexpected EOF");
    if (token->kind != kind)
        failwith_unexpected_token(token->line_row, token->line_column,
                                  token2str(token), tokenkind2str(kind));

    return token;
}

Token *peek_token()
{
    if (pending_token != NULL) return pending_token;
    pending_token = next_token();
    return pending_token;
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

int expect_reg()
{
    Token *token = expect_token(T_IDENT);
    char *sval = token->sval;
    if (streql(sval, "SP")) return 7;  // R7

    if (!(sval[0] == 'R' && '0' <= sval[1] && sval[1] < '8'))
        failwith_unexpected_token(token->line_row, token->line_column,
                                  token2str(token), "R[0-7]");
    return sval[1] - '0';
}

int expect_integer()
{
    int mul = 1;
    if (pop_token_if(T_MINUS))  //
        mul = -1;
    return mul * expect_token(T_INTEGER)->ival;
}

int match_integer()
{
    return match_token(T_MINUS) || match_token(T_INTEGER);
}

void expect_mem(int *base_reg, int *disp)
{
    // [Rbase (+ disp)?]
    expect_token(T_LBRACKET);
    *base_reg = expect_reg();
    *disp = 0;
    if (pop_token_if(T_PLUS))
        *disp = expect_integer();
    else if (match_token(T_MINUS))
        *disp = expect_integer();
    expect_token(T_RBRACKET);
}

char *new_string(const char *src)
{
    char *buf = (char *)malloc(strlen(src) + 1);
    strcpy(buf, src);
    return buf;
}

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
    if (vec->size == vec->rsved_size) {
        vec->rsved_size = vec->rsved_size > 0 ? vec->rsved_size * 2 : 2;
        void **ndata = (void **)malloc(sizeof(void *) * vec->rsved_size);
        memcpy(ndata, vec->data, vec->size * sizeof(void *));
        vec->data = ndata;
    }

    vec->data[vec->size++] = item;
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

char *vformat(const char *src, va_list ap)
{
    char buf[512];  // TODO: enough length?
    vsprintf(buf, src, ap);

    char *ret = (char *)malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

char *format(const char *src, ...)
{
    va_list args;
    va_start(args, src);
    char *ret = vformat(src, args);
    va_end(args);
    return ret;
}

Vector *emits = NULL;

void emit(char *str, ...)
{
    va_list args;
    va_start(args, str);
    vector_push_back(emits, vformat(str, args));
    va_end(args);
}

int emitted_size()
{
    return vector_size(emits);
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
        if (strcmp(kv->key, key) == 0) return kv;
    }

    return NULL;
}

int main()
{
    Map *labels = new_map();
    Vector *pending = new_vector();

    emits = new_vector();

    while (peek_token() != NULL) {
        char *ident = expect_ident();

        if (streql(ident, "MOV")) {
            if (match_token(T_LBRACKET)) {
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                expect_token(T_COMMA);
                int src_reg = expect_reg();

                emit("ST R%d, %d(R%d)", src_reg, disp, base_reg);
                continue;
            }

            int dst_reg = expect_reg();
            expect_token(T_COMMA);

            if (match_integer()) {
                // MOV Rn, imm
                int src_imm = expect_integer();
                emit("LI R%d, %d", dst_reg, src_imm);
                continue;
            }

            if (match_token(T_LBRACKET)) {
                // MOV Rn, [Rbase (+ disp)?]
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                emit("LD R%d, %d(R%d)", dst_reg, disp, base_reg);
                continue;
            }

            // MOV Rn, Rm
            int src_reg = expect_reg();
            emit("MOV R%d, R%d", dst_reg, src_reg);
            continue;
        }

        if (streql(ident, "ADD")) {
            int dst_reg = expect_reg();
            expect_token(T_COMMA);

            if (match_integer()) {
                // ADD Rn, imm -> ADDI Rn, imm
                int src_imm = expect_integer();
                emit("ADDI R%d, %d", dst_reg, src_imm);
                continue;
            }

            // ADD Rn, Rm
            int src_reg = expect_reg();
            emit("ADD R%d, R%d", dst_reg, src_reg);
            continue;
        }

        if (streql(ident, "CMP")) {
            int dst_reg = expect_reg();
            expect_token(T_COMMA);

            if (match_integer()) {
                // CMP Rn, imm -> CMPI Rn, imm
                int src_imm = expect_integer();
                emit("CMPI R%d, %d", dst_reg, src_imm);
                continue;
            }

            // CMP Rn, Rm
            int src_reg = expect_reg();
            emit("CMP R%d, R%d", dst_reg, src_reg);
            continue;
        }

        {
            char *simple_ops_reg_reg[] = {"SUB", "AND", "OR", "XOR"};
            int i = 0, size = sizeof(simple_ops_reg_reg) / sizeof(char *);
            for (; i < size; i++)
                if (streql(ident, simple_ops_reg_reg[i])) break;
            if (i < size) {
                char *op = simple_ops_reg_reg[i];
                int dst_reg = expect_reg();
                expect_token(T_COMMA);
                int src_reg = expect_reg();
                emit("%s R%d, R%d", op, dst_reg, src_reg);
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
                int dst_reg = expect_reg();
                expect_token(T_COMMA);
                int src_imm = expect_integer();
                emit("%s R%d, %d", op, dst_reg, src_imm);
                continue;
            }
        }

        {
            char *jump_ops_src[] = {"JMP", "JE", "JNE", "JL", "JLE", "CALL"},
                 *jump_ops_dst[] = {"B", "BE", "BNE", "BLT", "BLE", "BAL"};
            int i = 0, size = sizeof(jump_ops_src) / sizeof(char *);
            for (; i < size; i++)
                if (streql(ident, jump_ops_src[i])) break;
            if (i < size) {
                if (match_integer()) {
                    int d = expect_integer();
                    emit("%s %d", jump_ops_dst[i], d);
                    continue;
                }

                char *label_name = new_string(expect_ident());
                vector_push_back(pending,
                                 new_pair(label_name, (void *)emitted_size()));
                emit("%s", jump_ops_dst[i]);
                continue;
            }
        }

        if (streql(ident, "IN")) {
            int src_reg = expect_reg();
            emit("IN R%d", src_reg);
            continue;
        }

        if (streql(ident, "OUT")) {
            int src_reg = expect_reg();
            emit("OUT R%d", src_reg);
            continue;
        }

        if (streql(ident, "RET")) {
            emit("BR");
            continue;
        }

        if (streql(ident, "HLT")) {
            emit("HLT");
            continue;
        }

        // label
        char *label_name = new_string(ident);
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
            failwith(-1, -1, "Undeclared label: \e[1m%s\e[m", label_name);
        int d = (int)(kv->value) - emit_index - 1;
        vector_set(emits, emit_index,
                   format("%s %d", (char *)vector_get(emits, emit_index), d));
    }

    int nemits = emitted_size();
    for (int i = 0; i < nemits; i++) {
        char *str = (char *)vector_get(emits, i);
        puts(str);
    }
}
