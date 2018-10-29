#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

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

        if (isalpha(ch)) {  // read an identifier
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

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

int main()
{
    while (peek_token() != NULL) {
        char *ident = expect_ident();

        if (streql(ident, "MOV")) {
            if (match_token(T_LBRACKET)) {
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                expect_token(T_COMMA);
                int src_reg = expect_reg();

                printf("ST R%d, %d(R%d)\n", src_reg, disp, base_reg);
                continue;
            }

            int dst_reg = expect_reg();
            expect_token(T_COMMA);

            if (match_integer()) {
                // MOV Rn, imm
                int src_imm = expect_integer();
                printf("LI R%d, %d\n", dst_reg, src_imm);
                continue;
            }

            if (match_token(T_LBRACKET)) {
                // MOV Rn, [Rbase (+ disp)?]
                int base_reg, disp;
                expect_mem(&base_reg, &disp);
                printf("LD R%d, %d(R%d)\n", dst_reg, disp, base_reg);
                continue;
            }

            // MOV Rn, Rm
            int src_reg = expect_reg();
            printf("MOV R%d, R%d\n", dst_reg, src_reg);
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
                printf("%s R%d, R%d\n", op, dst_reg, src_reg);
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
                printf("%s R%d, %d\n", op, dst_reg, src_imm);
                continue;
            }
        }

        if (streql(ident, "HLT")) {
            printf("HLT\n");
            continue;
        }
    }
}
