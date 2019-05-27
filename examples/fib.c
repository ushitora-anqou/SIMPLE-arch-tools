int a0, a1;
a0 = 0;
a1 = 1;

while (a0 < 55) {
    int t;
    t = a0;
    a0 = a1;
    a1 = a1 + t;
}

return a0;
