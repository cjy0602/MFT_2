#include "../NTFSLib/NTFS.h"
#include <stdio.h>
#include <windows.h>
#include "time.h"
#include <iostream>
#include <Setupapi.h>
#include <Ntddstor.h>
#include <string>
#include <stdlib.h>
#include "../sqlite/Debug/sqlite3.h"
#include "define.h"


#pragma comment( lib, "setupapi.lib" )
int MFTtest(struct mftstruct *u3);
using namespace std;

// 이부분은 MFT부분에서 시간 8개의 값을 이쁘게 우리가 보기 좋게 표현할 수 있게 하는 함수로 만들어 주셨음.
// 현재 8개의 시간 중 6개는 10글자의 숫자로 나옴. 2개는 -1로 나오고. 이 부분은 재영이가 수정해서 다시 합치기로 했엉
// 이론상 정상출력 되면 시간이 이 함수를 통해 이쁘게 나올 것 같음. 밑에 똑같은 함수 주석 되있는 것은 멘토님이 짜주신 원본 아래것은 내가 어떻게 해보려고 수정하던것.
time_t TimeFromSystemTime(const SYSTEMTIME pTime)
{
    struct tm tm;
	time_t t;
	
    memset(&tm, 0, sizeof(tm));

	 tm.tm_hour = pTime.wHour;
    tm.tm_min = pTime.wMinute;
    tm.tm_sec = pTime.wSecond;

	tm.tm_mon = pTime.wMonth - 1;
    tm.tm_mday = pTime.wDay;
    tm.tm_year = pTime.wYear-1900;
    

    return mktime(&tm);
}
/*
time_t TimeFromSystemTime(const SYSTEMTIME pTime)
{
    struct tm tm;
	time_t t;

    memset(&tm, 0, sizeof(tm));

    tm.tm_year = pTime.wYear-1900;
    tm.tm_mon = pTime.wMonth - 1;
    tm.tm_mday = pTime.wDay;

    tm.tm_hour = pTime.wHour;
    tm.tm_min = pTime.wMinute;
    tm.tm_sec = pTime.wSecond;
	
	t = mktime(&tm);

    return t;
}*/

//MFT Struct 구조체
struct mftstruct{
	ULONGLONG entry;
	ULONGLONG ParentRef;
	char FILENAME[100]; 
	SYSTEMTIME SI_writeTm, SI_createTm, SI_accessTm, SI_mftTm;
	SYSTEMTIME FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm;
};
typedef struct mftstruct MFTSTRUCT;

unsigned int entry_count; //for문 돌렸을때 행수 체크하는 변수선언

//여기서부터 MFT함수까지는 MFT 코드 부분
int totalfiles = 0;
int totaldirs = 0;

#define START_ERROR_CHK()           \
    DWORD error = ERROR_SUCCESS;    \
    DWORD failedLine;               \
    string failedApi;

#define CHK( expr, api )            \
    if ( !( expr ) ) {              \
        error = GetLastError( );    \
        failedLine = __LINE__;      \
        failedApi = ( api );        \
        goto Error_Exit;            \
    }

#define END_ERROR_CHK()             \
    error = ERROR_SUCCESS;          \
    Error_Exit:                     \
    if ( ERROR_SUCCESS != error ) { \
        cout << failedApi << " failed at " << failedLine << " : Error Code - " << error << endl;    \
    }

int getPhysicalDrive() {

    HDEVINFO diskClassDevices;
    GUID diskClassDeviceInterfaceGuid = GUID_DEVINTERFACE_DISK;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
    DWORD requiredSize;
    DWORD deviceIndex;

    HANDLE disk = INVALID_HANDLE_VALUE;
    STORAGE_DEVICE_NUMBER diskNumber;
    DWORD bytesReturned;

    START_ERROR_CHK();

    //
    // Get the handle to the device information set for installed
    // disk class devices. Returns only devices that are currently
    // present in the system and have an enabled disk device
    // interface.
    //
    diskClassDevices = SetupDiGetClassDevs( &diskClassDeviceInterfaceGuid,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT |
                                            DIGCF_DEVICEINTERFACE );
    CHK( INVALID_HANDLE_VALUE != diskClassDevices,
         "SetupDiGetClassDevs" );

    ZeroMemory( &deviceInterfaceData, sizeof( SP_DEVICE_INTERFACE_DATA ) );
    deviceInterfaceData.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );
    deviceIndex = 0;

    while ( SetupDiEnumDeviceInterfaces( diskClassDevices,
                                         NULL,
                                         &diskClassDeviceInterfaceGuid,
                                         deviceIndex,
                                         &deviceInterfaceData ) ) {

        ++deviceIndex;

        SetupDiGetDeviceInterfaceDetail( diskClassDevices,
                                         &deviceInterfaceData,
                                         NULL,
                                         0,
                                         &requiredSize,
                                         NULL );
        CHK( ERROR_INSUFFICIENT_BUFFER == GetLastError( ),
             "SetupDiGetDeviceInterfaceDetail - 1" );

        deviceInterfaceDetailData = ( PSP_DEVICE_INTERFACE_DETAIL_DATA ) malloc( requiredSize );
        CHK( NULL != deviceInterfaceDetailData,
             "malloc" );

        ZeroMemory( deviceInterfaceDetailData, requiredSize );
        deviceInterfaceDetailData->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA );

        CHK( SetupDiGetDeviceInterfaceDetail( diskClassDevices,
                                              &deviceInterfaceData,
                                              deviceInterfaceDetailData,
                                              requiredSize,
                                              NULL,
                                              NULL ),
             "SetupDiGetDeviceInterfaceDetail - 2" );

        disk = CreateFile( deviceInterfaceDetailData->DevicePath,
                           GENERIC_READ,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL );
        CHK( INVALID_HANDLE_VALUE != disk,
             "CreateFile" );

        CHK( DeviceIoControl( disk,
                              IOCTL_STORAGE_GET_DEVICE_NUMBER,
                              NULL,
                              0,
                              &diskNumber,
                              sizeof( STORAGE_DEVICE_NUMBER ),
                              &bytesReturned,
                              NULL ),
             "IOCTL_STORAGE_GET_DEVICE_NUMBER" );

        CloseHandle( disk );
        disk = INVALID_HANDLE_VALUE;

        cout << deviceInterfaceDetailData->DevicePath << endl;
        cout << "\\\\?\\PhysicalDrive" << diskNumber.DeviceNumber << endl;
        cout << endl;
    }
    CHK( ERROR_NO_MORE_ITEMS == GetLastError( ),
         "SetupDiEnumDeviceInterfaces" );

    END_ERROR_CHK();

Exit:

    if ( INVALID_HANDLE_VALUE != diskClassDevices ) {
        SetupDiDestroyDeviceInfoList( diskClassDevices );
    }

    if ( INVALID_HANDLE_VALUE != disk ) {
        CloseHandle( disk );
    }

    return error;
}

void pause(void) {
  printf("Press any key to continue . . .");
  getchar();  // 아무 키나 1개 입력 받기
  puts(""); // 줄바꿈
}

void usage()
{
	printf("\n# Invalid parameter\n");
	printf("# Usage: mftparset.exe c:\n");
}

// get volume name 'C', 'D', ...
// *ppath -> "c:\program files\common files"
char getvolume(char **ppath)
{
	char *p = *ppath;
	char volname;

	// skip leading blank and "
	while (*p)
	{
		if (*p == ' ' || *p == '"')
			p++;
		else
			break;
	}
	if (*p == '\0')
		return '\0';
	else
	{
		volname = *p;
		p++;
	}

	// skip blank
	while(*p)
	{
		if (*p == ' ')
			p++;
		else
			break;
	}
	if (*p == '\0')
		return '\0';

	if (*p != ':')
		return '\0';

	// forward to '\' or string end
	while (*p)
	{
		if (*p != '\\')
			p++;
		else
			break;
	}
	// forward to not '\' and not ", or string end
	while (*p)
	{
		if (*p == '\\' || *p == '"')
			p++;
		else
			break;
	}

	*ppath = p;
	return volname;
}

void printfile(const CIndexEntry *ie) // 호출 X
{
	// Hide system metafiles
	if (ie->GetFileReference() < MFT_IDX_USER)
	{
		printf("MFT Metadata : Entry < 15\n");
		return;
	}

	// Ignore DOS alias file names
	if (!ie->IsWin32Name())
		return;

	FILETIME ft;
	char fn[MAX_PATH];
	int fnlen = ie->GetFileName(fn, MAX_PATH);
	if (fnlen > 0)
	{
		ie->GetFileTime(&ft);
		SYSTEMTIME st;
		if (FileTimeToSystemTime(&ft, &st))
		{
			printf("%d-%02d-%02d  %02d:%02d\t%s    ", st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, ie->IsDirectory()?"<DIR>":"     ");

			if (!ie->IsDirectory())
				printf("%I64u\t", ie->GetFileSize());
			else
				printf("\t");

			printf("<%c%c%c>\t%s\n", ie->IsReadOnly()?'R':' ',
				ie->IsHidden()?'H':' ', ie->IsSystem()?'S':' ', fn);
		}

		if (ie->IsDirectory())
			totaldirs ++;
		else
			totalfiles ++;
	}
}

int MFTtest(struct mftstruct  *u3)
{
    //database 파일 생성코드

    sqlite3 *db = NULL; 
    sqlite3_stmt *stmt = NULL; //sqlite3 statement 
    char *sql; //쿼리문 들어가는 것
    int rc; //sqlite3 함수를 사용해서 나온 값을 저장하는 부분인데. 그냥 실행시킬때 받는 변수라 생각하면 될듯.
    unsigned int i; //for 문 돌릴 때 변수
    char *buffer = (char *)malloc(500);  //sql로 해도 되지만 insert부분의 쿼리를 넣기 위한 변수로 새로 만들었음
 
    memset(buffer, 0x00, sizeof(char)*500); //buffer부분의 메모리 초기화

	//여기서부터 테이블 생성 전까지는 db파일 생성부분
    int error = sqlite3_open("test444.db", &db);
    if(error)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB연결 완료.\n");
    if(sqlite3_open("test444.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }

    //MFT 테이블 생성
    sql = "CREATE TABLE IF NOT EXISTS MFT (FILENAME TEXT ,entry INT, ParentRef INT, Sl_writeTm INT, SI_createTm INT, SI_accessTm INT, SI_mftTm INT, FN_writeTm INT, FN_createTm INT, FN_accessTm INT, FN_mftTm INT);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) { //멘토님께서 디버깅 편하시려고 수정해주신 부분. 연결되면 콘솔에 succeeded 출력됨.
        fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
        puts("테이블 생성에 실패했습니다.");
        exit(1);
    }

    //데이터 추가 코드.
    char* errorMsg = NULL; //error 생겼을 때 출력 위한 변수
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg); //insert 속도 개선 위해 사용한 코드. insert문 끝날 때 끝내는 문장도 있음.
    fprintf(stderr, " Commit begin result : %s\n", errorMsg); //디버깅 편하게 하기 위해 콘솔에 출력해주는 부분. 나중에는 지워도 될듯
	sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)");
  //buffer라는 변수에 insert 쿼리문을 넣는 것. 쿼리문에 변수에 맞게 넣고. values에 넣으면됨. 1,2,3,4,5,6~~ 숫자는 아래 sqlite3_bind_text stmt, 숫자부분을 바꿔주면 됨. 숫자다음은 저장되있는 변수값들)
    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK) //디버깅 확인 위한 if문
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("테이블 값 입력에 실패하였습니다.");
    }

    for( i=0; i<entry_count; i++){	//sqlite3에 맞는 데이터 형식으로 맞춰주는 함수. int는 인자 3개 text는 인자 4개. SQLITE_STATIC (TEXT는 이거 사용하면 될듯) 자세한 내용은 한글파일 첨부.
		sqlite3_bind_text(stmt, 1, u3[i].FILENAME, strlen(u3[i].FILENAME), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, (int)(u3[i].entry));
        sqlite3_bind_int(stmt, 3, (int)(u3[i].ParentRef));
        sqlite3_bind_int(stmt, 4, TimeFromSystemTime(u3[i].SI_writeTm));
        sqlite3_bind_int(stmt, 5, (int)TimeFromSystemTime(u3[i].SI_createTm));
        sqlite3_bind_int(stmt, 6, (int)TimeFromSystemTime(u3[i].SI_accessTm));
        sqlite3_bind_int(stmt, 7, (int)TimeFromSystemTime(u3[i].SI_mftTm));
        sqlite3_bind_int(stmt, 8, (int)TimeFromSystemTime(u3[i].FN_writeTm));
        sqlite3_bind_int(stmt, 9, (int)TimeFromSystemTime(u3[i].FN_createTm));
        sqlite3_bind_int(stmt, 10, (int)TimeFromSystemTime(u3[i].FN_accessTm));
        sqlite3_bind_int(stmt, 11, (int)TimeFromSystemTime(u3[i].FN_mftTm)); // entry와 ParentRef는 그냥 int 숫자이기에 저렇게 표시했고 나머지 SYSTEMTIME 들은 이쁘게 표현하기위해 함수를 사용했는데 일단 안나오는 상황

		// 이 if문은 없어도됨. 멘토님께서 직접 디버깅 편히 보시려고 해놓으셨다고 함. 이거질문했었음.ㅋㅋ
        if ( sqlite3_step(stmt) != SQLITE_DONE )  { 
            fprintf(stderr, ">> SQLite Insert failed! \n");
            fprintf(stderr, ">> Values : %s, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", u3[i].FILENAME, u3[i].entry, u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);
        }
        sqlite3_reset(stmt); 
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);//위에 insert문 속도빠르게 하기위한거 끝마치는것.
    fprintf(stderr, " Commit result : %s\n", errorMsg);//디버깅편하기 위해 콘솔에 끝났다고 표시
    sqlite3_finalize(stmt);//sqlite3작동 끝내는 함수.

    sqlite3_close(db);//db 닫음.
    return 0;
 }

int mft_live()
{
	clock_t start, end; // 프로그램 실행 시간 측정 용

	DWORD mydrives = 100;// buffer length
	char lpBuffer[100];// buffer for drive string storage
	
	printf("# The Physical Drives of this Machine : \n");
	//getPhysicalDrive();
	system("wmic diskdrive get Caption, Name");
	printf("\n");

	printf("# The Logical Drives of this Machine : \n");
	system("wmic logicaldisk get Description, Name, FileSystem, SystemName");


		////////////////////////////////////// 분석할 드라이브 입력.

	char Name[200];
	printf("\n# Enter the drive Name : ");
	scanf("%200s", Name);
	char *path = Name;

	char volname;
	volname = getvolume(&path); // Double Pointer !

	if (!volname)
	{
		usage();
		return -1;
	}

	CNTFSVolume volume(volname);
	if (!volume.IsVolumeOK())
	{
		printf("Cannot get NTFS BPB from boot sector of volume %c\n", volname);
		return -1;
	}
	
	
	entry_count = volume.GetRecordsCount();	// ULONGLONG GetRecordsCount() const
	printf("MFT Records Count = %d\n\n", entry_count);

	// MFT 정보가 저장되는 구조체 배열 동적할당.
	MFTSTRUCT *u3; 
	u3 = (MFTSTRUCT *)malloc(sizeof(MFTSTRUCT) * entry_count);
	if(u3 ==NULL){
		puts("Malloc Failed...");
		exit(1);
	}

	ULONGLONG  mft_addr;
	mft_addr = volume.GetMFTAddr();
	// printf("$MFT metafile start Adderss = %p\n\n\n", mft_addr);
	// Relative start address of the $MFT metafile. Get from BPB.
	
	//////////////////////////////////////////////////////////////////////////////
	// 분석할 드라이브 선택 후 볼륨 정보까지 가지고왔음.


	// get root directory info
	//.(Root Directory)는 볼륨의 루트디렉터리를 의미하는 것으로 
	// 디렉터리 구조의 파일을 빠르게 접근하기 위해 INDEX 구조로 저장된다.
	
	CFileRecord fr(&volume);

	// we only need INDEX_ROOT and INDEX_ALLOCATION
	// don't waste time and ram to parse unwanted attributes
	//fr.SetAttrMask( MASK_INDEX_ROOT | MASK_INDEX_ALLOCATION);

	fr.SetAttrMask(MASK_STANDARD_INFORMATION | MASK_FILE_NAME);

	//------------------------------------------------------------------------
	if (!fr.ParseFileRecord(MFT_IDX_ROOT))	// 볼륨의 루트 디렉토리 
	{
		printf("Cannot read root directory of volume %c\n", volname);
		return -1;
	}

	if (!fr.ParseAttrs())
	{
		printf("Cannot parse attributes\n");
		return -1;
	}
	//---------------------------------------------- MFT 루트 디렉토리 파싱

	CIndexEntry ie;
	
	// 파일의 MAC타임 정보 저장.
	FILETIME SI_writeTm;
	FILETIME SI_createTm;
	FILETIME SI_accessTm;
	FILETIME SI_mftTm;

	FILETIME FN_writeTm;
	FILETIME FN_createTm;
	FILETIME FN_accessTm;
	FILETIME FN_mftTm;

	_TCHAR fn[MAX_PATH];
	
	start = clock(); // 프로그램 실행 시간 측정 (전체 MFT Entry 디코드 시간)

	//for(int i=30 ;i<40 ; i++)
	for(int i=0 ;i<entry_count ; i++)
	{
		fr.ParseFileRecord(i);

		if (!fr.ParseAttrs())	// <--- 해당 파일 엔트리 파싱.
		{
			//printf("Entry NUM %d Cannot parse attributes\n", i);
			continue;
		}

		int fnlen = fr.GetFileName(fn, MAX_PATH);
		if (fnlen > 0)
		{
			fr.GetFileTime(&SI_writeTm, &SI_createTm, &SI_accessTm, &SI_mftTm, &FN_writeTm, &FN_createTm, &FN_accessTm, &FN_mftTm);

			SYSTEMTIME SI_writeTm_s;
			SYSTEMTIME SI_createTm_s;
			SYSTEMTIME SI_accessTm_s;
			SYSTEMTIME SI_mftTm_s;

			SYSTEMTIME FN_writeTm_s;
			SYSTEMTIME FN_createTm_s;
			SYSTEMTIME FN_accessTm_s;
			SYSTEMTIME FN_mftTm_s;
			
			FileTimeToSystemTime(&SI_writeTm, &SI_writeTm_s); 
			FileTimeToSystemTime(&SI_createTm, &SI_createTm_s);
			FileTimeToSystemTime(&SI_accessTm, &SI_accessTm_s);
			FileTimeToSystemTime(&SI_mftTm, &SI_mftTm_s);

			FileTimeToSystemTime(&FN_writeTm, &FN_writeTm_s); 
			FileTimeToSystemTime(&FN_createTm, &FN_createTm_s);
			FileTimeToSystemTime(&FN_accessTm, &FN_accessTm_s);
			FileTimeToSystemTime(&FN_mftTm, &FN_mftTm_s);


			if (fr.IsDirectory())
				totaldirs ++;
			else
				totalfiles ++;

			// 구조체 배열에 값 저장.
			strcpy(u3[i].FILENAME, (char *)fn);
			
			u3[i].SI_writeTm = SI_writeTm_s;
		    u3[i].SI_createTm = SI_createTm_s;
			u3[i].SI_accessTm = SI_accessTm_s;
			u3[i].SI_mftTm = SI_mftTm_s;
			
			u3[i].FN_writeTm = FN_writeTm_s;
		    u3[i].FN_createTm = FN_createTm_s;
			u3[i].FN_accessTm = FN_accessTm_s;
			u3[i].FN_mftTm = FN_mftTm_s;

			u3[i].entry = i;
			u3[i].ParentRef = fr.GetParentRef();

			


			if (0) // 화면에 출력 하냐 안하냐 설정   0 = 출력안함 / 1 = 출력함
			{
				printf("************************************************************\n\n");
				printf("Current MFT Entry NUM : %u\n", i);
				printf("MFT Parent Reference : %u\n", fr.GetParentRef());

				printf("SI_WRITE TIME : %d-%02d-%02d  %02d:%02d\t\n", SI_writeTm_s.wYear, SI_writeTm_s.wMonth, SI_writeTm_s.wDay,
									SI_writeTm_s.wHour, SI_writeTm_s.wMinute);
				printf("SI_CREATE TIME : %d-%02d-%02d  %02d:%02d\t\n", SI_createTm_s.wYear, SI_createTm_s.wMonth, SI_createTm_s.wDay,
									SI_createTm_s.wHour, SI_createTm_s.wMinute);
				printf("SI_ACCESS TIME : %d-%02d-%02d  %02d:%02d\t\n", SI_accessTm_s.wYear, SI_accessTm_s.wMonth, SI_accessTm_s.wDay,
									SI_accessTm_s.wHour, SI_accessTm_s.wMinute);
				printf("SI_MFT TIME : %d-%02d-%02d  %02d:%02d\t\n", SI_mftTm_s.wYear, SI_mftTm_s.wMonth, SI_mftTm_s.wDay,
									SI_mftTm_s.wHour, SI_mftTm_s.wMinute);

				printf("FN_WRITE TIME : %d-%02d-%02d  %02d:%02d\t\n", FN_writeTm_s.wYear, FN_writeTm_s.wMonth, FN_writeTm_s.wDay,
									FN_writeTm_s.wHour, FN_writeTm_s.wMinute);
				printf("FN_CREATE TIME : %d-%02d-%02d  %02d:%02d\t\n", FN_createTm_s.wYear, FN_createTm_s.wMonth, FN_createTm_s.wDay,
									FN_createTm_s.wHour, FN_createTm_s.wMinute);
				printf("FN_ACCESS TIME : %d-%02d-%02d  %02d:%02d\t\n", FN_accessTm_s.wYear, FN_accessTm_s.wMonth, FN_accessTm_s.wDay,
									FN_accessTm_s.wHour, FN_accessTm_s.wMinute);
				printf("FN_MFT TIME : %d-%02d-%02d  %02d:%02d\t\n", FN_mftTm_s.wYear, FN_mftTm_s.wMonth, FN_mftTm_s.wDay,
									FN_mftTm_s.wHour, FN_mftTm_s.wMinute);


				printf("File TYPE : \t%s", fr.IsDirectory()?"<DIR>\n":"<FILE>\n");

			   if (!fr.IsDirectory())
					printf("filesize = %I64u\n", fr.GetFileSize());
			   else
					printf("\t");

			   printf("<%c%c%c%c>\t%s\n", fr.IsReadOnly()?'R':' ',
				   fr.IsHidden()?'H':' ', fr.IsSystem()?'S':' ',fr.IsDeleted()?'D':' ',fn);

			  //printf("filename = %s\n", fn);
			}
		}
		
	}

	end = clock();
	printf("\n##### 전체 소요시간 : %5.2f초 #####\n", (float)(end-start)/CLOCKS_PER_SEC);

	/*
	for(int i=0 ;i<entry_count ; i++)
	{
		printf(" [i] entry values\n ");
		printf(" FILENAME = %s\n", u3[i].FILENAME);
		printf(" W_TIME = %u\n", u3[i].W_TIME);
		printf(" A_TIME = %u\n", u3[i].A_TIME);
		printf(" C_TIME = %u\n", u3[i].C_TIME);
		printf(" Entry Num = %u\n", u3[i].entry);
		printf(" ParentReg Num = %u\n\n", u3[i].ParentRef);
	}
	*/

	// list it !
	// fr.TraverseSubEntries(printfile);  // CallBack 함수 등록 !!!!!
	printf("Files: %d, Directories: %d\n", totalfiles, totaldirs);
	
	MFTtest(u3);

	free(u3);

	return 0;
}