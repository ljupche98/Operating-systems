#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
 
#define maxn (1 << 10)
 
bool bg;
char write_to[maxn], read_from[maxn];
int write_desc, read_desc;
int copy_write_desc, copy_read_desc;
 
int ret_status;
char working_dir[maxn];
char name[maxn];
char line[maxn];
 
int argc;
char *args[maxn];
char prog_path[maxn];
 
void eval();
void prepare_exec();
void prepare_desc();
void rec_desc();
 
void parse()
{
    int len = strlen(line);
 
    if (line[len - 1] == '\n') line[len - 1] = 0;
 
    for (int i = 0; i < len && line[i]; i++) {
        int j = i, vc = 0;
        while (j < len && line[j] == ' ') j++;
 
        args[argc++] = line + j;
        for (; j < len && line[j] && (vc || line[j] != ' '); j++) vc ^= line[j] == '"';
        line[i = j++] = 0;
    }
 
    if (!strcmp(args[argc - 1], "&")) bg = true, argc--;
    if (args[argc - 1][0] == '>') strcpy(write_to, args[argc - 1] + 1), argc--;
    if (args[argc - 1][0] == '<') strcpy(read_from, args[argc - 1] + 1), argc--;
    args[argc] = NULL;
}
 
void filterArgs()
{
    for (int i = 0; i < argc; i++)
        if (args[i][0] == '"' && args[i][strlen(args[i]) - 1] == '"')
            args[i][strlen(args[i]) - 1] = 0,
            args[i]++;
}
 
void sigchld_handler(int signum)
{
    int pid, status, serrno = errno;
 
    while (true) {
        pid = waitpid(WAIT_ANY, &status, WNOHANG);
        if (pid <= 0) break;
    }
 
    errno = serrno;
}
 
void initialize(char *ar)
{
    strcpy(name, "mysh");
    getcwd(working_dir, sizeof working_dir);
    getcwd(prog_path, sizeof prog_path);
    strcpy(prog_path + strlen(prog_path), ar + 1);
    signal(SIGCHLD, sigchld_handler);
}
 
void reset()
{
    argc = 0;
    bg = false;
    read_desc = 0;
    copy_read_desc = -1;
    write_desc = 1;
    copy_write_desc = -1;
    memset(write_to, 0, sizeof write_to);
    memset(read_from, 0, sizeof read_from);
}
 
void funsupported()
{
    char exec_path[maxn];
 
    if (args[0][0] != '/') {
        strcpy(exec_path, "/bin/");
        strcpy(exec_path + strlen(exec_path), args[0]);
    } else strcpy(exec_path, args[0]);
 
    int x = fork();
    if (x < 0) perror("fork"); else
    if (!x) {
        execv(exec_path, args);
 
        char new_path[maxn];
        strcpy(new_path, "/usr");
        strcpy(new_path + strlen(new_path), exec_path);
 
        if (execv(new_path, args)) exit(0);
    } else {
        if (bg) return;
 
        int status = 0;
        waitpid(x, &status, 0);
 
        if (WIFEXITED(status)) ret_status = WEXITSTATUS(status);
    }
}
 
void fname()
{
    if (argc <= 1) printf("%s\n", name);
    else strcpy(name, args[1]);
    ret_status = 0;
}
 
void fhelp()
{
    printf("This isn't helpful...\n");
    ret_status = 0;
}
 
void fstatus()
{
    printf("%d\n", ret_status);
    ret_status = 0;
}
 
void fexit()
{
    assert(argc >= 2);
    exit(atoi(args[1]));
}
 
void fprint()
{
    for (int i = 1; i < argc; i++) {
        if (args[i][0] != '"') printf("%s", args[i]);
        else for (int j = 1; args[i][j + 1]; j++) printf("%c", args[i][j]);
 
        if (i < argc - 1) printf(" ");
    }
 
    ret_status = 0;
}
 
void fecho()
{
    for (int i = 1; i < argc; i++) {
        if (args[i][0] != '"') printf("%s", args[i]);
        else for (int j = 1; args[i][j + 1]; j++) printf("%c", args[i][j]);
 
        if (i < argc - 1) printf(" ");
    }
 
    printf("\n");
    ret_status = 0;
}
 
void fpid()
{
    printf("%d\n", getpid());
    ret_status = 0;
}
 
void fppid()
{
    printf("%d\n", getppid());
    ret_status = 0;
}
 
void fdirchange()
{
    if (argc <= 1) {
        if (chdir("/")) ret_status = errno, perror("dirchange");
        else ret_status = 0;
        getcwd(working_dir, sizeof working_dir);
        return;
    }
 
    if (chdir(args[1])) ret_status = errno, perror("dirchange");
    else ret_status = 0;
    getcwd(working_dir, sizeof working_dir);
}
 
void fdirwhere()
{
    printf("%s\n", working_dir);
    ret_status = 0;
}
 
void fdirmake()
{
    if (mkdir(args[1], S_IRWXU | S_IRWXG | S_IRWXO)) ret_status = errno, perror("dirmake");
    else ret_status = 0;
}
 
void fdirremove()
{
    if (rmdir(args[1])) ret_status = errno, perror("dirremove");
    else ret_status = 0;
}
 
void fdirlist()
{
    char dir_name[maxn];
    strcpy(dir_name, argc <= 1 ? working_dir : args[1]);
 
    DIR *dir;
    struct dirent *fd;
 
    if (dir = opendir(dir_name)) {
        while (fd = readdir(dir)) printf("%s  ", fd->d_name);
        printf("\n");
        closedir(dir);
        ret_status = 0;
    } else ret_status = errno, perror("dirlist");
}
 
void prepare_args(char *from, char *which)
{
    if (which[0] == '/') strcpy(from, which);
    else {
        strcpy(from, working_dir);
        strcpy(from + strlen(from), "/");
        strcpy(from + strlen(from), which);
    }
}
 
void flinkhard()
{
    char from[maxn], to[maxn];
    prepare_args(from, args[1]);
    prepare_args(to, args[2]);
 
    if (link(from, to)) ret_status = errno, perror("linkhard");
    else ret_status = 0;
}
 
void flinksoft()
{
    char from[maxn], to[maxn];
 
    int count = 0;
    for (int i = 1; i < argc && count < 2; i++)
        if (args[i][0] != '-') {
            if (!count) prepare_args(from, args[i]);
            else prepare_args(to, args[i]);
 
            count++;
        }
 
    if (symlink(from, to)) ret_status = errno, perror("linksoft");
    else ret_status = 0;
}
 
char *filter(char *path)
{
    int i;
    for (i = 0; path[i] && working_dir[i] && path[i] == working_dir[i]; i++);
    return path + i + (path[i] == '/') - (i == 1);
;
}
 
void flinkread()
{
    char from[maxn], point[maxn];
    prepare_args(from, args[1]);
 
    int size = readlink(from, point, sizeof point);
    if (size < 0) ret_status = errno, perror("linkread");
    else point[size] = 0, ret_status = 0, printf("%s\n", filter(point));
}
 
void flinklist()
{
    char path[maxn];
    prepare_args(path, args[1]);
 
    struct stat cmp_stat;
    if (lstat(path, &cmp_stat)) ret_status = errno, perror("lstat");
    else ret_status = 0;
 
    DIR *dir;
    struct dirent *fd;
 
    if (dir = opendir(working_dir)) {
        while (fd = readdir(dir)) {
            struct stat dir_stat;
 
            char file_path[maxn];
            prepare_args(file_path, working_dir);
            strcpy(file_path + strlen(file_path), "/");
            strcpy(file_path + strlen(file_path), fd->d_name);
 
            if (lstat(file_path, &dir_stat)) ret_status = errno, perror("lstat");
            else {
                ret_status = 0;
 
                if (cmp_stat.st_ino == dir_stat.st_ino) printf("%s  ", fd->d_name);
            }
        }
    } else ret_status = errno, perror("opendir");
 
    printf("\n");
}
 
void funlink()
{
    char path[maxn];
    prepare_args(path, args[1]);
 
    if (unlink(path)) ret_status = errno, perror("unlink");
    else ret_status = 0;
}
 
void frename()
{
    char from[maxn], to[maxn];
    strcpy(from, working_dir);
    strcpy(from + strlen(from), "/");
    strcpy(from + strlen(from), args[1]);
    strcpy(to, working_dir);
    strcpy(to + strlen(to), "/");
    strcpy(to + strlen(to), args[2]);
 
    if (rename(from, to)) ret_status = errno, perror("rename");
    else ret_status = 0;
}
 
void fcpcat()
{
    int s = 0, t = 1;
 
    if (argc == 2 || argc >= 3 && strcmp(args[1], "-")) {
        char dir_name[maxn];
        prepare_args(dir_name, args[1]);
        s = open(dir_name, O_RDONLY);
    }
 
    if (argc >= 3) {
        char dir_name[maxn];
        prepare_args(dir_name, args[2]);
        t = open(dir_name, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    }
 
    char ch;
    while (read(s, &ch, 1) > 0) write(t, &ch, 1);
 
    if (s != 0) close(s);
    if (t != 1) close(t);
 
    ret_status = 0;
}
 
void fpipes()
{
    int pipes[maxn][2];
 
    for (int i = 1; i < argc; i++) {
        if (pipe(pipes[i])) perror("pipe");
 
        int x = fork();
 
        if (x < 0) perror("fork"); else
        if (!x) {
            if (i == 1) {
                dup2(pipes[i][1], 1);
 
                close(pipes[i][0]);
                close(pipes[i][1]);
            } else if (i < argc - 1) {
                dup2(pipes[i - 1][0], 0);
                dup2(pipes[i][1], 1);
 
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
                close(pipes[i][0]);
                close(pipes[i][1]);
            } else {
                dup2(pipes[i - 1][0], 0);
 
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
 
            execl(prog_path, "unimportant_name",  args[i], NULL);
            perror("execl");
        } else {
            if (i > 1) close(pipes[i - 1][0]);
            close(pipes[i][1]);
 
            int status;
            waitpid(x, &status, 0);
        }
    }
 
    ret_status = 0;
}
 
int main(int ac, char *ar[])
{
    initialize(ar[0]);
 
    if (ac > 1) {
        strcpy(line, ar[1]);
        reset();
        parse();
        filterArgs();
        prepare_exec();
        return 0;
    }
 
    while (true) {
        if (!fgets(line, sizeof line, stdin)) break;
        if (strlen(line) == 1) continue;
 
        reset();
        parse();
 
        if (args[0][0] == '#') continue;
 
        filterArgs();
        prepare_exec();
    }
 
    return 0;
}
 
void eval()
{
    if (!strcmp(args[0], "name")) fname(); else
    if (!strcmp(args[0], "help")) fhelp(); else
    if (!strcmp(args[0], "status")) fstatus(); else
    if (!strcmp(args[0], "exit")) fexit(); else
    if (!strcmp(args[0], "print")) fprint(); else
    if (!strcmp(args[0], "echo")) fecho(); else
    if (!strcmp(args[0], "pid")) fpid(); else
    if (!strcmp(args[0], "ppid")) fppid(); else
    if (!strcmp(args[0], "dirchange")) fdirchange(); else
    if (!strcmp(args[0], "dirwhere")) fdirwhere(); else
    if (!strcmp(args[0], "dirmake")) fdirmake(); else
    if (!strcmp(args[0], "dirremove")) fdirremove(); else
    if (!strcmp(args[0], "dirlist")) fdirlist(); else
    if (!strcmp(args[0], "linkhard")) flinkhard(); else
    if (!strcmp(args[0], "linksoft")) flinksoft(); else
    if (!strcmp(args[0], "linkread")) flinkread(); else
    if (!strcmp(args[0], "linklist")) flinklist(); else
    if (!strcmp(args[0], "unlink")) funlink(); else
    if (!strcmp(args[0], "rename")) frename(); else
    if (!strcmp(args[0], "cpcat")) fcpcat(); else
    if (!strcmp(args[0], "pipes")) fpipes(); else
    funsupported();
    fflush(stdout);
}
 
void prepare_exec()
{
    fflush(stdout);
 
    prepare_desc();
 
    if (!bg) {
        eval();
        rec_desc();
        fflush(stdout);
        return;
    }
 
    int x = fork();
    if (x < 0) perror("fork"); else
    if (!x) {
        eval();
        exit(ret_status);
    } else {
        int status;
        waitpid(x, &status, 0);
    }
}
 
void prepare_desc()
{
    if (write_to[0]) {
        char path[maxn];
        prepare_args(path, write_to);
 
        copy_write_desc = dup(write_desc);
        int x = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        dup2(x, write_desc);
    }
 
    if (read_from[0]) {
        char path[maxn];
        prepare_args(path, read_from);
 
        copy_read_desc = dup(read_desc);
        int x = open(path, O_RDONLY);
        dup2(x, read_desc);
    }
}
 
void rec_desc()
{
    dup2(copy_read_desc, read_desc); close(copy_read_desc);
    dup2(copy_write_desc, write_desc); close(copy_write_desc);
}