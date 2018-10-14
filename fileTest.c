/*Testing file IO in C
Ross van der Heyde
13 October 2018*/

#include <stdio.h>
#include <stdlib.h>

int* parseNumbers(char* numbers) {
	printf("parseNumbers\n");



	return vec;
}

int main(int argc, char *argv[]) {
	printf("This is test\n");

	printf("argc: %d\n", argc);

	char* flag = argv[1];
	printf("%s\n", flag);
	char* fileName =  argv[2];
	printf("%s\n", fileName);


	if (strcmp(flag, "-i") == 0) {
		printf("yes\n");
	} else {
		printf("no\n");
	}

	//try to read file
	FILE* inputFile = fopen(fileName, "r");

	char* dcdFileName = NULL;
	size_t dcdFileNameLength = 0;
	size_t temp;
	size_t read = getline(&dcdFileName, &dcdFileNameLength, inputFile);
	printf("dcdFileName: %s\n", dcdFileName);

	char* kLine = NULL;
	getline(&kLine, &temp, inputFile);
	int k = atoi(kLine);
	printf("k: %d\n", k);

	char* aLine = NULL;
	size_t aLen = 0;
	getline(&aLine, &aLen, inputFile);
	printf("atoms from a: %s\n", aLine);

	char* bLine = NULL;
	size_t bLen = 0;
	getline(&bLine, &bLen, inputFile);
	printf("atoms from b: %s\n", bLine);

	fclose(inputFile);

	int aIndx = parseNumbers(aLine);

	printf("end\n");
	return 0;
}