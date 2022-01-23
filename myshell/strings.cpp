#include <iostream>
#include <string.h>
using namespace std;

int n, m;
int Index(const char* s, const char* p) {
    int i = 0, j = 0;
    while (i < n && j < m) {
        printf("%c %c\n", s[i], p[j]);
        if (s[i] == p[j]) {
            i++, j++;
        } else {
            i = i - j + 1;
            j = 0;
        }
    }

    if (j >= m) {
        return i - m;
    }
    else
        return -1;
}

int main() {
    char *s1 = "find";
    char *s2 = "f";
    n = strlen(s1);
    m = strlen(s2);
    cout << Index(s1, s2) << endl;
}
