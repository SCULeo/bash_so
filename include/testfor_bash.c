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
void print_tab(int num)
{
    int i;
    for (i=0;i<num;i++){
        printf("    ");
    }
}
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
    "COND_AND","COND_OR","COND_UNARY","COND_BINARY","COND_TERM","COND_EXPR",
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
    printf("redirectee:{dest:%d,filename:%s}\n",redirect->redirectee.dest,redirect->redirectee.filename);
    if (redirect->here_doc_eof)
    {
        print_tab(num);
        printf("here_doc_eof:%s\n",redirect->here_doc_eof);
    }
    print_tab(num);
    printf("}\n");
}
void print_word_desc(WORD_DESC*name,int num){
   
    printf("{\n");
    print_tab(num+1);
    printf("word:%s\n",name->word);
    print_tab(num+1);
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
    if (simple->redirects)
    {
        print_redirect(simple->redirects,num+1);
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
    print_self_command(connection_content->first,num+1);
    print_tab(num);
    printf("Second:");
    print_self_command(connection_content->second,num+1);
    print_tab(num);
    printf("}\n");
}
void pirnt_PATTERN_LIST(PATTERN_LIST *clauses,int num)
{
    printf("{\n");
    print_tab(num);
    print_WORD_LIST(clauses->patterns,num+1);
    print_tab(num);
    print_self_command(clauses->action,num+1);
    print_tab(num);
    printf("}\n");
    if (clauses->next)
    {
        pirnt_PATTERN_LIST(clauses->next,num);
    }
}
void print_conmand_case(struct case_com * case_content,int num)
{
    printf("{\n");
    print_tab(num);
    print_word_desc(case_content->word,num+1);
    print_tab(num);
    pirnt_PATTERN_LIST(case_content->clauses,num+1);
    print_tab(num);
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
    printf("exp:");
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
    print_word_desc(Cond->op,num+1);
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
int main(int argc,char**argv,char**envp) {
    // char str[80] = "cat///r/n/etc/passwd";
    int num = 0;
    char *str = ReadFile(argv[1],&num);
    // char *str = "a=c;b=at;c=fl;d=ag;e=.txt;$a$b $c$d$e;";
    /* printf("替换前:%s\n",str); */
    // StrReplace(str,"\\\n","");
    // StrReplace(str,"$IFS"," ");
    // StrReplace(str,"${IFS}"," ");
        /* printf("替换后:%s\n",str); */
    /* else printf("没有任何替换。\n"); */
    detect_bash_language(str,envp);
    if (global_command){
        print_self_command(global_command,1);
    }
    
   return 0;
} 
