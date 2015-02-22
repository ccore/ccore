#include <stdio.h>

#include <ccore/file.h>
#include <ccore/string.h>

int main(int argc, char **argv)
{
	ccFileDir directory;

	ccFileDirFindFirst(&directory, ccFileDataDirGet());

	printf("Found %s\n", directory.name);

	while(true) {
		ccFileDirFind(&directory);

		if(directory.name == NULL) break;

		printf("Found %s\n", directory.name);
	}

	getchar();

	return 0;
}