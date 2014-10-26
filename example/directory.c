#include <stdio.h>

#include <ccore/file.h>
#include <ccore/string.h>

int main(int argc, char **argv)
{
	ccFileDir directory;
	char *testDir;
	char *buffer;

	testDir = ccStringConcatenate(2, ccFileDataDirGet(), "\\");

	ccFileDirFindFirst(&directory, &buffer, testDir);
	free(testDir);

	printf("Found %s\n", buffer);
	free(buffer);

	while(true) {
		ccFileDirFind(&directory, &buffer);

		if(buffer == NULL) break;

		printf("Found %s\n", buffer);
		free(buffer);
	}

	getchar();

	return 0;
}