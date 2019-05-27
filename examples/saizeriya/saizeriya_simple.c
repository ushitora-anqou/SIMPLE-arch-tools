int SAIZERIYA_CALORIE_ADDR, SAIZERIYA_PRICE_ADDR, SAIZERIYA_SIZE,
    SAIZERIYA_BUDGET;
SAIZERIYA_CALORIE_ADDR = (4 << 8);  // 0x400
SAIZERIYA_PRICE_ADDR = (5 << 8);    // 0x500
SAIZERIYA_SIZE = 115;
SAIZERIYA_BUDGET = (125 << 3);  // 1000

int P_ADDR, Q_ADDR, PATH_P_ADDR, PATH_Q_ADDR;
P_ADDR = (8 << 8);          // 0x0800
Q_ADDR = (12 << 8);         // 0x0C00
PATH_P_ADDR = (0x10 << 8);  // 0x1000
PATH_Q_ADDR = (0x30 << 8);  // 0x3000

int src, dst, path_src, path_dst;
src = P_ADDR;
dst = Q_ADDR;
path_src = PATH_P_ADDR;
path_dst = PATH_Q_ADDR;

int MINF;
MINF = (8 << 12);  // 0x8000 == -INF

{
    // initialize memory
    int i, size;
    mem[src] = 0;
    for (i = 1; i <= SAIZERIYA_BUDGET; i = i + 1) mem[src + i] = MINF;
    size = PATH_P_ADDR + (4 << 11);
    for (i = PATH_P_ADDR; i < size; i = i + 8) mem[i] = 0;
}

{
    int i;
    for (i = 0; i < SAIZERIYA_SIZE; i = i + 1) {
        int j;
        for (j = 0; j <= SAIZERIYA_BUDGET; j = j + 1) {
            int val, price_addr, price, addr, dst_path_addr_base;
            val = mem[src + j];
            price_addr = SAIZERIYA_PRICE_ADDR + i;
            price = mem[price_addr];
            addr = src + j - price;
            dst_path_addr_base = path_dst + (j << 3);

            {
                int k, src_path_addr_base;
                src_path_addr_base = path_src + (j << 3);
                for (k = 0; k < 8; k = k + 1)
                    mem[dst_path_addr_base + k] = mem[src_path_addr_base + k];
            }

            if (src <= addr) {
                int val1;
                val1 = mem[addr] + mem[SAIZERIYA_CALORIE_ADDR + i];
                if (val <= val1) {
                    val = val1;

                    int addr, k;
                    addr = path_src + ((j - price) << 3);
                    for (k = 0; k < 8; k = k + 1)
                        mem[dst_path_addr_base + k] = mem[addr + k];

                    int size;
                    size = mem[addr] + 1;
                    mem[dst_path_addr_base + size] = i;
                    mem[dst_path_addr_base] = size;
                }
            }

            mem[dst + j] = val;
        }

        int tmp;
        tmp = src;
        src = dst;
        dst = tmp;

        tmp = path_src;
        path_src = path_dst;
        path_dst = tmp;
    }
}

{
    int i, best_price, best_calorie;
    best_price = 0;
    best_calorie = 0;
    for (i = 0; i <= SAIZERIYA_BUDGET; i = i + 1) {
        int calorie;
        calorie = mem[src + i];
        if (best_calorie <= calorie) {
            best_calorie = calorie;
            best_price = i;
        }
    }

    int best_path_addr, best_path_size;
    best_path_addr = path_src + (best_price << 3);
    best_path_size = mem[best_path_addr];
    mem[P_ADDR] = best_price;
    mem[P_ADDR + 1] = best_calorie;
    int k;
    for (k = 0; k < 8; k = k + 1) {
        if (k < best_path_size) {
            mem[P_ADDR + k + 2] = mem[best_path_addr + k + 1];
        }
    }
}

{
    // set the result to registers
    //__builtin_output(mem[P_ADDR]);
    int res0, res1, res2, res3, res4;
    res0 = mem[P_ADDR + 0];
    res1 = mem[P_ADDR + 1];
    res2 = mem[P_ADDR + 2];
    res3 = mem[P_ADDR + 3];
    res4 = mem[P_ADDR + 4];
    __builtin_load(R3, res0);
    __builtin_load(R4, res1);
    __builtin_load(R5, res2);
    __builtin_load(R6, res3);
    __builtin_load(R7, res4);
}

__builtin_halt();
