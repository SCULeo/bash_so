#ifndef _COMPILE_REGEX_H_
#define _COMPILE_REGEX_H_
#include <regex.h>
#define word_len_token(string,line,token) switch (strlen(string)/10) \
      {\
        case 0:\
          snprintf(line, sizeof(line), "%s_%d", token,0);\
          break;\
        case 1:\
            snprintf(line, sizeof(line), "%s_%d", token,10);\
          break;\
        case 2:\
            snprintf(line, sizeof(line), "%s_%d", token,20);\
          break;\
        case 3:\
            snprintf(line, sizeof(line), "%s_%d", token,30);\
          break;\
        case 4:\
            snprintf(line, sizeof(line), "%s_%d", token,40);\
          break;\
        case 5:\
          snprintf(line, sizeof(line), "%s_%d", token,50);\
          break;\
        case 6:\
            snprintf(line, sizeof(line), "%s_%d", token,60);\
          break;\
        case 7:\
            snprintf(line, sizeof(line), "%s_%d", token,70);\
          break;\
        case 8:\
            snprintf(line, sizeof(line), "%s_%d", token,80);\
          break;\
        case 9:\
            snprintf(line, sizeof(line), "%s_%d", token,90);\
          break;\
        case 10:\
            snprintf(line, sizeof(line), "%s_%d", token,100);\
          break;\
        default:\
           snprintf(line, sizeof(line), "%s_%d", token,100);\
          break;\
      } 
/**
 * 执行批量替换的参数
 * 
 * 本文件初始化了一批这样的参数，用来对字符串进行一系列的替换
 */
struct replace_word_param {
    /**
     * 执行替换使用的正则表达式
     */
    const char *pattern;

    /**
     * 新的子串
     * 
     * 注意：其长度不宜太长，如果超过被替换部分的长度，会发生错误
     */
    const char *sub;

    /**
     * 额外的flag，默认执行 @ref regcomp() 时都是带有flag REG_EXTENDED 的，
     * 如果需要增加额外的参数，则使用ext_flag。
     */
    int ext_flag;


    /**
     * 正则编译结果
     */
   
    regex_t reg;

};
#define word_len_token(string,line,token) switch (strlen(string)/10) \
      {\
        case 0:\
          snprintf(line, sizeof(line), "%s_%d", token,0);\
          break;\
        case 1:\
            snprintf(line, sizeof(line), "%s_%d", token,10);\
          break;\
        case 2:\
            snprintf(line, sizeof(line), "%s_%d", token,20);\
          break;\
        case 3:\
            snprintf(line, sizeof(line), "%s_%d", token,30);\
          break;\
        case 4:\
            snprintf(line, sizeof(line), "%s_%d", token,40);\
          break;\
        case 5:\
          snprintf(line, sizeof(line), "%s_%d", token,50);\
          break;\
        case 6:\
            snprintf(line, sizeof(line), "%s_%d", token,60);\
          break;\
        case 7:\
            snprintf(line, sizeof(line), "%s_%d", token,70);\
          break;\
        case 8:\
            snprintf(line, sizeof(line), "%s_%d", token,80);\
          break;\
        case 9:\
            snprintf(line, sizeof(line), "%s_%d", token,90);\
          break;\
        case 10:\
            snprintf(line, sizeof(line), "%s_%d", token,100);\
          break;\
        default:\
           snprintf(line, sizeof(line), "%s_%d", token,100);\
          break;\
      } 
extern int put_word_in_buf(char *buf, char *word, int word_len, int buf_len, int used);

typedef struct param{
  char * param_list[100];
  int num;
}Param;
#endif