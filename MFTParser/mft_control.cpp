#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "define.h"
#include <tchar.h>
#include <windows.h>

int main(int argc, char **argv1)
{
	int select;
	
	_TCHAR *volName = L"d:";
		  // -v 옵션으로 입력받은 볼륨 이름.  
	_TCHAR *ImagePath = L"C:\\sample.dd";
		 // -I 옵션으로 입력받은 디스크이미지 경로

	   

	printf(" LIVE Analyze : 1  / DISK Image Analyze : 2 \n");
	scanf("%d", &select);
	getchar();

	switch (select)
	{
	case 1: // 라이브

		char* ctmp;
		int len; 
		len = WideCharToMultiByte(CP_ACP, 0, volName, -1, NULL, 0, NULL,NULL);
		ctmp = new char[len];
		WideCharToMultiByte(CP_ACP, 0, volName, -1, ctmp, len, NULL, NULL);
		mft_live(ctmp);
		break;

	case 2: // 디스크

    // MFT image to db 
		char* ctmp2;
		int len2; 
		len2 = WideCharToMultiByte(CP_ACP, 0, ImagePath, -1, NULL, 0, NULL,NULL);
		ctmp2 = new char[len2];
		WideCharToMultiByte(CP_ACP, 0, ImagePath, -1, ctmp2, len2, NULL, NULL);

		// 경로에 case를 -n옵션으로 받은 case명으로 변경하는 코드 추가 필요.
		if (freopen("./case/image.mft", "w", stdout) == NULL)
			fprintf(stderr, "error redirecting stdout\n");
		mft_image(ImagePath);
		fclose (stdout);
		freopen("CON", "w", stdout);
		//printf("image.mft 생성 완료\n");
		mft_image2db();
		if ( remove("./case/image.mft") == -1 )
			perror ( "Could not delete image.mft file\n");

	// end
		break;
	}
}

