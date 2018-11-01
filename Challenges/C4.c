#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

DIR* d;
struct dirent *f;

int err;
char wd[1 << 10];

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		if (!getcwd(wd, sizeof(wd))) err = errno, perror("Napaka pri pridobitvi trenutnega imenika"), exit(err);
	} else *wd = argv[1];

	if (!(d = opendir(wd))) err = errno, perror("Napaka pri odpiranju imenika"), exit(err);
	else {
		while (f = readdir(d)) write(1, f->d_name, strlen(f->d_name)), puts(""); /// write(1, "\n", 1);
		closedir(d);
	}
	return 0;
}

/** ------------------------------------------------------ **/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

int err;

int main(int argc, char *argv[])
{
	if (argc <= 1) write(1, "Napacna uporaba programa.\nUporaba: ./mymkdir <imenik>\n", 55);
	else {
		if (mkdir(argv[1], S_IRWXU)) err = errno, perror("Napaka pri ustvarjanju imenika"), exit(err);
		else write(1, "Imenik je bil ustvarjen!\n", 25);
	}
	return 0;
}
