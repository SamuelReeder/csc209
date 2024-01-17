#include <stdio.h>


int main() {
    char phone[11];
    int i;

    if (scanf("%10s", phone) != 1) {
        return 1;
    }

    while (scanf("%d", &i) != EOF) {
        if (i == -1) {
            printf("%s\n", phone);
        } else if (i >= 0 && i < 10) {
            printf("%c\n", phone[i]);
        } else {
            printf("ERROR\n");
            return 1;
        }
    }

    return 0;
}
