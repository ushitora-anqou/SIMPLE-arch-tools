#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

typedef struct Token Token;
struct Token {
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

Token *next_token()
{
    static Token token;
    static char sval[128];

    int ch;
    while ((ch = getchar()) != EOF) {
        if (isspace(ch)) continue;

        if (isalpha(ch) || ch == '.' || ch == '_') {  // read an identifier
            int i = 0;
            sval[i++] = ch;
            while (isalnum(ch = getchar())) sval[i++] = ch;
            ungetc(ch, stdin);
            sval[i++] = '\0';

            token.kind = T_IDENT;
            token.sval = sval;
            return &token;
        }

        if (isdigit(ch)) {  // read an integer
            int ival = ch - '0';
            while (isdigit(ch = getchar())) ival = ival * 10 + ch - '0';
            ungetc(ch, stdin);

            token.kind = T_INTEGER;
            token.ival = ival;
            return &token;
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
            assert(0);
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
    assert(token != NULL && token->kind == kind);

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
    char *sval = expect_ident();
    if (streql(sval, "SP")) return 7;  // R7

    assert(sval[0] == 'R' && '0' <= sval[1] && sval[1] < '8');
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
    if (pop_token_if(T_PLUS)) *disp = expect_integer();
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

        {
            char *simple_ops_reg_reg[] = {"ADD", "SUB", "AND",
                                          "OR",  "XOR", "CMP"};
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
        assert(kv != NULL);
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
