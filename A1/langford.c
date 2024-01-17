#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

void print_usage(char *program_name) {
    fprintf(stderr, "usage: %s [-h] -c n | num...\n", program_name);
}

void print_sequence(int *sequence, int length) {
    printf("Your sequence: [");
    for (int i = 0; i < length; i++) {
        printf("%d", sequence[i]);
        if (i < length - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}


bool is_langford_pairing(int size, const int sequence[]) {
    // must be even
    if (size % 2 != 0) {
        return false;
    }

    int n = size / 2;

    int *counts = calloc(n + 1, sizeof(int));

    for (int i = 0; i < size; i++) {
        int num = sequence[i];

        // check if the number is within the valid range and count
        if (num < 1 || num > n || counts[num - 1] >= 2) {
            free(counts);
            return false;
        }


        if (counts[num - 1] == 0) {
            // check if the distance to the next same number is valid
            int nextIndex = i + num + 1;
            if (nextIndex >= size || sequence[nextIndex] != num) {
                free(counts);
                return false;
            }
        } else  {
            // check if the distance to the previous same number is valid
            int prevIndex = i - num - 1;
            if (prevIndex < 0 || sequence[prevIndex] != num) {
                free(counts);
                return false;
            }
        }
        
        counts[num - 1]++;
    }

    free(counts);
    return true;
}

void print_numbers(int *array, int size, bool last) {
    // prints an array of numbers separated by commas unless specified for last
    for (int i = 0; i < size; ++i) {
        if (i == size - 1 && last) {
            printf("%d", array[i]);
            continue;
        }
        printf("%d, ", array[i]);
    }
}

void print_number(int num) {
    // prints a number followed by a comma
    printf("%d, ", num);
}


int* reverse(const int *array, int size) {
    // reverses an array of integers
    int* reversed = malloc(size * sizeof(int));
    if (reversed == NULL) {
        return NULL;
    }

    for (int i = 0; i < size; ++i) {
        reversed[i] = array[size - 1 - i];
    }

    return reversed;
}


void generate_langford_pairing(int n) {
    // immediately invalidates pairing
    if (n % 4 == 2 || n % 4 == 1) {
        printf("No results found.\n");
        exit(0);
    }

    // algorithm as shown in the video 
    
    int x = (n + 3) / 4;
    int a = (2 * x - 1), b = (4 * x - 2), c = (4 * x - 1), d = (4 * x);
    int p[a-2], q[a-3], r[b - a - 3 ], s[b - a - 2];
    int p_i = 0, q_i = 0, r_i = 0, s_i = 0;


    for (int i = 1; i < a; i += 2) {
        p[p_i++] = i;
    }

    for (int i = 2; i < a; i += 2) {
        q[q_i++] = i;
    }

    for (int i = a + 2; i < b; i++) {
        if (i % 2 == 1) {
            r[r_i++] = i;
        }
    }

    for (int i = a + 1; i < b; i++) {
        if (i % 2 == 0) {
            s[s_i++] = i;
        }
    }

    int *r_p = reverse(p, p_i), *r_q = reverse(q, q_i), *r_r = reverse(r, r_i), *r_s = reverse(s, s_i);

    if (n % 4 == 0) {
        printf("[");
        print_numbers(r_s, s_i,false);
        print_numbers(r_p, p_i, false);
        print_number(b);
        print_numbers(p, p_i, false);
        print_number(c);
        print_numbers(s, s_i, false);
        print_number(d);
        print_numbers(r_r, r_i, false);   
        print_numbers(r_q, q_i, false);
        print_number(b);
        print_number(a);
        print_numbers(q, q_i, false);
        print_number(c);
        print_numbers(r, r_i, false);
        print_number(a);
        printf("%d, ", d);
        printf("]\n");
        exit(0);
    }

    if (n % 4 == 3) {
        printf("[");
        print_numbers(r_s, s_i, false);
        print_numbers(r_p, p_i, false);
        print_number(b);
        print_numbers(p, p_i, false);
        print_number(c);
        print_numbers(s, s_i, false);
        print_number(a);
        print_numbers(r_r, r_i, false);   
        print_numbers(r_q, q_i, false);
        print_number(b);
        print_number(a);
        print_numbers(q, q_i, false);
        print_number(c);
        print_numbers(r, r_i, true);
        printf("]\n");
        exit(0);
    }
}

int main(int argc, char *argv[]) {

    // help or no arguments
    if (argc < 2 || strcmp(argv[1], "-h") == 0) {
        print_usage(argv[0]);
        exit(0);
    }

    // creation
    if (strcmp(argv[1], "-c") == 0) {

        if (argc < 3) {
            fprintf(stderr, "%s: -c option requires an argument.\n", argv[0]);
            exit(1);
        } else if (argc > 3) {
            fprintf(stderr, "%s: -c option received too many arguments.\n", argv[0]);
            exit(1);
        }
         
        char *endptr;
        long n = strtol(argv[2], &endptr, 10);

        if (endptr && endptr[0] != '\0') {
            fprintf(stderr, "error: %s is not a valid integer.\n", argv[2]);
            exit(1);
        }

        printf("Creating a langford pairing with n=%ld\n", n);
        generate_langford_pairing(n);
    } else {
        // validation
        int *seq = malloc((argc - 1) * sizeof(int));
        for (int i = 1; i < argc; i++) {
            char *endptr;
            long num = strtol(argv[i], &endptr, 10);
            if (endptr && endptr[0] != '\0') {
                fprintf(stderr, "error: %s is not a valid integer.\n", argv[i]);
                free(seq);
                exit(1);
            }
            seq[i - 1] = num;
        }
        print_sequence(seq, argc - 1);

        if (is_langford_pairing(argc - 1, seq)) {
        printf("It is a langford pairing!\n");
        } else {
            printf("It is NOT a langford pairing.\n");
        }

        free(seq);
    }

    return 0;
}