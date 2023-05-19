#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

int query_proc_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names)  
{  
    int i;  
    int* nof_records = (int*) p_data;  
    (*nof_records)++;  
  
    if (*nof_records == 1)  
    {  
        for (i=0; i < num_fields; i++)  
        {  
            printf("%10s", p_col_names[i]);  
        }  
        printf("\n");  
        for (i=0; i< num_fields*10; i++)  
        {  
            printf("=");  
        }  
        printf("\n");  
    }  
  
    for(i=0; i < num_fields; i++)  
    {   
        if (p_fields[i])  
        {  
            printf("%10s", p_fields[i]);  
        }  
        else  
        {  
            printf("%10s", " ");  
        }  
    }  
  
    printf("\n");  
    return 0;  
}  

int query_proc(sqlite3 *pDB, const char *sql_cmd)
{
    char *errMsg;  
    int   ret;  
    int   nrecs = 0;  
  
    ret = sqlite3_exec(pDB, sql_cmd, query_proc_callback, &nrecs, &errMsg);  
  
    if(ret!=SQLITE_OK)  
    {   
        printf("Error in select statement %s [%s].\n", sql_cmd, errMsg);
        sqlite3_free(errMsg);
        return -1;
    }
    
    printf("\n   %d records returned.\n", nrecs);  
    return 0;
}

int main(void)
{
    sqlite3 *pDB = NULL;
    char * errMsg = NULL;
    char sql_cmd[1000];
    char *insert_cmd[] = {
        "insert into film values ('Silence of the Lambs, The', 118, 1991, 'Jodie Foster')",
        "insert into film values ('Contact', 153, 1997, 'Jodie Foster')",
        "insert into film values ('Crouching Tiger, Hidden Dragon', 120, 2000, 'Yun-Fat Chow')",
        "insert into film values ('Hours, The', 114, 2002, 'Nicole Kidman')",
        NULL
    };
    char **p;

    int ret = sqlite3_open("test.db", &pDB);
    if (ret)
    {
        fprintf(stderr, "Open the databse failed:%d!\n", ret);
        return -1;
    }

    fprintf(stderr, "create the database successful!\n"); 
    strcpy(sql_cmd, "create table film(title, length, year, starring)");
    ret = sqlite3_exec(pDB, sql_cmd, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
    {
        fprintf(stderr, "create table failed.%s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(pDB);
        return -1;
    }

    p = insert_cmd;
    while (*p != NULL)
    {
        ret = sqlite3_exec(pDB, *p, NULL, NULL, &errMsg);
        if (ret != SQLITE_OK)
        {
            fprintf(stderr, "insert table failed.%s\n", errMsg);
            sqlite3_free(errMsg);
            sqlite3_close(pDB);
            return -1;
        }

        p++;
    }

    strcpy(sql_cmd, "select * from film");
    query_proc(pDB, sql_cmd);
    sqlite3_close(pDB);
    return 0;
}

