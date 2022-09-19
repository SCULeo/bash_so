#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <malloc.h>
#include "../include/shell.h" 
#include"time.h"
#include"bash_detect_demo.h"
#include "../include/common.h"
#include "../include/general.h"
#include "../include/execute_cmd.h"
#include "../include/parser.h"
#include "token_extract.h"
char **g_argv = NULL;
int g_argc = 0;
pthread_mutex_t gMutex;
pthread_mutex_t cMutex;
pthread_mutex_t c2Mutex;
extern __thread Param assign_param_list;
// extern int parse_and_execute (char *string,const char *from_file,int flags);
extern struct replace_word_param g_command_re_params[];
extern __thread int running_trap;
struct param_env{
    int id;
    char ** env;
};




void print_self_command(COMMAND*command,int num);
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

void print_tab(int num)
{
    int i;
    for (i=0;i<num;i++){
        printf("    ");
    }
}
char* self_flags_type[] ={"NULL","variable substitution","process substitution","brace","command substitution","arithmetic substitution","assignment","single_quote"};
char* command_type[] ={"cm_for","cm_case", "cm_while", "cm_if", "cm_simple", "cm_select",
		    "cm_connection", "cm_function_def", "cm_until", "cm_group",
		    "cm_arith", "cm_cond", "cm_arith_for", "cm_subshell", "cm_coproc"};
char* instruction[] ={"r_output_direction", "r_input_direction", "r_inputa_direction",
  "r_appending_to", "r_reading_until", "r_reading_string",
  "r_duplicating_input", "r_duplicating_output", "r_deblank_reading_until",
  "r_close_this", "r_err_and_out", "r_input_output", "r_output_force",
  "r_duplicating_input_word", "r_duplicating_output_word",
  "r_move_input", "r_move_output", "r_move_input_word", "r_move_output_word",
  "r_append_err_and_out"};
char* connection_type[] ={
"&&","||",">>","<<",
"<&","<<<",">&",";;",
";&","<<-","&>","&>>",
"<>",">|","|&",
};
char *cond_type[]={
    "COND_AND","COND_OR","COND_UNARY","COND_BINARY","COND_TERM","COND_EXPR","COND_EQUAL_EQUAL","COND_EQUAL"
};
void print_redirect(REDIRECT*redirect,int num){
    printf("reirect:{\n");
    print_tab(num);
    printf("redirector:{dest:%d}\n",redirect->redirector.dest);
    print_tab(num);
    printf("rflags:%d\n",redirect->rflags);
    print_tab(num);
    printf("flags:%d\n",redirect->flags);
    print_tab(num);
    printf("instruction:%s\n",instruction[redirect->instruction]);
    print_tab(num);
    if (redirect->instruction==5||redirect->instruction==14||redirect->instruction==18||redirect->instruction==19)
    {
        printf("redirectee:{dest:%d,filename:%s}\n",redirect->redirectee.dest,redirect->redirectee.filename->word);
        
    }else{
        printf("redirectee:{dest:%d}\n",redirect->redirectee.dest);
    }
    
    if (redirect->here_doc_eof)
    {
        print_tab(num);
        printf("here_doc_eof:%s\n",redirect->here_doc_eof);
    }
    print_tab(num);
    printf("}\n");
}
void run_and_print_command(char *word,int num)
{
    COMMAND *local_command = NULL;
     pthread_mutex_lock(&gMutex);
        // parse_and_execute (savestring (msg), "-c", SEVAL_NOHIST|SEVAL_RESETLINE);
        run_one_command(word);
        pthread_mutex_unlock(&gMutex);
        local_command = global_command;
        global_command = NULL;
        if (local_command){
             print_self_command(local_command,num);  
        }
        pthread_mutex_lock(&cMutex);
        

        current_token  = 0;
        parse_and_execute_cleanup (-1);
        
        pthread_mutex_unlock(&cMutex);

        pthread_mutex_lock(&c2Mutex);
        dispose_command(local_command);
        
        pthread_mutex_unlock(&c2Mutex);
}
void print_word_desc(WORD_DESC*name,int num){
   
    printf("{\n");
    print_tab(num+1);
    printf("word:%s\n",name->word);
    if (name->flags&&W_QUOTED \
        &&name->word[0]!='\'' \
        &&(name->word[0]=='\"'||name->word[0]=='`'))
    {
        char * temp_word=(char *)malloc(strlen(name->word)*sizeof(char));
        memset(temp_word,0,strlen(temp_word));
        // for (int i =1;i<strlen(name->word)-1;i++)
        // {
        //     temp_word[i-1] = name->word[i];
        // }
        strncpy(temp_word,&name->word[1],strlen(name->word)-2);

        temp_word[strlen(name->word)-2] = '\0';
        print_tab(num+2);
        run_and_print_command(temp_word,num);
        free(temp_word);
    }
    
    print_tab(num+1);
    printf("self_flags:%s\n",self_flags_type[name->self_flags]);
    print_tab(num);
    printf("flags:%d\n",name->flags);
    print_tab(num);
    printf("}\n");
}
void print_WORD_LIST(WORD_LIST*map_list,int num){
    WORD_LIST* temp = map_list;
    printf("{\n");
    
    while (temp)
    {
        print_tab(num+1);
        printf("word_desc:");
        printf("{\n");
        print_tab(num+2);
        printf("word:%s\n",temp->word->word);
        print_tab(num+2);
        if (temp->word->flags&&W_QUOTED \
        &&temp->word->word[0]!='\'' \
        &&(temp->word->word[0]=='\"'||temp->word->word[0]=='`'))
    {

        char * temp_word=(char *)malloc(strlen(temp->word->word)*sizeof(char));
        memset(temp_word,0,strlen(temp_word));
        // for (int i =1;i<strlen(temp->word->word)-1;i++)
        // {
        //     temp_word[i-1] = temp->word->word[i];
        // }
        strncpy(temp_word,&temp->word->word[1],strlen(temp->word->word)-2);
        temp_word[strlen(temp->word->word)-2] = '\0';
        print_tab(num+2);
        run_and_print_command(temp_word,num+2);
        free(temp_word);
    }
        printf("self_flags:%s\n",self_flags_type[temp->word->self_flags]);
        print_tab(num+2);
        printf("flags:%d\n",temp->word->flags);
        temp = temp->next;
        print_tab(num+1);
        printf("}\n");
    }

    print_tab(num);
    printf("}\n");
}


void print_conmand_for(struct for_com *for_com_content,int num){
    printf("{\n");
    print_tab(num);
    printf("name:");
    if (for_com_content->name)
    {
        print_word_desc(for_com_content->name,num+1);
    }else{
        printf("NULL\n");
    }
    
    printf("map_list:");
    if (for_com_content->map_list)
    {
        print_WORD_LIST(for_com_content->map_list,num+1);
    }else{
        printf("NULL\n");
    }
    
    printf("action:");
    if (for_com_content->action)
    {
        print_self_command(for_com_content->action,num+1);
    }else{
        printf("NULL\n");
    }
    print_tab(num);
    printf("}\n");
}

void print_conmand_simple(struct simple_com * simple,int num){
    printf("{\n");
    print_tab(num+1);
    printf("words:");
    if (simple->words)
    {
        print_WORD_LIST(simple->words,num+1);
    }else{
        printf("NULL\n");
    }
    print_tab(num+1);
    printf("redirects:");
    printf("{\n");
    print_tab(num+1);
    if (simple->redirects)
    {
        print_redirect(simple->redirects,num+2);
    }else{
        printf("NULL\n");
    }
    print_tab(num);
    printf("}\n");
}
void print_conmand_connection(struct connection *connection_content,int num){
    printf("{\n");
    print_tab(num);
    printf("connector:");
    if (connection_content->connector<280)
    {
         printf("%c\n",(char)connection_content->connector);
    }else{
        printf("%s\n",connection_type[connection_content->connector-288]);
    }
    print_tab(num);
    printf("First:");
    if (connection_content->first!=NULL)
    print_self_command(connection_content->first,num+1);
    print_tab(num);
    printf("Second:");
    if (connection_content->second!=NULL)
    print_self_command(connection_content->second,num+1);
    print_tab(num);
    printf("}\n");
}
void pirnt_PATTERN_LIST(PATTERN_LIST *clauses,int num)
{
    printf("{\n");
    print_tab(num);
    print_WORD_LIST(clauses->patterns,num+1);
    print_tab(num+1);
    
    if (clauses->action)
    {
        
        print_self_command(clauses->action,num+1);
    }    
    else
    {
        printf("action:NULL\n");
    }
    print_tab(num);
    printf("}\n");
    print_tab(num);
    if (clauses->next)
    {
        pirnt_PATTERN_LIST(clauses->next,num);
    }
}
void print_conmand_case(struct case_com * case_content,int num)
{
    printf("{\n");
    print_tab(num+1);
    print_word_desc(case_content->word,num+1);
    print_tab(num+1);
    pirnt_PATTERN_LIST(case_content->clauses,num+1);
    print_tab(num+1);
    printf("}\n");
}
void print_conmand_while(struct while_com* while_content,int num)
{
    printf("{\n");
    print_tab(num);
    printf("test:");
    print_self_command(while_content->test,num+1);
    print_tab(num);
    printf("action:");
    print_self_command(while_content->action,num+1);
    
    print_tab(num);
    printf("}\n");
}
void print_conmand_Group(struct group_com *Group,int num)
{
    printf("{\n");
    print_tab(num);
    printf("command:");
    print_self_command(Group->command,num+1);
    print_tab(num);
    printf("}\n");
}
void print_conmand_Arith(struct arith_com *Arith,int num)
{
    printf("{\n");
    print_tab(num);
    printf("expdispose_command:");
    print_WORD_LIST(Arith->exp,num+1);
    print_tab(num);
    printf("}\n");
}
void print_conmand_Cond(struct cond_com *Cond,int num)
{
    printf("{\n");
    print_tab(num);
    printf("type:%s\n",cond_type[Cond->type-1]);
    print_tab(num);
    printf("op:");
    if (Cond->op)
        print_word_desc(Cond->op,num+1);
    else
        printf("NULL\n");
    print_tab(num);
    printf("left:");
    if (Cond->left)
    {
        print_conmand_Cond(Cond->left,num+1);
    }else{
        
        printf("NULL\n");
    }
    print_tab(num);
    printf("right:");
    if (Cond->right)
    {
        print_conmand_Cond(Cond->right,num+1);
    }else{
        printf("NULL\n");
    }
    
    print_tab(num);
    printf("}\n");
}
void print_conmand_ArithFor(struct arith_for_com* ArithFor,int num)
{
    printf("{\n");
    print_tab(num);
    printf("init:");
    print_WORD_LIST(ArithFor->init,num+1);

    printf("test:");
    print_WORD_LIST(ArithFor->test,num+1);

    printf("step:");
    print_WORD_LIST(ArithFor->step,num+1);
    
    printf("action:");
    print_self_command(ArithFor->action,num+1);
    print_tab(num);
    printf("}\n");
}
void print_conmand_Subshell(struct subshell_com*subshell,int num){
    printf("{\n");
    printf("command:");
    print_self_command(subshell->command,num+1);
    print_tab(num);
    printf("}\n");
}
void print_conmand_Coproc(struct coproc_com*coproc,int num){
    printf("{\n");
    printf("name:%s",coproc->name);
    printf("command:");
    print_self_command(coproc->command,num+1);
    print_tab(num);
    printf("}\n");
}

void print_self_command(COMMAND*command,int num){
    
    printf("{\n");
    print_tab(num);
    printf("type:%s\n",command_type[command->type]);
    if (command->redirects){
        print_redirect(command->redirects,num+1);
    }
    switch(command->type){
    case cm_for:
        print_tab(num);
        printf("For:");
        print_conmand_for(command->value.For,num+1);
        break;
    case cm_case:
        print_tab(num);
        printf("Case:");
        print_conmand_case(command->value.Case,num+1);
        break;
    case cm_while:
        print_tab(num);
        printf("While:");
        print_conmand_while(command->value.While,num+1);
        break;
    case cm_simple:
        print_tab(num);
        printf("Simple:");
        print_conmand_simple(command->value.Simple,num+1);
        break;
    case cm_connection:
        print_tab(num);
        printf("Connection:");
        print_conmand_connection(command->value.Connection,num+1);
        break;
    case cm_until:
        print_tab(num);
        printf("Until:");
        print_conmand_while(command->value.While,num+1);
        break;
    case cm_group:
        print_tab(num);
        printf("Group:");
        print_conmand_Group(command->value.Group,num+1);
        break;
    case cm_arith:
        print_tab(num);
        printf("Arith:");
        print_conmand_Arith(command->value.Arith,num+1);    
        break;
    case cm_cond:
        print_tab(num);
        printf("Cond:");
        print_conmand_Cond(command->value.Cond,num+1);
        break;
    case cm_arith_for:
        print_tab(num);
        printf("ArithFor:");
        print_conmand_ArithFor(command->value.ArithFor,num+1);
        break;
    case cm_subshell:
        print_tab(num);
        printf("Subshell:");
        print_conmand_Subshell(command->value.Subshell,num+1);
        break;
    case cm_coproc:
        print_tab(num);
        printf("Coproc:");
        print_conmand_Coproc(command->value.Coproc,num+1);
        break;
    default:
        break;
    }
    print_tab(num);
    printf("}\n");
}
void* bash_lint_pthread(void* arg)
{
    COMMAND *local_command =NULL;
    char msg[1024];
    char buf[4096]={0};
    size_t len = 0;
    FILE *fp = NULL;
    int feature[num_max_feature]={0};
    int used =0;
    int begintime,endtime;
    double sum = 0;
    struct param_env temp = *((struct param_env*)arg);
    if(temp.id == NULL) {
        printf("Paramter error!\n");
        pthread_exit(NULL);
    }
    fp = fopen((char *)g_argv[temp.id], "r");
    int n = 0;
    int errornum  = 0;
    int num  = 0;
    FILE * fpwrite = fopen("data.txt","w");
    char temp_add_word[1024]={0};
    while (NULL==feof(fp) && fgets(msg,4096-1,fp) > 0){
        // msg[strlen(msg)-1] = '\0';
        
        global_command =NULL;
        local_command =NULL;
        num++;
        
        begintime = clock();
        // printf("num:%d\n",num);
        // printf("input_line:%s",msg);
        pthread_mutex_lock(&gMutex);
        // parse_and_execute (savestring (msg), "-c", SEVAL_NOHIST|SEVAL_RESETLINE);
        run_one_command(msg);
        pthread_mutex_unlock(&gMutex);
        if (global_command){
             local_command = global_command;
             global_command = NULL;
                print_self_command(local_command,0);
                memset(buf,0,strlen(buf));
                
             used = token_command(local_command,&buf,used,&feature);
            for (int i =0 ;i<strlen(buf);i++) 
             {
                 if(buf[i]=='\n'||buf[i]=='\r')
                 {
                     buf[i]= ' ';
                 }
             }
             buf[used] = '\0';
            // printf("msg:%s\n",buf);
            // printf("buf:%s\n",buf);
            // fprintf(fpwrite,"msg:%s",msg);
            //  fprintf(fpwrite,"token:%s\n\n",buf);
            //  fprintf(fpwrite,"%s\n",buf);
        }else
        { 
            sprintf(temp_add_word,"%s",msg);
            if (msg[0]=='"')
            {
                sprintf(temp_add_word,"%s%s","\"",temp_add_word);

            }
            if (msg[0]=='\'')
            {
                sprintf(temp_add_word,"%s%s","\'",temp_add_word);

            }
            if (msg[0]=='&'||msg[0]=='|'||msg[0]==';')
            {
                
                sprintf(temp_add_word,"%s%s","front_content",temp_add_word);
                goto run_command;
            }
            if (msg[strlen(msg)-2]=='&'||msg[strlen(msg)-2]=='|'||msg[strlen(msg)-2]=='/')
            {
                sprintf(temp_add_word,"%s%s", temp_add_word, "back_content");
                goto run_command;
            }
            
            
            
        run_command:
            pthread_mutex_lock(&gMutex);
            
            run_one_command(temp_add_word);
            pthread_mutex_unlock(&gMutex);
        if (global_command){
             local_command = global_command;
             global_command = NULL;
             print_self_command(local_command,0);
             used = token_command(local_command,&buf,used,&feature);

            //  printf("input_line:%s",temp_add_word);
            //  printf("buf:%s\n\n",buf);
             for (int i =0 ;i<strlen(buf);i++)
             {
                 if(buf[i]=='\n'||buf[i]=='\r')
                 {
                     buf[i]= ' ';
                 }
             }
             buf[used]='\0';

            //  printf("msg:%s\n",temp_add_word);
            //  printf("buf:%s\n",buf);
             fprintf(fpwrite,"msg:%s",msg);
             fprintf(fpwrite,"token:%s\n\n",buf);
            //  fprintf(fpwrite,"%s\n",buf);
            }else{
                errornum++;
                printf("error msg:%s",msg);
                // printf("temp_add_word:%s\n",temp_add_word);
            }
        }
        
        
        fprintf(fpwrite,"msg:%s",msg);
        fprintf(fpwrite,"token:%s\n\n",buf);

        pthread_mutex_lock(&cMutex);
        current_token  = 0;
        
        parse_and_execute_cleanup (-1);
        
        pthread_mutex_unlock(&cMutex);

        pthread_mutex_lock(&c2Mutex);
        dispose_command(local_command);
       
        pthread_mutex_unlock(&c2Mutex);
        memset(buf,0,strlen(buf));
        clean_paramlist(&assign_param_list);
        memset(&assign_param_list,0,sizeof(assign_param_list));
        used = 0;
        endtime = clock();
        
        
       
        // printf("begintime:%d\n",begintime);
        // printf("endtime:%d\n\n",endtime);
        sum = sum + endtime - begintime;
        // if (global_command){
            
        //      print_self_command(global_command,1);
        //     // printf("the string:%s\n",line);
        // } 

       
        
        
        
        // if (global_command){
        //     print_self_command(global_command,1);
        //     // printf("the string:%s\n",msg);
        // }
        // // dispose_command(global_command);
        // global_command = NULL;
        n++;
        memset(msg,0,1024);
        len = 0;
        
    }
    printf("error number:%d\n",errornum);
    printf("num number:%d\n",num);
    printf("the totall:%lf\n,the average time:%lf\n,the number n:%d\n",sum/CLOCKS_PER_SEC,sum/(n*CLOCKS_PER_SEC),n);
    fclose(fpwrite);

    // struct param temp = *((struct param*)arg);
    // char * line = (char *)g_argv[temp.id];
    // detect_bash_language(line,temp.env);




        // char *line = "sudo find / -xdev -type f -size +100000 -name \"*.log\" -exec gzip -v {} \\; 2>&1 | awk '{print $6}'";
        // global_command =NULL;
        // detect_bash_language(line,envp);     
        // if (global_command){
        //     print_self_command(global_command,1);
        //     printf("the string:%s\n",line);
        // }
    
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

    init_hash_words(0);
    init_regex(g_command_re_params,MAX_REPLACE_WORD_PARAM_NUM);
   
//    ppr_daemon();
    pthread_mutex_init(&gMutex, NULL);
    pthread_mutex_init(&cMutex, NULL);
    pthread_mutex_init(&c2Mutex, NULL);
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
        struct param_env parmment;
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
    sum = sum/CLOCKS_PER_SEC;
    float average = sum/12639;
    printf("\nthe begintime:%d\n",begintime);
    printf("the endtime:%d\n",endtime);
    printf("the sum:%lf\n,the average:%lf",sum,average);
    return ret;
}