#include <stdio.h>

FILE *fp;
char line[(int) 1e5 + 17];

int main()
{
	fp = fopen("/etc/shadow", "r");

	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) printf("%s", line);
		fclose(fp);
	}

	return 0;
}
