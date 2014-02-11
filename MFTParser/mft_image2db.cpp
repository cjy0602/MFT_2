#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "../sqlite/Debug/sqlite3.h"
#include "define.h"

char seps[] = "|\n";
char *token;

struct MFT {
	char PATH[260];
	int atime, mtime, ctime, crtime;
};
typedef struct MFT test;


int MFTtest(struct MFT  *u3, int countresult)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char *sql;
    int rc;
    int j;
    unsigned int i;
    char *buffer = (char *)malloc(500);
    memset(buffer, 0x00, sizeof(char)*500);

    int error = sqlite3_open("test8.db", &db);
    if(error)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }
    fprintf(stdout, "DB연결 완료.\n");
    if(sqlite3_open("test8.db", &db) != SQLITE_OK)
    {
        fprintf(stderr, "DB접근이 어렵습니다. (오류 %s)\n", sqlite3_errmsg(db));
    }

    //MFT 테이블 생성
    sql = "CREATE TABLE IF NOT EXISTS MFT (FULLPATH TEXT ,ATIME INT,MTIME INT, CTIME INT, CRTIME INT);";
    if( sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK) {
        fprintf(stderr, ">> SQLite Table creation Succeeded!\n");
    } else {
        puts("테이블 생성에 실패했습니다.");
        exit(1);
    }

    //데이터 추가 코드.
    char* errorMsg = NULL;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit begin result : %s\n", errorMsg);
	sprintf (buffer,"INSERT INTO MFT(FULLPATH, ATIME, MTIME, CTIME, CRTIME) VALUES ( ?1, ?2, ?3, ?4, ?5)");

    if(sqlite3_prepare_v2(db, buffer, strlen(buffer), &stmt, NULL) == SQLITE_OK)
    {
        puts(">> Prepared Statement is ready : Succeeded!\n");
    }
    else
    {
        puts("테이블 값 입력에 실패하였습니다.");
    }

    for( i=0; i<countresult; i++){
		 sqlite3_bind_text(stmt, 1, u3[i].PATH, strlen(u3[i].PATH), SQLITE_STATIC);
		sqlite3_bind_int(stmt, 2, (int)(u3[i].atime));
        sqlite3_bind_int(stmt, 3, (int)(u3[i].mtime));
        sqlite3_bind_int(stmt, 4, (int)(u3[i].ctime));
		sqlite3_bind_int(stmt, 5, (int)(u3[i].crtime));

        if ( sqlite3_step(stmt) != SQLITE_DONE )  {
            fprintf(stderr, ">> SQLite Insert failed! \n");
        }
        sqlite3_reset(stmt);
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION;", NULL, NULL, &errorMsg);
    fprintf(stderr, " Commit result : %s\n", errorMsg);
    sqlite3_finalize(stmt);

    sqlite3_close(db);
   
    return 0;
 }
 
int mft_image2db()
{
	FILE *f;
	char buf[256];
	int i,j, count;
	test *u3;

	//u3 = (test *)malloc(sizeof(test) * 1000000);
	u3 = (test *)calloc(1000000, sizeof(test));

	if(u3==NULL){
		puts("Malloc Failed...");
		exit(1);
	}

	f = fopen("image.mft","rt");

	if(f!=NULL){
		for(i=0;f!=NULL;i++){
			if(fgets(buf,256,f)==NULL){
				break;
			}
			token = strtok( buf, seps );
			for(j=0; token != NULL;j++)
			{
				if(j==1){
					strcpy(u3[i].PATH,token);
				}else if(j==7){
					u3[i].atime = atoi(token);
				}else if(j==8){
					u3[i].mtime = atoi(token);
				}else if(j==9){
					u3[i].ctime = atoi(token);
				}else if(j==10){
					u3[i].crtime = atoi(token);
				}
				token = strtok( NULL, seps );
			}
			if(f!=NULL){
				count = i;
			}
		}
		fclose(f);
	}
	MFTtest(u3,count);
	free(u3);

	return 0;
}
