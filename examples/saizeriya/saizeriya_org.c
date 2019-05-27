#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

typedef uint16_t Word;

Word mem[0x5000];

enum {
    SAIZERIYA_CALORIE_ADDR = 0x400,
    SAIZERIYA_PRICE_ADDR = 0x500,
    SAIZERIYA_SIZE = 115,
    SAIZERIYA_BUDGET = 1000,
    P_ADDR = 0x0800,
    Q_ADDR = 0x0C00,
    PATH_P_ADDR = 0x1000,
    PATH_Q_ADDR = 0x3000
};

int lte(Word lhs, Word rhs)
{
    return (int16_t)lhs <= (int16_t)rhs;
}

Word routine()
{
    Word src = P_ADDR, dst = Q_ADDR;
    Word path_src = PATH_P_ADDR, path_dst = PATH_Q_ADDR;

    mem[src + 0] = 0;
    for (int i = 1; i <= SAIZERIYA_BUDGET; i++) mem[src + i] = 0x8000;  // -INF
    for (int i = PATH_P_ADDR; i < PATH_P_ADDR + (0x0400 << 3); i += 8)
        mem[i] = 0;

    for (int i = 0; i < SAIZERIYA_SIZE; i++) {
        for (int j = 0; j <= SAIZERIYA_BUDGET; j++) {
            Word val = mem[src + j];
            Word price_addr = SAIZERIYA_PRICE_ADDR + i;
            Word price = mem[price_addr];
            Word addr = src + j - price;

            for (int k = 0; k < 8; k++)
                mem[path_dst + (j << 3) + k] = mem[path_src + (j << 3) + k];

            if (lte(src, addr)) {
                Word val1 = mem[addr] + mem[SAIZERIYA_CALORIE_ADDR + i];
                if (lte(val, val1)) {
                    val = val1;

                    Word addr = path_src + ((j - price) << 3);
                    for (int k = 0; k < 8; k++)
                        mem[path_dst + (j << 3) + k] = mem[addr + k];

                    Word size = mem[addr];
                    size++;
                    assert(size != 8);
                    mem[path_dst + (j << 3) + size] = i;
                    mem[path_dst + (j << 3)] = size;
                }
            }
            mem[dst + j] = val;
        }

        Word tmp = src;
        src = dst;
        dst = tmp;

        tmp = path_src;
        path_src = path_dst;
        path_dst = tmp;
    }

    Word best_price = 0, best_calorie = 0;
    for (int i = 0; i <= SAIZERIYA_BUDGET; i++) {
        Word calorie = mem[src + i];
        if (lte(best_calorie, calorie)) {
            best_calorie = calorie;
            best_price = i;
        }
    }
    Word best_path_addr = path_src + (best_price << 3);
    Word best_path_size = mem[best_path_addr];
    mem[P_ADDR] = best_price;
    mem[P_ADDR + 1] = best_calorie;
    for (int k = 0; k < 8; k++) {
        if (k >= best_path_size) break;
        mem[P_ADDR + (k + 2)] = mem[best_path_addr + k + 1];
    }

    return P_ADDR;
}

int main()
{
#include "saizeriya.inc"
    uint16_t src_index = routine();
    for (int i = 0; i < 0x4000; i++) {
        printf("%04X : %04" PRIX16 "\n", src_index + i, mem[src_index + i]);
    }
    /*
    routine();
    for (int i = 0; i < 0x5000; i++) {
        printf("%04X : %04X\n", i, mem[i]);
    }
    */
}
