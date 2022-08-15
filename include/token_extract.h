#ifndef _TOKEN_EXTRACT_H_
#define _TOKEN_EXTRACT_H_
#include "shell.h"
#include "command.h"
#include "y.tab.h"
#include "../include/parser.h"
#include "compile_regex.h"
#include "hash_token.h"
#include "bash_detect_demo.h"
#define REPLACE_WORD_PARAMS { \
    {"^[[:digit:]]+$", "num_dec", 0, NULL},   \
	  {"^[1-9][[:digit:]]{7,9}$", "IP_dec", 0, NULL},    \
    {"^[\\][0][0-7]+$", "num_oct", 0, NULL},\
    {"^[\\][0][x][[:xdigit:]]+$", "num_hex", 0, NULL},\
    {"^[[:alnum:]_]+$","",0,NULL},\
    {"^[.]{0,2}[/]([[:alnum:]_.?]+[/]?)+$","path",0,NULL},\
     {"^([/][[:alnum:]_-]+)+$","",0,NULL},\
      {"^[^/]+[.][[:alnum:]]{1,4}$","",0,NULL},\
      {"^(\-)[[:alnum:]]{1,5}(=.*)?","",0,NULL},\
      {"^(\-\-)[[:alpha:]]+","",0,NULL}, \
      {"^(\-)[[:alpha:]]{1,5}(=.*)?","",0,NULL},\
      {"^[[:alnum:]_]{1,6}$","",0,NULL}, \
     {"^[^[:alnum:]_]{1,3}$","",0,NULL}, \
     {"[`].*?[`]","",0,NULL},\
}

enum replace_word_param_num{
    IS_DECIMAL_OR_OCTAL_NUM, //匹配纯数字（十进制）或反斜杠开头（八进制预过滤）的纯数字
    IS_DECTIMAL_IP_ADDRESS_NUM, //匹配在IP范围内的十进制数字
    IS_OCTAL_NUM, //匹配在IP范围内的八进制数字
    IS_HEX_NUM, //匹配符合十六进制表示方式的数字
    NO_SPECIAL_SIGN, //无特殊字符字符串
    IS_PATH,//判断它是否是一个路径
    COMMAND_USED,   //命令调用
    IS_FILE, //判断它是不是一个文件
    IS_COMMAND_ONE_LINE, //判断是否是命令参数
    IS_COMMAND_TWO_LINE,
    IS_COMMAND_ONE_LINE_ALPHA,
    IS_AL_LESS_SIX_SIZE,//纯字母、长度小于6的词
    IS_SIGN_LESS_THREE_SIZE, //纯字符小于3的词
    QUTOED_WORD,
    MAX_REPLACE_WORD_PARAM_NUM
};

enum{
  num_of_word,
  num_of_used_sensitive_word,
  num_of_for,
  num_of_while,
  num_of_until,
  num_of_heredoc,
  num_of_operator,
  num_of_command,
  num_of_processsubstitution,
  num_of_commandsubstitution,
  num_of_function,
  num_of_param_is_env_var,
  num_of_unassigned_param_call,
  num_of_expansioned_word_call,
  num_of_expansioned_param_call,
  num_of_assigned_param_call,
  num_of_assignment,
  num_of_parameter,
  num_of_tilde,
  num_of_pipe,
  
  num_of_case,
  num_of_connection,
  num_of_group,
  num_of_arith,
  num_of_cond,
  num_of_arith_for,
  num_of_subshell,
  num_of_coproc,
  num_max_feature
};
extern int token_command(COMMAND*command,char* buf,int used,int *feature);
extern void clean_paramlist(Param * param);
#define EXPCHAR(c) ((c) == '{' || (c) == '~' || (c) == '$' || (c) == '`')
#endif // _TOKEN_EXTRACT_H_