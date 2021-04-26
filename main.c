#define _GNU_SOURCE
//#define DEBUG

#include <stdlib.h>
#include <wait.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#define ZERO_PID_FILE "zero_pid.txt"
#define FIRST_PID_FILE "first_pid.txt"
#define SECOND_PID_FILE "second_pid.txt"
#define THIRD_PID_FILE "third_pid.txt"
#define FOURTH_PID_FILE "fourth_pid.txt"
#define FIFTH_PID_FILE "fifth_pid.txt"
#define SIXTH_PID_FILE "sixth_pid.txt"
#define SEVENTH_PID_FILE "seventh_pid.txt"
#define EIGHTH_PID_FILE "eighth_pid.txt"

const char *FILES[] = {ZERO_PID_FILE, FIRST_PID_FILE, SECOND_PID_FILE, THIRD_PID_FILE, FOURTH_PID_FILE, FIFTH_PID_FILE,
                       SIXTH_PID_FILE, SEVENTH_PID_FILE, EIGHTH_PID_FILE};
int curr_proc_num = 0;
int recieved_by_first = 0;
int send_total_sigusr1 = 0;
int send_total_sigusr2 = 0;

void delete_files() {
    for (int i = 0; i < 9; i++) {
        errno = 0;
        if (remove(FILES[i]) == -1) {
            fprintf(stderr, "%d: %s: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno), FILES[i]);
        }
    }
}

void init_action(int sig_num, void (*handler_function)()) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handler_function;
    sigaction(sig_num, &action, 0);
}

void write_pid_file(int pid, char *filename) {
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

int read_pid_file(char *filename) {
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

int get_time_in_millis() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (int) (currentTime.tv_usec % 1000L);
}

void send_group_sigusr1(int proc_num, int group) {
    fprintf(stdout, "%d %d %d sent SIGUSR1 %d\n", proc_num, getpid(), getppid(), get_time_in_millis());
    fflush(stdout);
    killpg(group, SIGUSR1);
    send_total_sigusr1++;
}

void send_sigusr2(int proc_num, int pid) {
    fprintf(stdout, "%d %d %d sent SIGUSR2 %d\n", proc_num, getpid(), getppid(), get_time_in_millis());
    fflush(stdout);
    kill(pid, SIGUSR2);
    send_total_sigusr2++;
}

void send_group_sigusr2(int proc_num, int group) {
    fprintf(stdout, "%d %d %d sent SIGUSR2 %d\n", proc_num, getpid(), getppid(), get_time_in_millis());
    fflush(stdout);
    killpg(group, SIGUSR2);
    send_total_sigusr2++;
}

void receive_sigusr1() {
    fprintf(stdout, "%d %d %d received SIGUSR1 %d\n", curr_proc_num, getpid(), getppid(), get_time_in_millis());
    fflush(stdout);
}

void receive_sigusr2(){
    fprintf(stdout, "%d %d %d received SIGUSR2 %d\n", curr_proc_num, getpid(), getppid(), get_time_in_millis());
    fflush(stdout);
}

void receive_sigterm() {
    fprintf(stdout, "%d %d terminated after %d SIGUSR1 and %d SIGUSR2\n", getpid(), getppid(), send_total_sigusr1, send_total_sigusr2);
    fflush(stdout);
}

void eighth_proc_handler_sigterm() {
    receive_sigterm();
    int fifth_pid = read_pid_file(FIFTH_PID_FILE);
    killpg(fifth_pid, SIGTERM);
    for (int i = 0; i < 2; ++i) {
        wait(0);
    }
    exit(EXIT_SUCCESS);
}

void seventh_proc_handler_sigterm() {
    receive_sigterm();
    exit(EXIT_SUCCESS);
}

void sixth_proc_handler_sigterm() {
    receive_sigterm();
    exit(EXIT_SUCCESS);
}

void fifth_proc_handler_sigterm() {
    receive_sigterm();
    int second_pid = read_pid_file(SECOND_PID_FILE);
    killpg(second_pid, SIGTERM);
    for (int i = 0; i < 3; ++i) {
        wait(0);
    }
    exit(EXIT_SUCCESS);
}

void fourth_proc_handler_sigterm() {
    receive_sigterm();
    exit(EXIT_SUCCESS);
}

void third_proc_handler_sigterm() {
    receive_sigterm();
    exit(EXIT_SUCCESS);
}

void second_proc_handler_sigterm() {
    receive_sigterm();
    exit(EXIT_SUCCESS);
}

void eighth_proc_handler_sigusr1() {
    receive_sigusr1();
    int fifth_pid = read_pid_file(FIFTH_PID_FILE);
    send_group_sigusr1(8, fifth_pid);
}

void seventh_proc_handler_sigusr1() {
    receive_sigusr1();
}

void sixth_proc_handler_sigusr1() {
    receive_sigusr1();
}

void fifth_proc_handler_sigusr1() {
    receive_sigusr1();
    int second_pid = read_pid_file(SECOND_PID_FILE);
    send_group_sigusr2(5, second_pid);
}

void fourth_proc_handler_sigusr2() {
    receive_sigusr2();
}

void third_proc_handler_sigusr2() {
    receive_sigusr2();
}



void second_proc_handler_sigusr2() {
    receive_sigusr2();
    int first_pid = read_pid_file(FIRST_PID_FILE);
    send_sigusr2(2, first_pid);
}


void first_proc_handler_sigusr2() {
    receive_sigusr2();

    recieved_by_first++;
    int seventh_pid = read_pid_file(SEVENTH_PID_FILE);
    if (recieved_by_first == 101) {
        killpg(seventh_pid, SIGTERM);
        for (int i = 0; i < 2; i++) {
            wait(0);
        }
        receive_sigterm();
        exit(EXIT_SUCCESS);
    }

    send_group_sigusr1(1, seventh_pid);
}

void init_eighth_proc() {
    curr_proc_num = 8;

    init_action(SIGUSR1, eighth_proc_handler_sigusr1);
    init_action(SIGTERM, eighth_proc_handler_sigterm);

    struct stat buf;
    while (lstat(FIFTH_PID_FILE, &buf) == -1) {}
    while (lstat(SEVENTH_PID_FILE, &buf) == -1) {}

#ifdef DEBUG
    int zeroth_pid = read_pid_file(ZERO_PID_FILE);
    char ps_tree_cmd[64] = {0};
    sprintf(ps_tree_cmd, "pstree -p %d", zeroth_pid);
    system(ps_tree_cmd);
#endif

    write_pid_file(getpid(), EIGHTH_PID_FILE);

    while (1) {
        pause();
    }
}

void init_seventh_proc() {
    curr_proc_num = 7;

    init_action(SIGUSR1, seventh_proc_handler_sigusr1);
    init_action(SIGTERM, seventh_proc_handler_sigterm);

    write_pid_file(getpid(), SEVENTH_PID_FILE);

    while (1) {
        pause();
    }
}

void init_sixth_proc() {
    curr_proc_num = 6;

    init_action(SIGUSR1, sixth_proc_handler_sigusr1);
    init_action(SIGTERM, sixth_proc_handler_sigterm);

    int seventh_pid = fork();
    if (seventh_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (seventh_pid == 0) {
        init_seventh_proc();
    }

    if (setpgid(seventh_pid, seventh_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid_file(getpid(), SIXTH_PID_FILE);

    while (1) {
        pause();
    }

}

void init_fifth_proc() {
    curr_proc_num = 5;

    init_action(SIGUSR1, fifth_proc_handler_sigusr1);
    init_action(SIGTERM, fifth_proc_handler_sigterm);

    write_pid_file(getpid(), FIFTH_PID_FILE);

    while (1) {
        pause();
    }
}

void init_fourth_proc() {
    curr_proc_num = 4;

    init_action(SIGUSR2, fourth_proc_handler_sigusr2);
    init_action(SIGTERM, fourth_proc_handler_sigterm);

    int eighth_pid = fork();
    if (eighth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (eighth_pid == 0) {
        init_eighth_proc();
    }

    struct stat buf;
    while (lstat(SEVENTH_PID_FILE, &buf) == -1) {}

    int seventh_pid = read_pid_file(SEVENTH_PID_FILE);
    if (setpgid(eighth_pid, seventh_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid_file(getpid(), FOURTH_PID_FILE);

    while (1) {
        pause();
    }
}

void init_third_proc() {
    curr_proc_num = 3;

    init_action(SIGUSR2, third_proc_handler_sigusr2);
    init_action(SIGTERM, third_proc_handler_sigterm);

    int fourth_pid = fork();
    if (fourth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (fourth_pid == 0) {
        init_fourth_proc();
    }

    struct stat buf;
    while (lstat(SECOND_PID_FILE, &buf) == -1) {}

    int second_pid = read_pid_file(SECOND_PID_FILE);
    if (setpgid(fourth_pid, second_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    int fifth_pid = fork();
    if (fifth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (fifth_pid == 0) {
        init_fifth_proc();
    }

    if (setpgid(fifth_pid, fifth_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    int sixth_pid = fork();
    if (sixth_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (sixth_pid == 0) {
        init_sixth_proc();
    }

    if (setpgid(sixth_pid, fifth_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid_file(getpid(), THIRD_PID_FILE);

    while (1) {
        pause();
    }
}

void init_second_proc() {
    curr_proc_num = 2;

    init_action(SIGUSR2, second_proc_handler_sigusr2);
    init_action(SIGTERM, second_proc_handler_sigterm);

    int third_pid = fork();
    if (third_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (third_pid == 0) {
        init_third_proc();
    }
    if (setpgid(third_pid, getpid()) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid_file(getpid(), SECOND_PID_FILE);
    while (1) {
        pause();
    }
}

void init_first_proc() {
    curr_proc_num = 1;

    init_action(SIGUSR2, first_proc_handler_sigusr2);

    int second_pid = fork();

    if (second_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (second_pid == 0) {
        init_second_proc();
    }
    if (setpgid(second_pid, second_pid) == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
    }

    write_pid_file(getpid(), FIRST_PID_FILE);

    struct stat buf;
    while (lstat(EIGHTH_PID_FILE, &buf) == -1) {}



    int seventh_pid = read_pid_file(SEVENTH_PID_FILE);
    send_group_sigusr1(1, seventh_pid);


    while (1) {
        pause();
    }
}

int main(int argc, char *argv[]) {
    write_pid_file(getpid(), ZERO_PID_FILE);
    int first_pid = fork();
    if (first_pid == -1) {
        fprintf(stderr, "%d: %s: %s\n", getpid(), program_invocation_short_name, strerror(errno));
        exit(EXIT_FAILURE);
    } else if (first_pid == 0) {
        init_first_proc();
    } else {
        wait(0);
    }
    delete_files();
    exit(EXIT_SUCCESS);
}