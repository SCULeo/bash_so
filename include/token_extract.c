#include<stdio.h>
#include "command.h"

char* instruction[] ={">", "<", "r_inputa_direction",
  ">>", "<<", "<<<",
  "<&", ">&", "<<-",
  "<&-", "&>", "<>", ">|",
  "<&", ">&",
  "r_move_input", "r_move_output", "r_move_input_word", "r_move_output_word",
  "r_append_err_and_out"};
enum puts_word_type{
    REDIRECT_TYPE,
};
static int puts_word(char *buf, size_t size,const char *string,int type,int used_size)
{
    int used_size = 0;
    #define SAFE_COPY(dst, src, n, total, used)    do{if((total) - (used) > ((n)+1)){vs_memcpy((dst), ((total)-(used)), (src), (n)); (used) = (n+1);(dst[n]) = ' ';}else{return 0;}}while(0)
    if(type == REDIRECT_TYPE)
    {
        SAFE_COPY(buf, instruction[string], strlen(instruction[string]), size, used_size);
    }
    return used_size;
    
}
void visit_redirect(REDIRECT*redirect,char *buf)
{
    #define SAFE_COPY(dst, src, n, total, used)    do{if((total) - (used) > ((n)+1)){vs_memcpy((dst), ((total)-(used)), (src), (n)); (used) = (n+1);(dst[n]) = ' ';}else{return 0;}}while(0)
    printf("redirector:{dest:%d}\n",redirect->redirector.dest);
    print_tab(num);
    printf("rflags:%d\n",redirect->rflags);
    print_tab(num);
    printf("flags:%d\n",redirect->flags);
    print_tab(num);
    printf("instruction:%s\n",instruction[redirect->instruction]);
    print_tab(num);
    if (redirect->instruction==5||redirect->instruction==14||redirect->instruction==15||redirect->instruction==18||redirect->instruction==19)
    {
        printf("redirectee:{dest:%d,filename:%s}\n",redirect->redirectee.dest,redirect->redirectee.filename);
        
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
void visit_conmand_for(struct for_com *for_com_content,char *buf)
{

}
void token_command(COMMAND*command,char* buf){
    
    
    
    switch(command->type){
    case cm_for:
       
  
        visit_conmand_for(command->value.For,buf);
        break;
    case cm_case:
       
        
        print_conmand_case(command->value.Case,buf);
        break;
    case cm_while:
       
        
        print_conmand_while(command->value.While,buf);
        break;
    case cm_simple:
       
        
        print_conmand_simple(command->value.Simple,buf);
        break;
    case cm_connection:
       
        
        print_conmand_connection(command->value.Connection,buf);
        break;
    case cm_until:
       
        
        print_conmand_while(command->value.While,buf);
        break;
    case cm_group:
       
        
        print_conmand_Group(command->value.Group,buf);
        break;
    case cm_arith:
       
       
        print_conmand_Arith(command->value.Arith,buf);    
        break;
    case cm_cond:
       
        
        print_conmand_Cond(command->value.Cond,buf);
        break;
    case cm_arith_for:
       
        
        print_conmand_ArithFor(command->value.ArithFor,buf);
        break;
    case cm_subshell:
       
        
        print_conmand_Subshell(command->value.Subshell,buf);
        break;
    case cm_coproc:
       
        
        print_conmand_Coproc(command->value.Coproc,buf);
        break;
    default:
        break;
    }
    if (command->redirects){
        visit_redirect(command->redirects,buf);
    }
}
int token_extract(COMMAND*local,char *output,int output_len)
{
    
}