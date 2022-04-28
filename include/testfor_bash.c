#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "shell.h" 
#include"command.h"
// 将strRes中的t替换为s，替换成功返回1，否则返回0。
int StrReplace(char strRes[],char from[], char to[]) {
    int i,flag = 0;
    char *p,*q,*ts;
    for(i = 0; strRes[i]; ++i) {
        if(strRes[i] == from[0]) {
            p = strRes + i;
            q = from;
            while(*q && (*p++ == *q++));
            if(*q == '\0') {
                ts = (char *)malloc(strlen(strRes) + 1);
                strcpy(ts,p);
                strRes[i] = '\0';
                strcat(strRes,to);
                strcat(strRes,ts);
                free(ts);
                flag = 1;
            }
        }
    }
    return flag;
}
/**
* 读取文件内容
* path:文件路径
* length:文件大小(out)
* return:文件内容
*/
char * ReadFile(char * path, int *length)
{
	FILE * pfile;
	char * data;
 
	pfile = fopen(path, "rb");
	if (pfile == NULL)
	{
		return NULL;
	}
	fseek(pfile, 0, SEEK_END);
	*length = ftell(pfile);
	data = (char *)malloc((*length + 1) * sizeof(char));
	rewind(pfile);
	*length = fread(data, 1, *length, pfile);
	data[*length] = '\0';
	fclose(pfile);
	return data;
} 
int main(int argc,char**argv,char**envp) {
    // char str[80] = "cat///r/n/etc/passwd";
    int num = 0;
    char *str = ReadFile(argv[1],&num);
    /* printf("替换前:%s\n",str); */
    StrReplace(str,"\\\n","");
    StrReplace(str,"$IFS"," ");
    StrReplace(str,"${IFS}"," ");
        /* printf("替换后:%s\n",str); */
    /* else printf("没有任何替换。\n"); */
   detect_bash_language(str,envp);
    printf("123456");
   return 0;
} 
