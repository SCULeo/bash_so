#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include"time.h"
#include <pthread.h>
char **g_argv = NULL;
int g_argc = 0;

extern __thread int running_trap;
struct param{
    int id;
    char ** env;
};


void* bash_lint_pthread(void* arg)
{
    char msg[1024];
    size_t len = 0;
    FILE *fp = NULL;
    int begintime,endtime;
    double sum = 0;
    struct param temp = *((struct param*)arg);
    if(temp.id == NULL) {
        printf("Paramter error!\n");
        pthread_exit(NULL);
    }
    fp = fopen((char *)g_argv[temp.id], "r");
    int n = 0;
    while (NULL==feof(fp) && fgets(msg,1024-1,fp) > 0){
        msg[strlen(msg)-1] = '\0';
        printf("input_line:%s\n",msg);
        
        memset(msg,0,1024);
        len = 0;
    }
    printf("the totall:%lf\n,the average time:%lf\n,the number n:%d\n",sum/CLOCKS_PER_SEC,sum/(n*CLOCKS_PER_SEC),n);
    fclose(fp);
   return 0;
}
int main(int argc, char **argv,char**envp)
{
    int ret = -1;
    char* err_str = NULL;
    int err_offset = 0, i;
    pthread_t tid, *tids = NULL;
    int begintime,endtime;
    g_argv = argv;
    g_argc = argc;
    float sum = 0;
//    ppr_daemon();
    // pthread_mutex_init(&gMutex, NULL);
    if (argc == 1) {
        printf("Error! Use as \"bash_detect_demo detected-filepath1 detected-filepath2 ......\"\n");
        return -1;
    }
    begintime = clock();
    tids = malloc((argc - 1) * sizeof(pthread_t));
    if (tids == NULL) {
        printf("thread id array alloc failed!\n");
        return 0;
    }
    memset(tids, 0, (argc - 1) * sizeof(pthread_t));

    for (i = 1; i < argc; i++) {
        int n = i;
        struct param parmment;
        parmment.env=envp;
        parmment.id= i;
        if (pthread_create(&tid, NULL, bash_lint_pthread, &parmment)) {
            printf("Thread create failed! Path:%s\n", argv[i]);
            continue;
        }
        
        printf("Thead %lu created\n", tid);
            tids[i - 1] = tid;
        
#if 0
        pthread_detach(tid);
#endif
    }

    for (i = 1; i < argc; i++) {
        pthread_join(tids[i - 1], NULL);
    }

    ret = 0;
    endtime = clock();
    sum =endtime-begintime;
    printf("\nthe begintime:%d\n",begintime);
    printf("the endtime:%d\n",endtime);
    printf("the sum:%d\n",sum);
    printf("the totall:%lf\n",sum/CLOCKS_PER_SEC);
    return ret;
}