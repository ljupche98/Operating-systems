#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/types.h>

#define maxn (1 << 10)

int p, energy;
char cout[2] = {'.', '*'};
char prog_name[maxn];

void printtime()
{
	time_t ctime;
	struct tm *timed;

	time(&ctime);
	timed = localtime(&ctime);
	printf("\n%s", asctime(timed));
}

void increase()
{
	energy += 10;
	printtime();
	printf("Bonus energy (%d)\n", energy);
}

void swap_chr()
{
	p ^= 1;
}

void new_chld()
{
	int x = fork();

	if (x < 0) perror("FORK_ERR");
	else if (!x) {
		sleep((energy % 7) + 1);
		printtime();
		printf("Child exit with status %d\n", (42 * energy) % 128);
		exit((42 * energy) % 128);
	} else printtime(), printf("Forked child %d\n", x);
}

void zombiehn()
{
	int status;
	waitpid(-1, &status, 0);

	if (WIFEXITED(status)) printtime(), printf("Zombie caught with status %d\n", WEXITSTATUS(status));
}

int main(int argc, char *args[])
{
	if (argc <= 2) {
		int x = fork();
		if (x < 0) perror("fork"); else
		if (!x) return execl(args[0], args[0], argc == 1 ? "42" : args[1], "demon", NULL);
		return 0;
	}

	strcpy(prog_name, args[0] + 2);
	strcpy(prog_name + strlen(prog_name), ".log");

	int x = open(prog_name, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
	if (errno) printf("err");

	close(0);
	close(2);
	dup2(x, 1);
	close(x);

	signal(SIGTERM, increase);
	signal(SIGUSR1, swap_chr);
	signal(SIGUSR2, new_chld);
	signal(SIGCHLD, zombiehn);

	printtime();
	printf("My PID: %d\n", getpid());

	energy = argc == 1 ? 42 : atoi(args[1]);

	while (energy) {
		printf("%c", cout[p]); fflush(stdout);
		energy--;
		sleep(1);
	}

	return 0;
}
