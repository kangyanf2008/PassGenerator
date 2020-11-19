#include <stdio.h>
#include <string.h>
#include "pwKey.h"



int main()
{
	unsigned char *data[1024];
	unsigned int ilen;
	
	int result = getHiddenDataKey(0, 0, data, &ilen);
	printf("%s ===\n", data);
	printf("%d\n", ilen);
	printf("%d\n", result);

	result = getExtDataKey(0, 0, data, &ilen);
	printf("%s", data);
	printf("%d\n", ilen);
	printf("%d\n", result);

	if (result != NULL) {
		free(data);
	}
	
	return 0;
}


