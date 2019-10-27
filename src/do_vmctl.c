/*
 * clang -O2 -s do_vmctl.c -o do_vmctl
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#ifndef VMCTL_PATH
#    define VMCTL_PATH "/usr/sbin/vmctl"
#endif

#define PW_BUF_SIZE (1024)

int main(int argc, char **argv) {
    if (unveil(VMCTL_PATH, "rx") < 0) {
        perror("unveil");
        return 1;
    }

    if (pledge("stdio exec id getpw", NULL) < 0) {
        perror("pledge");
        return 1;
    }

    // Usage: do_vmctl login[:group] <vmctl args...>
    if (argc < 3) {
        fprintf(stderr, "Invalid usage\n");
        return 1;
    }

    // Parse login[:group] argument
    char *login = strdup(argv[1]);
    char *group = NULL;

    if (strchr(login, ':') != NULL) {
        char *duplogin = strdup(login);
        free(login);
        char *l = duplogin;
        char *tok;
        int i = 0;
        while ((tok = strsep(&l, ":")) != NULL) {
            if (i == 0) {
                login = strdup(tok);
            } else if (i == 1) {
                group = strdup(tok);
            } else {
                fprintf(stderr, "Invalid usage\n");
                free(l);
                return 1;
            }
            i++;
        }
        free(duplogin);
    }

    // Find and set user/group
    int r;
    size_t buf_size = PW_BUF_SIZE;
    char *buf = malloc(buf_size);
    struct passwd pws, *pwsp;
    struct group gws, *gwsp;
    uid_t target_uid = -1;
    gid_t target_gid = -1;

read_pwdb:
    bzero(buf, buf_size);
    if ((r = getpwnam_r(login, &pws, buf, buf_size, &pwsp)) != 0) {
        if (r == ERANGE) {
            buf_size += 512;
            buf = realloc(buf, buf_size);
            goto read_pwdb;
        }
        perror("getpwnam_r");
        return 1;
    }
    if (pwsp == NULL) {
        // TODO: maybe it's a number?
        fprintf(stderr, "No such user '%s'\n", login);
        return 1;
    } else {
        target_uid = pwsp->pw_uid;
        target_gid = pwsp->pw_gid;
    }
    endpwent();

    if (group != NULL) {
read_groupdb:
        bzero(buf, buf_size);
        if ((r = getgrnam_r(group, &gws, buf, buf_size, &gwsp)) != 0) {
            if (r == ERANGE) {
                buf_size += 512;
                buf = realloc(buf, buf_size);
                goto read_groupdb;
            }
            perror("getgrnam_r");
            return 1;
        }
        if (gwsp == NULL) {
            // TODO: maybe it's a number?
            fprintf(stderr, "No such group '%s'\n", group);
            return 1;
        } else {
            target_gid = gwsp->gr_gid;
        }
        endgrent();
    }

    free(login);
    free(group);
    free(buf);
    if (getgid() != target_gid) {
        if (setgid(target_gid) < 0) {
            perror("setgid");
            return 1;
        }
    }
    if (getuid() != target_uid) {
        if (setuid(target_uid) < 0) {
            perror("setuid");
            return 1;
        }
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
