#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int p, energy;
char cout[2] = {'.', '*'};

void increase()
{
	energy += 10;
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
		printf("Child exit with status %d\n", (42 * energy) % 128);
		exit((42 * energy) % 128);
	} else printf("Forked child %d\n", x);
}

void zombiehn()
{
	int status;
	waitpid(-1, &status, 0);

	if (WIFEXITED(status)) printf("Zombie caught with status %d\n", WEXITSTATUS(status));
}

int main(int argc, char *args[])
{
	signal(SIGTERM, increase);
	signal(SIGUSR1, swap_chr);
	signal(SIGUSR2, new_chld);
	signal(SIGCHLD, zombiehn);

	printf("My PID: %d\n", getpid());

	energy = argc == 1 ? 42 : atoi(args[1]);

	while (energy) {
		printf("%c", cout[p]); fflush(stdout);
		energy--;
		sleep(1);
	}

	return 0;
}
