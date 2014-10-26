#include <stdio.h>

#include <ccore/file.h>

int main(int argc, char **argv)
{
	printf("%s\n", ccFileTempDirGet());

	getchar();

	return 0;
}