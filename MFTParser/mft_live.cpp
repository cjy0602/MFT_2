#include "../NTFSLib/NTFS.h"
#include <stdio.h>
#include <windows.h>
#include "time.h"
#include <iostream>
#include <Setupapi.h>
#include <Ntddstor.h>
#include <iostream>
#include <string>
#include "../sqlite/Debug/sqlite3.h"
#include "define.h"
//#include <string.h>

#pragma comment( lib, "setupapi.lib" )

using namespace std;

void FileTimeToUnixTime(LPFILETIME pft, unsigned __int64 * pt) {
    LONGLONG ll; // 64 bit value
    ll = (((LONGLONG)(pft->dwHighDateTime)) << 32) + pft->dwLowDateTime;
    *pt = (time_t)((ll - 116444736000000000L) / 10000000L);
}

/*
unsigned int filetime_to_microseconds(const FILETIME& ft)
{
    // FILETIME 은 1601-Jan-01 기준으로 10,000,000 분의 1초(100 나노초) 로 기록됨
    union
    {
        FILETIME asFileTime ;
        unsigned int asInt64 ;
    } myFileTime;
 
    myFileTime.asFileTime = ft;
    myFileTime.asInt64 -= 116444736000000000ULL; // 1970-Jan-01 기준 으로 변환
    return (myFileTime.asInt64 / 10); // microseconds 로 변환
}
*/

struct mftstruct{
   ULONGLONG entry;
   ULONGLONG ParentRef;
   char FILENAME[MAX_PATH]; 
   char FULLPATH[MAX_PATH];
   unsigned __int64 SI_writeTm, SI_createTm, SI_accessTm, SI_mftTm;
   unsigned __int64 FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm;
};
typedef struct mftstruct MFTSTRUCT;

unsigned int entry_count;

int totalfiles = 0;
int totaldirs = 0;

char fullPath_[MAX_PATH];
char root[10] = "$ROOT";
char BackSlash[MAX_PATH] = "\\";


char *getFullPath(int entry, MFTSTRUCT *mftStruct, int saved_entry)
{
   //static char buff[MAX_PATH] = "";

   if ( entry == 5 )
   {   
      return root;
   }

   if ( entry != 5 )
   {
      //sprintf(mftStruct[saved_entry].FULLPATH, "\\%s", getFullPath(mftStruct[entry].ParentRef, mftStruct, saved_entry));

      strcat(mftStruct[saved_entry].FULLPATH,  getFullPath(mftStruct[entry].ParentRef, mftStruct, saved_entry));
      char BackSlash[MAX_PATH] = "\/";
      strcat(BackSlash, mftStruct[entry].FILENAME);
      //return mftStruct[entry].FILENAME;
      return BackSlash;
   }
}

void printStruct(MFTSTRUCT *mftStruct)
{
   // for(int i=303 ;i<404 ; i++)
   for(int i=0 ;i<entry_count ; i++)
   {
      //printf(" [i] entry values\n ");
      printf(" FILENAME = %s\n", mftStruct[i].FILENAME);
      //printf(" W_TIME = %u\n", mftStruct[i].FN_accessTm);
      //printf(" A_TIME = %u\n", mftStruct[i].FN_createTm);
      //printf(" C_TIME = %u\n", mftStruct[i].FN_mftTm);
      //printf(" Entry Num = %u\n", mftStruct[i].entry);
      //printf(" ParentReg Num = %u\n\n", mftStruct[i].ParentRef);
   }
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
    int error = sqlite3_open("test159.db", &db);
    if(error)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB연결 완료.\n");
    if(sqlite3_open("test159.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }

    //MFT 테이블 생성
    sql = "CREATE TABLE IF NOT EXISTS MFT (FILENAME TEXT, FULLPATH TEXT, entry INT, ParentRef INT, Sl_writeTm INT, SI_createTm INT, SI_accessTm INT, SI_mftTm INT, FN_writeTm INT, FN_createTm INT, FN_accessTm INT, FN_mftTm INT);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) { //멘토님께서 디버깅 편하시려고 수정해주신 부분. 연결되면 콘솔에 succeeded 출력됨.
        fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
        puts("테이블 생성에 실패했습니다.");
        exit(1);
    }
    //if(sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) //SQL 쿼리문을 넣는 코드. 그래서 테이블이 생성됨. 쿼리문은 sql 변수에 저장
    //{
    //    if ( sqlite3_step(stmt) != SQLITE_DONE )  {
    //        fprintf(stderr, ">> SQLite Table creation failed!\n");
    //        exit(1);
    //    }
    //}
    //else
    //{
    //    puts("테이블 생성에 실패했습니다.");
    //}
    //sqlite3_finalize(stmt);

    //데이터 추가 코드.
    char* errorMsg = NULL;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit begin result : %s\n", errorMsg);
   sprintf (buffer,"INSERT INTO MFT(FILENAME, FULLPATH, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12)");
    //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( ?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11)", , , , u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i]., u3[i].);

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK)
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("테이블 값 입력에 실패하였습니다.");
    }

    for( i=0; i<entry_count; i++){

        //sprintf (buffer,"INSERT INTO MFT(FILENAME, entry, ParentRef, Sl_writeTm, SI_createTm, SI_accessTm, SI_mftTm, FN_writeTm, FN_createTm, FN_accessTm, FN_mftTm) VALUES ( \"%s\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\", \"%d\")", u3[i].FILENAME, (int)u3[i].entry, (int)u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);

      
      sqlite3_bind_text(stmt, 1, u3[i].FILENAME, strlen(u3[i].FILENAME), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, u3[i].FULLPATH, strlen(u3[i].FULLPATH), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, (int)(u3[i].entry));
        sqlite3_bind_int(stmt, 4, (int)(u3[i].ParentRef));
        sqlite3_bind_int(stmt, 5, (int)(u3[i].SI_writeTm));
        sqlite3_bind_int(stmt, 6, (int)(u3[i].SI_createTm));
        sqlite3_bind_int(stmt, 7, (int)(u3[i].SI_accessTm));
        sqlite3_bind_int(stmt, 8, (int)(u3[i].SI_mftTm));
        sqlite3_bind_int(stmt, 9, (int)(u3[i].FN_writeTm));
        sqlite3_bind_int(stmt, 10, (int)(u3[i].FN_createTm));
        sqlite3_bind_int(stmt, 11, (int)(u3[i].FN_accessTm));
        sqlite3_bind_int(stmt, 12, (int)(u3[i].FN_mftTm));


        if ( sqlite3_step(stmt) != SQLITE_DONE )  {
            fprintf(stderr, ">> SQLite Insert failed! \n");
            fprintf(stderr, ">> Values : %s, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", u3[i].FILENAME, u3[i].entry, u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);
            //exit(1);
        }

        //sqlite3_finalize(stmt);
        sqlite3_reset(stmt);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    for(i=0; i<165000;i++){
        //printf("%s, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n", u3[i].FILENAME, u3[i].entry, u3[i].ParentRef, u3[i].SI_writeTm, u3[i].SI_createTm, u3[i].SI_accessTm, u3[i].SI_mftTm, u3[i].FN_writeTm, u3[i].FN_createTm, u3[i].FN_accessTm, u3[i].FN_mftTm);
    }
    return 0;
 }

int mft_live()
{
   clock_t start, end; // 프로그램 실행 시간 측정 용

   
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


   entry_count = volume.GetRecordsCount();   // ULONGLONG GetRecordsCount() const
   printf("MFT Records Count = %d\n\n", entry_count);

   // MFT 정보가 저장되는 구조체 배열 동적할당.
   MFTSTRUCT *u3; 
   //u3 = (MFTSTRUCT *)malloc(sizeof(MFTSTRUCT) * entry_count);
   u3 = (MFTSTRUCT *)calloc(entry_count, sizeof(MFTSTRUCT));
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
   if (!fr.ParseFileRecord(MFT_IDX_ROOT))   // 볼륨의 루트 디렉토리 
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


   //char fn[MAX_PATH];
   _TCHAR fn[MAX_PATH];
   
   start = clock(); // 프로그램 실행 시간 측정 (전체 MFT Entry 디코드 시간)

   //for(int i=303 ;i<404 ; i++)
   for(int i=0 ;i<entry_count ; i++)
   {
      fr.ParseFileRecord(i);

      if (!fr.ParseAttrs())   // <--- 해당 파일 엔트리 파싱.
      {
         //printf("Entry NUM %d Cannot parse attributes\n", i);
         continue;
      }

      int fnlen = fr.GetFileName(fn, MAX_PATH);
      if (fnlen > 0)
      {
         fr.GetFileTime(&SI_writeTm, &SI_createTm, &SI_accessTm, &SI_mftTm, &FN_writeTm, &FN_createTm, &FN_accessTm, &FN_mftTm);

         unsigned __int64 SI_writeTm_s;
         unsigned __int64 SI_createTm_s;
         unsigned __int64 SI_accessTm_s;
         unsigned __int64 SI_mftTm_s;

         unsigned __int64 FN_writeTm_s;
         unsigned __int64 FN_createTm_s;
         unsigned __int64 FN_accessTm_s;
         unsigned __int64 FN_mftTm_s;

         /*
         FileTimeToSystemTime(&SI_writeTm, &SI_writeTm_s); 
         FileTimeToSystemTime(&SI_createTm, &SI_createTm_s);
         FileTimeToSystemTime(&SI_accessTm, &SI_accessTm_s);
         FileTimeToSystemTime(&SI_mftTm, &SI_mftTm_s);

         FileTimeToSystemTime(&FN_writeTm, &FN_writeTm_s); 
         FileTimeToSystemTime(&FN_createTm, &FN_createTm_s);
         FileTimeToSystemTime(&FN_accessTm, &FN_accessTm_s);
         FileTimeToSystemTime(&FN_mftTm, &FN_mftTm_s);
         */

         FileTimeToUnixTime(&SI_writeTm, &SI_writeTm_s);
         FileTimeToUnixTime(&SI_createTm, &SI_createTm_s);
         FileTimeToUnixTime(&SI_accessTm, &SI_accessTm_s);
         FileTimeToUnixTime(&SI_mftTm, &SI_mftTm_s);

         FileTimeToUnixTime(&FN_writeTm, &FN_writeTm_s);
         FileTimeToUnixTime(&FN_createTm, &FN_createTm_s);
         FileTimeToUnixTime(&FN_accessTm, &FN_accessTm_s);
         FileTimeToUnixTime(&FN_mftTm, &FN_mftTm_s);

         if (fr.IsDirectory())
            totaldirs ++;
         else
            totalfiles ++;

	// K
		 int len; 
		 len = WideCharToMultiByte(CP_ACP, 0, fn, -1, NULL, 0, NULL,NULL);
		 char* ctmp = new char[len];
		 // 실제 변환
		 WideCharToMultiByte(CP_ACP, 0, fn, -1, ctmp, len, NULL, NULL);


         strcpy(u3[i].FILENAME, ctmp);

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

            printf("SI_WRITE TIME : %d\n", SI_writeTm_s);
            printf("SI_CREATE TIME : %d\n", SI_createTm_s);
            printf("SI_ACCESS TIME : %d\n", SI_accessTm_s);
            printf("SI_MFT TIME : %d\n", SI_mftTm_s);

            printf("FN_WRITE TIME : %d\n", FN_writeTm_s);
            printf("FN_CREATE TIME : %d\n", FN_createTm_s);
            printf("FN_ACCESS TIME : %d\n", FN_accessTm_s);
            printf("FN_MFT TIME : %d\n", FN_mftTm_s);

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

   // 구조체에 저장된 정보 출력 함수.
   //printStruct(u3);

   // 각 entry의 전체경로를 구한다.
   //for(int k=303 ;k<404 ; k++)
   for (int k = 0 ; k < entry_count ; k++)
   {
      getFullPath(k, u3, k);
	  strcat(u3[k].FULLPATH, "\/");
	  strcat(u3[k].FULLPATH, u3[k].FILENAME);
      //printf( "entry %d = %s\n\n", k, u3[k].FULLPATH ); 
   }


   end = clock();
   printf("\n##### 전체 소요시간 : %5.2f초 #####\n", (float)(end-start)/CLOCKS_PER_SEC);

   printf("Files: %d, Directories: %d\n", totalfiles, totaldirs);
   MFTtest(u3);

   free(u3);
   return 0;
}