int n = 10;
int a = 0;
int b = 1;
int i = 0;
while (i < n) {
    print(a);
    int t = a + b;
    a = b;
    b = t;
    i = i + 1;
}
if (a > 50 && !(b == 0))
    print(a * 2 - (3 + 4) / 2);
else
    print(-1);
