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

#define maxp (1 << 10)
#define maxn (1 << 15)

typedef struct process
{
	int id;
	int par;
	int files;
	long threads;

	char status;
	char name[maxp];
} process;

int proc_count;
process p[maxp];

int c_count, cum[maxp];
int l_count, level[maxp];
int t_count, target[maxp];

char tlc(char x)
{
	if ('A' <= x && x <= 'Z') return x - 'A' + 'a';
	return x;
}

int cmp(char a[], char b[])
{
	int i;
	for (i = 0; a[i] && b[i] && tlc(a[i]) == tlc(b[i]); i++);
	if (!a[i] && !b[i]) return 0;
	if (!b[i]) return 1;
	if (!a[i] || tlc(a[i]) < tlc(b[i])) return -1;
	return 1;
}

int idCmp(const process *a, const process *b)
{
	return a->id - b->id;
}

int nameCmp(const process *a, const process *b)
{
	int c = cmp(a->name, b->name);
	if (c) return c;
	return idCmp(a, b);
}

bool isNumber(char text[])
{
	for (int i = 0; text[i]; i++)
		if (text[i] < '0' || text[i] > '9')
			return false;
	return true;
}

bool reach(int x, int u)
{
	if (x == u) return true;

	for (int i = 0; i < proc_count; i++)
		if (p[i].par == u)
			if (reach(x, p[i].id))
				return true;

	return false;
}

void addProcess(char path[], char pid[])
{
	char cpath[maxp]; strcpy(cpath, path); strcat(cpath, "/"); strcat(cpath, pid);
	char fcpath[maxp]; strcpy(fcpath, cpath); strcat(fcpath, "/stat");
	char dcpath[maxp]; strcpy(dcpath, cpath); strcat(dcpath, "/fd");

	FILE *fp = fopen(fcpath, "r");
	if (fp) {
		fscanf(fp, "%d %s %c %d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %ld", &p[proc_count].id, p[proc_count].name, &p[proc_count].status, &p[proc_count].par, &p[proc_count].threads);
		fclose(fp);

		if (p[proc_count].name[0] == '(') { int i;
			for (i = 0; p[proc_count].name[i + 1] != ')'; i++)
				p[proc_count].name[i] = p[proc_count].name[i + 1];
			p[proc_count].name[i] = 0;
		}
	}

	DIR *md = opendir(dcpath);
	while (readdir(md)) p[proc_count].files++;
	closedir(md); p[proc_count].files -= 2;

	proc_count++;
}

void readProcesses(char path[])
{
	struct dirent *fd;
	DIR *d = opendir(path);

	while (fd = readdir(d))
		if (isNumber(fd->d_name))
			addProcess(path, fd->d_name);

	closedir(d);
}

void preprocess(bool rarg, char arg[])
{
	char text[maxp];

	if (!rarg) {
		while (scanf("%s", text) != EOF)
			target[t_count++] = atoi(text);
	} else {
		for (int i = 0; arg[i]; i++) {
			if (arg[i] < '0' || arg[i] > '9') continue;

			int j;
			for (j = i; arg[j] && '0' <= arg[j] && arg[j] <= '9'; j++);

			int t = arg[j];

			arg[j] = 0;
			target[t_count++] = atoi(arg + i);
			arg[j] = t;
			i = j;

			if (!t) break;
		}
	}

	int x = 0;
	for (int i = 0; i < t_count; i++) x += target[i] - 1;
	assert(x >= 0);
	while (x) assert(!target[t_count]), t_count++, x--;

	int it = 0, last = 1; level[0] = 1; l_count = 1;
	while (it < t_count) {
		int jt = it, newl = 0;
		while (jt < t_count && jt - it < last)
			newl += target[jt++];
		level[l_count++] = last = newl; it = jt;
	}

	c_count = l_count + 1;
	for (int i = 0; i < l_count; i++)
		cum[i + 1] = cum[i] + level[i];
}

int main(int argc, char* args[])
{
	if (!strcmp(args[1], "forktree")) {
		if (argc == 2) {
			preprocess(false, args[0]);

			char *argn[1 << 3];
			for (int i = 0; i < 5; i++) argn[i] = malloc(maxp * sizeof(char)), argn[i][0] = 0;

			strcpy(argn[0], args[0]);
			strcat(argn[1], "forktree");
			strcat(argn[2], "1");
			strcat(argn[3], "0");
			argn[5] = NULL;

			argn[4][0] = 0;
			for (int i = 0; i < t_count; i++) {
				char cn[maxp];
				sprintf(cn, "%d", target[i]);
				if (i) strcat(argn[4], " ");
				strcat(argn[4], cn);
			}

			execv(args[0], argn);
		} else {
			preprocess(true, args[4]);

			int clvl = atoi(args[2]);
			int cidx = atoi(args[3]);
			int tidx = cum[clvl - 1] + cidx;

			if (tidx >= t_count) pause();

			for (int i = 0; i < target[tidx]; i++) {
				int x = fork();

				if (!x) {
					char *argn[1 << 3];
 					for (int j = 0; j < 5; j++) argn[j] = malloc(maxp * sizeof(char)), argn[j][0] = 0;

					int pr = i;
					for (int j = cum[clvl - 1]; j < tidx; j++) pr += target[j];

					strcpy(argn[0], args[0]);
					strcpy(argn[1], args[1]);
					sprintf(argn[2], "%d", clvl + 1);
					sprintf(argn[3], "%d", pr);
					strcpy(argn[4], args[4]);
					argn[5] = NULL;

					if (execv(args[0], argn)) perror("EXECV_ERR");
				} else if (x < 0) printf("UNSUCESSFUL FORK\n"), exit(1);
			}

			usleep(4500 * (t_count - tidx));

			if (tidx) pause();
			else {
				char *argp[1 << 2];
				for (int i = 0; i < 3; i++) argp[i] = malloc(17 * sizeof(char)), argp[i][0] = 0;

				strcat(argp[0], "pstree");
				strcat(argp[1], "-c");
				sprintf(argp[2], "%d", getpid());
				argp[3] = NULL;

				int z = fork();

				if (z == 0) {
					if (!execv("/usr/bin/pstree", argp)) perror("EXECV_ERR");
				} else if (z > 0)
					usleep(4500 * t_count), exit(0);
			}
		}
	}

	if (argc < 3) return 0;

	if (!strcmp(args[1], "pids") || !strcmp(args[1], "names") || !strcmp(args[1], "ps") || !strcmp(args[1], "psext")) {
		readProcesses(args[2]);

		if (strcmp(args[1], "names")) 	 qsort(p, proc_count, sizeof(process), (int (*)(const void *, const void*)) idCmp);
		else				 qsort(p, proc_count, sizeof(process), (int (*)(const void *, const void*)) nameCmp);

		if (!strcmp(args[1], "pids")) {
			for (int i = 0; i < proc_count; i++)
				printf("%d\n", p[i].id);
		} else if (!strcmp(args[1], "names")) {
			for (int i = 0; i < proc_count; i++)
				printf("%d %s\n", p[i].id, p[i].name);
		} else if (!strcmp(args[1], "ps")) {
			printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");
			for (int i = 0; i < proc_count; i++)
				if (argc == 3 || reach(p[i].id, atoi(args[3])))
					printf("%5d %5d %6c %s\n", p[i].id, p[i].par, p[i].status, p[i].name);
		} else if (!strcmp(args[1], "psext")) {
			printf("%5s %5s %6s %6s %6s %s\n", "PID", "PPID", "STANJE", "#NITI", "#DAT.", "IME");
			for (int i = 0; i < proc_count; i++)
				if (argc == 3 || reach(p[i].id, atoi(args[3])))
					printf("%5d %5d %6c %6ld %6d %s\n", p[i].id, p[i].par, p[i].status, p[i].threads, p[i].files, p[i].name);
		}

		fflush(stdout);
	}

	if (!strcmp(args[1], "sys")) {
		char *path = malloc(sizeof(char) * maxn); strcpy(path, args[2]);
		int f = open(strcat(path, "/version"), O_RDONLY);

		char *text = malloc(sizeof(char) * maxn);
		read(f, text, maxn);

		int sc = 0;

		for (int i = 0; sc < 4 && text[i]; i++) {
			sc += text[i] == ' ';

			if (sc == 0) printf("%c", text[i]);
			if (sc == 1) printf(":"), sc++;
			if (sc == 3) printf("%c", text[i]);

			if (sc == 4) {
				sc = 0; int j = i;
				while (text[j] && text[j] != ')') j++;
				j += 3; printf("\n");

				for (; sc < 4 && text[j]; j++) {
					sc += text[j] == ' ';

					if (sc == 0) printf("%c", text[j]);
					if (sc == 1) printf(":"), sc++;
					if (sc == 3) printf("%c", text[j]);
				}

				break;
			}
		}

		printf("\nSwap: ");
		strcpy(path, args[2]);
		close(f); f = open(strcat(path, "/swaps"), O_RDONLY);
		read(f, text, maxn);

		int i = 0;
		while (text[i] != '\n') i++; i++;
		while (text[i] != ' ') printf("%c", text[i++]);

		printf("\nModules: ");
		strcpy(path, args[2]);
		close(f); f = open(strcat(path, "/modules"), O_RDONLY);
		read(f, text, maxn);

		i = 0; int c = 0;
		while (text[i]) c += text[i++] == '\n';
		printf("%d\n", c);
		close(f);
		free(path); free(text);
	}

	return 0;
}