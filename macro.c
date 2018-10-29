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
    return expect_token(T_INTEGER)->ival;
}

int streql(const char *lhs, const char *rhs)
{
    return strcmp(lhs, rhs) == 0;
}

int main()
{
    char *op = expect_ident();

    if (streql(op, "MOV")) {
        int lhs = expect_reg();
        expect_token(T_COMMA);
        int rhs = expect_reg();
        printf("MOV R%d, R%d\n", lhs, rhs);
    }
}
