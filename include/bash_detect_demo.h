#ifndef BASH_DETECT_DEMO_H
#define BASH_DETECT_DEMO_H
#include <pthread.h>
extern pthread_mutex_t gMutex;

extern pthread_mutex_t cMutex;
extern pthread_mutex_t c2Mutex;
#define BUF_LEN 4096
extern int run_one_command (char *command);
extern void pop_var_context (void);
extern int init_regex(struct replace_word_param *params,const int size);
#endif