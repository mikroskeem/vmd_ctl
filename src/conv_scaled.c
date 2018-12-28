/*
 * clang -O2 -s -lutil conv_scaled.c -o conv_scaled
 */

#include <util.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Invalid usage\n");
        return 1;
    }

    char *num = argv[1];
    long long val = -1;

    if(scan_scaled(num, &val) == 0) {
        printf("%lld\n", val);
        return 0;
    } else {
        fprintf(stderr, "Invalid value: %s\n", strerror(errno));
        return 1;
    }
}
