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

    // MFT2db (image 
		if (freopen("image.mft", "w", stdout) == NULL)
			fprintf(stderr, "error redirecting stdout\n");
		mft_image(argc, argv1);
		fclose (stdout);
	    
		//restore stdout
		freopen("CON", "w", stdout);
		mft_image2db();
				//printf("@@@@ 출력테스트 @@@@\n");
	// end
		break;
	}
}

