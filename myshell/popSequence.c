/**
5 7 5
1 2 3 4 5 6 7
3 2 1 7 5 6 4
7 6 5 4 3 2 1
5 6 4 3 7 2 1
1 7 6 5 4 3 2
 */
#include <stdio.h>

int main() {
    int M, N, K;
    scanf("%d %d %d", &M, &N, &K);
    int seq[1010];
    for (int i = 0; i < N; i++) {
        // ½øÕ»ÐòÁÐ
        seq[i] = i + 1;
    }

    for (int T = 0; T < K; T++) {
        int x;
        int stack[1010] = { 0 };
        int poped[1010] = { 0 };
        int stack_it = -1;
        for (int i = 0; i < N; i++) {
            scanf("%d", &poped[i]);
        }
        int poped_it = 0, seq_it = 0;
        int flag = 1;
        while (seq_it < N) {
            stack[++stack_it] = seq[seq_it++];
            if (stack_it >= M) {
                flag = 0;
                puts("NO");
                break;
            }
            while (stack_it >= 0 && poped_it < N
            && stack[stack_it] == poped[poped_it]) {
                stack[stack_it--] = 0;
                poped_it++;
            }
        }
        if (flag) {
            if (stack_it != -1) {
                puts("NO");
            } else {
                puts("YES");
            }
        }
    }
}
