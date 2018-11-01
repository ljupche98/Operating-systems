#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

char ch;
int s, t, m, err;

int main(int argc, char *args[])
{
	if (argc > 2 && strlen(args[2]) > 1) t = open(args[2], O_WRONLY); else t = STDOUT_FILENO;
	if (errno) err = errno, perror("Napaka pri odpiranju izhodne datoteke"), exit(err);
	if (t != 1 && flock(t, LOCK_EX)) err = errno, perror("Napaka pri zaklenenju izhodne datoteke"), exit(err);

	if (argc > 1 && strlen(args[1]) > 1) s = open(args[1], O_RDONLY); else s = STDIN_FILENO;
	if (errno) err = errno, perror("Napaka pri odpiranju vhodne datoteke"), exit(err);
	if (s != 0 && flock(s, LOCK_SH)) err = errno, perror("Napaka pri zaklenenju vhodne datoteke"), exit(err);

	while (read(s, &ch, 1) > 0) write(t, &ch, 1);

	flock(t, LOCK_UN); flock(s, LOCK_UN);
	close(s); close(t);
	return 0;
}
