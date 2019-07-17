/*
 * clang -O2 -s do_vmctl.c -o do_vmctl
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>

#ifndef VMCTL_PATH
#    define VMCTL_PATH "/usr/sbin/vmctl"
#endif

int main(int argc, char **argv) {
    if (unveil(VMCTL_PATH, "rx") < 0) {
        perror("unveil");
        return 1;
    }

    if (pledge("stdio exec id", NULL) < 0) {
        perror("pledge");
        return 1;
    }

    // Usage: do_vmctl login[:group] <vmctl args...>
    if (argc < 3) {
        fprintf(stderr, "Invalid usage\n");
        return 1;
    }

    // Find and set user/group
    // TODO!
    if (setgid(0) < 0 || setuid(0) < 0) {
        perror("setuid/setgid");
	return 1;
    }

    // Build arguments and environment
    const char **v_argv = calloc(1, (argc + 1) * sizeof(char *));
    v_argv[0] = VMCTL_PATH;
    for (int i = 2; i < argc; i++) {
        v_argv[i - 1] = argv[i];
    }

    char * const v_envp[] = {
        NULL
    };

    // Launch vmctl
    if (execve(v_argv[0], (char * const *) v_argv, v_envp) < 0) {
        perror("execve");
        return 1;
    }
}
