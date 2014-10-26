#include <stdio.h>

#include <ccore/file.h>

int main(int argc, char **argv)
{
	ccFileDir directory = ccFileDirOpen(ccFileDataDirGet());

	printf("Listing all files in %s\n", ccFileDataDirGet());

	getchar();

	return 0;
}