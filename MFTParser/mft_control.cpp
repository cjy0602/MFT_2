#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "define.h"

int main(int argc, char **argv1)
{
	int select;
	printf(" LIVE Analyze : 1  / DISK Image Analyze : 2 \n");
	scanf("%d", &select);
	getchar();

	switch (select)
	{
	case 1:
		mft_live();
		break;

	case 2:


    // stdout을 파일로 저장.
		if (freopen("image.mft", "w", stdout) == NULL)
			fprintf(stderr, "error redirecting stdout\n");
		mft_image(argc, argv1);
		fclose (stdout);
	    
		//restore stdout
		freopen("CON", "w", stdout);
		//printf("@@@@ 출력테스트 @@@@\n");
	// end

		mft_image2db();
		break;
	}
}

