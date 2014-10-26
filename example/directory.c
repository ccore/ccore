#include <stdio.h>

#include <ccore/file.h>

int main(int argc, char **argv)
{
	ccFileDir directory;
	char *buffer;

	ccFileDirFindFirst(&directory, &buffer, "C:\\Users\\Job\\Documents\\OpenTTD\\");
	printf("Found %s\n", buffer);

	while(true) {
		ccFileDirFind(&directory, &buffer);

		if(buffer == NULL) break;

		printf("Found %s\n", buffer);
	}

	getchar();

	return 0;
}