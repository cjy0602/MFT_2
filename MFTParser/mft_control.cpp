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
		  // -v �ɼ����� �Է¹��� ���� �̸�.  
	_TCHAR *ImagePath = L"C:\\sample.dd";
		 // -I �ɼ����� �Է¹��� ��ũ�̹��� ���

	   

	printf(" LIVE Analyze : 1  / DISK Image Analyze : 2 \n");
	scanf("%d", &select);
	getchar();

	switch (select)
	{
	case 1: // ���̺�

		char* ctmp;
		int len; 
		len = WideCharToMultiByte(CP_ACP, 0, volName, -1, NULL, 0, NULL,NULL);
		ctmp = new char[len];
		WideCharToMultiByte(CP_ACP, 0, volName, -1, ctmp, len, NULL, NULL);
		mft_live(ctmp);
		break;

	case 2: // ��ũ

    // MFT image to db 

		// ��ο� case�� -n�ɼ����� ���� case������ �����ϴ� �ڵ� �߰� �ʿ�.
		if (freopen("./case/image.mft", "w", stdout) == NULL)
			fprintf(stderr, "error redirecting stdout\n");
		mft_image(ImagePath);
		fclose (stdout);
		freopen("CON", "w", stdout);
		//printf("image.mft ���� �Ϸ�\n");
		mft_image2db();
		if ( remove("./case/image.mft") == -1 )
			perror ( "Could not delete image.mft file\n");

	// end
		break;
	}
}

