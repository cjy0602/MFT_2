#include <stdio.h>
#include <string.h>
#include "define.h"

int main()
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
		char **argv;
		int argc = 2;

		argv[0] = "Filename";
		argv[1] = "";

		scanf("%s", &argv[1]);
		getchar();

		printf("%d, %s, %s\n", argc, argv[0],argv[1]);

		//freopen ("image.mft","w",stdout);

		if (freopen("image.mft", "w", stdout) == NULL)
			fprintf(stderr, "error redirecting stdout\n");
		// stdout을 파일로 저장.
		mft_image(argc, argv);
		fclose (stdout);
		break;
	}
}

