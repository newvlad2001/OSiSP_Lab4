//1->2 2->3 3->(4,5,6) 6->7 4->8
//1->(2,3,4,5) SIGUSR1 5->(6,7,8) SIGUSR1 8->1 SIGUSR1

#define _GNU_SOURCE
#define PRINT_TREE

#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

void delete_tmpfiles();

void set_handler(int sig_num, void (*handler_function)(int));

void write_pid(int pid, const char filename[]);

int read_pid(const char filename[]);

int get_time();

void send_sigusr1(int proc_num, int pid);

void send_sigusr1_g(int proc_num, int group);

void send_sigusr2(int proc_num, int pid);

void send_sigusr2_g(int proc_num, int group);

void sigusr1_received();

void sigusr2_received();

void sigterm_received();

const char *FILES[] = {"./tmp/zero_pid.txt", "./tmp/first_pid.txt", "./tmp/second_pid.txt", "./tmp/third_pid.txt",
                       "./tmp/fourth_pid.txt", "./tmp/fifth_pid.txt", "./tmp/sixth_pid.txt", "./tmp/seventh_pid.txt",
                       "./tmp/eighth_pid.txt"};
int curr_proc_num = 0;
int first_received = 0;
int sigusr1_am = 0;
int sigusr2_am = 0;
int ready2 = 0;
int ready3 = 0;
int ready4 = 0;
int ready6 = 0;
int ready7 = 0;

//HANDLERS------------------------------------------------

void sigterm8(int sig_num) {
    sigterm_received();
    exit(0);
}

void sigterm7(int sig_num) {
    sigterm_received();
    exit(0);
}

void sigterm6(int sig_num) {
    sigterm_received();
    exit(0);
}

void sigterm5(int sig_num) {
    sigterm_received();
    exit(0);
}

void sigterm4(int sig_num) {
    sigterm_received();
    int eighth_pid = read_pid(FILES[8]);
    kill(eighth_pid, SIGTERM);
    wait(0);
    exit(0);
}

void sigterm3(int sig_num) {
    sigterm_received();
    int seventh_pid = read_pid(FILES[7]);
    kill(seventh_pid, SIGTERM);
    wait(0);
    exit(0);
}

void sigterm2(int sig_num) {
    sigterm_received();
    int sixth_pid = read_pid(FILES[6]);
    kill(sixth_pid, SIGTERM);
    wait(0);
    exit(0);
}

void eighth_sigusr1(int sig_num) {
    sigusr1_received();
    while (ready7 == 0) {}
    ready7 = 0;
    int first_pid = read_pid(FILES[1]);
    send_sigusr1(8, first_pid);
}

void seventh_sigusr1(int sig_num) {
    sigusr1_received();
    while (ready6 == 0) {}
    ready6 = 0;
    int eighth_pid = read_pid(FILES[8]);
    kill(eighth_pid, SIGUSR2);
}

void sixth_sigusr1(int sig_num) {
    sigusr1_received();
    int seventh_pid = read_pid(FILES[7]);
    kill(seventh_pid, SIGUSR2);
}

void fifth_sigusr1(int sig_num) {
    sigusr1_received();
    while (ready4 == 0) {}
    ready4 = 0;
    int first_pid = read_pid(FILES[1]);
    send_sigusr1_g(5, first_pid);
    int seventh_pid = read_pid(FILES[7]);
}

void fourth_sigusr1(int sig_num) {
    sigusr1_received();
    while (ready3 == 0) {}
    ready3 = 0;
    int fifth_pid = read_pid(FILES[5]);
    kill(fifth_pid, SIGUSR2);
}

void third_sigusr1(int sig_num) {
    sigusr1_received();
    while (ready2 == 0) {}
    ready2 = 0;
    int fourth_pid = read_pid(FILES[4]);
    kill(fourth_pid, SIGUSR2);
}

void second_sigusr1(int sig_num) {
    sigusr1_received();
    int third_pid = read_pid(FILES[3]);
    kill(third_pid, SIGUSR2);
}


void first_sigusr1(int sig_num) {
    sigusr1_received();

    first_received++;
    int second_pid = read_pid(FILES[2]);
    if (first_received == 101) {
        killpg(second_pid, SIGTERM);
        for (int i = 0; i < 4; i++) {
            wait(0);
        }
        sigterm_received();
        exit(0);
    }

    send_sigusr1_g(1, second_pid);
}

void third_ready(int sig_num) {
    ready3 = 1;
}

void second_ready(int sig_num) {
    ready2 = 1;
}

void fourth_ready(int sig_num) {
    ready4 = 1;
}

void six_ready(int sig_num) {
    ready6 = 1;
}

void seventh_ready(int sig_num) {
    ready7 = 1;
}

//END_HANDLERS----------------------------------------

//INIT----------------------------------------
void init8() {
    curr_proc_num = 8;

    set_handler(SIGUSR1, eighth_sigusr1);
    set_handler(SIGUSR2, seventh_ready);
    set_handler(SIGTERM, sigterm8);

    struct stat buf;
    while (lstat(FILES[5], &buf) == -1) {}
    while (lstat(FILES[7], &buf) == -1) {}

#ifdef PRINT_TREE
    int zeroth_pid = read_pid(FILES[0]);
    char ps_tree_cmd[64] = {0};
    sprintf(ps_tree_cmd, "pstree -p %d", zeroth_pid);
    system(ps_tree_cmd);
#endif

    write_pid(getpid(), FILES[8]);

    while (1) {
        pause();
    }
}

void init7() {
    curr_proc_num = 7;
    set_handler(SIGUSR1, seventh_sigusr1);
    set_handler(SIGUSR2, six_ready);
    set_handler(SIGTERM, sigterm7);

    write_pid(getpid(), FILES[7]);

    while (1) {
        pause();
    }
}

void init6() {
    curr_proc_num = 6;

    set_handler(SIGUSR1, sixth_sigusr1);
    set_handler(SIGTERM, sigterm6);

    int seventh_pid = fork();
    if (seventh_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (seventh_pid == 0) {
        init7();
    }

    int first_pid = read_pid(FILES[1]);
    if (setpgid(seventh_pid, first_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid(getpid(), FILES[6]);

    while (1) {
        pause();
    }

}

void init5() {
    curr_proc_num = 5;

    set_handler(SIGUSR1, fifth_sigusr1);
    set_handler(SIGUSR2, fourth_ready);
    set_handler(SIGTERM, sigterm5);

    write_pid(getpid(), FILES[5]);

    while (1) {
        pause();
    }
}

void init4() {
    curr_proc_num = 4;

    set_handler(SIGUSR1, fourth_sigusr1);
    set_handler(SIGUSR2, third_ready);
    set_handler(SIGTERM, sigterm4);

    int eighth_pid = fork();
    if (eighth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (eighth_pid == 0) {
        init8();
    }

    int first_pid = read_pid(FILES[1]);
    if (setpgid(eighth_pid, first_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }
    struct stat buf;
    while (lstat(FILES[7], &buf) == -1) {}

    write_pid(getpid(), FILES[4]);

    while (1) {
        pause();
    }
}

void init3() {
    curr_proc_num = 3;

    set_handler(SIGUSR1, third_sigusr1);
    set_handler(SIGUSR2, second_ready);
    set_handler(SIGTERM, sigterm3);
    write_pid(getpid(), FILES[3]);

    int fourth_pid = fork();
    if (fourth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (fourth_pid == 0) {
        init4();
    }


    int fifth_pid = fork();
    if (fifth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (fifth_pid == 0) {
        init5();
    }


    int sixth_pid = fork();
    if (sixth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (sixth_pid == 0) {
        init6();
    }

    int first_pid = read_pid(FILES[1]);
    if (setpgid(sixth_pid, first_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    int second_pid = read_pid(FILES[2]);
    if (setpgid(fourth_pid, second_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    if (setpgid(fifth_pid, second_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    while (1) {
        pause();
    }
}

void init2() {
    curr_proc_num = 2;

    set_handler(SIGUSR1, second_sigusr1);
    set_handler(SIGTERM, sigterm2);
    write_pid(getpid(), FILES[2]);

    int third_pid = fork();
    if (third_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (third_pid == 0) {
        init3();
    }

    if (setpgid(third_pid, getpid()) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    while (1) {
        pause();
    }
}

void init1() {
    curr_proc_num = 1;

    set_handler(SIGUSR1, first_sigusr1);

    if (setpgid(getpid(), getpid()) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid(getpid(), FILES[1]);

    int second_pid = fork();
    if (second_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (second_pid == 0) {
        init2();
    }
    if (setpgid(second_pid, second_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    struct stat buf;
    while (lstat(FILES[8], &buf) == -1) {}

    int zero_pid = read_pid(FILES[0]);
    if (setpgid(getpid(), zero_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    send_sigusr1_g(1, second_pid);

    while (1) {
        pause();
    }
}

//END_INIT-----------------------------------------------------------------

int main(int argc, char *argv[]) {
    write_pid(getpid(), FILES[0]);
    int first_pid = fork();
    if (first_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (first_pid == 0) {
        init1();
    } else {
        wait(0);
    }
    delete_tmpfiles();
    exit(0);
}

void delete_tmpfiles() {
    for (int i = 0; i < 9; i++) {
        errno = 0;
        if (remove(FILES[i]) == -1) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), FILES[i]);
        }
    }
}

void set_handler(int sig_num, void (*handler_function)(int)) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler_function;
    sigaction(sig_num, &action, 0);
}

void write_pid(int pid, const char filename[]) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), filename);
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d", pid);
    if (fclose(file) == EOF) {
        fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), filename);
        exit(EXIT_FAILURE);
    }
}

int read_pid(const char filename[]) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), filename);
        exit(EXIT_FAILURE);
    }
    char buf[64];
    fgets(buf, 64, file);
    errno = 0;
    long pid = strtol(buf, NULL, 10);
    if (errno != 0) {
        fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), filename);
        exit(EXIT_FAILURE);
    }
    if (fclose(file) == EOF) {
        fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), filename);
        exit(EXIT_FAILURE);
    }
    return (int) pid;
}

int get_time() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (int) (currentTime.tv_usec % 1000L);
}

void send_sigusr1_g(int proc_num, int group) {
    fprintf(stdout, "%d %d %d sent SIGUSR1 %d\n", proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
    killpg(group, SIGUSR1);
    sigusr1_am++;
}

void send_sigusr1(int proc_num, int pid) {
    fprintf(stdout, "%d %d %d sent SIGUSR1 %d\n", proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
    kill(pid, SIGUSR1);
    sigusr1_am++;
}

void send_sigusr2(int proc_num, int pid) {
    fprintf(stdout, "%d %d %d sent SIGUSR2 %d\n", proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
    kill(pid, SIGUSR2);
    sigusr2_am++;
}

void send_sigusr2_g(int proc_num, int group) {
    fprintf(stdout, "%d %d %d sent SIGUSR2 %d\n", proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
    killpg(group, SIGUSR2);
    sigusr2_am++;
}

void sigusr1_received() {
    fprintf(stdout, "%d %d %d received SIGUSR1 %d\n", curr_proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
}

void sigusr2_received() {
    fprintf(stdout, "%d %d %d received SIGUSR2 %d\n", curr_proc_num, getpid(), getppid(), get_time());
    fflush(stdout);
}

void sigterm_received() {
    fprintf(stdout, "%d %d terminated after %d SIGUSR1 and %d SIGUSR2\n", getpid(), getppid(), sigusr1_am, sigusr2_am);
    fflush(stdout);
}
