#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "token_extract.h"

token_hash_t g_token_env_hash;
SEARCH_LIST g_token_fuzzy_suffix_hash;
SEARCH_LIST g_token_key_path_hash;
SEARCH_LIST g_token_key_word_file_hash;
token_hash_t g_token_keywords_hash;
__thread Param assign_param_list={0};
__thread Param used_param_list={0};
int ai_update_times = 0;
__thread int grep_flag = 0;
__thread int assignment_flag = 0;


struct replace_word_param g_command_re_params[] = REPLACE_WORD_PARAMS;

static int init_single_replace_param(struct replace_word_param *param)
{
    int ret = 0;
        ret = regcomp(&param->reg, param->pattern, REG_EXTENDED | param->ext_flag);
        if (ret != 0)
        {
            return -1;
        }
    return 0;
}

static long strHexToint(char *s)
{
	long i = 0;
	long m = 0;
	long temp=0;
	long n = 0;
	
	m=strlen(s);//十六进制是按字符串传进来的，所以要获得他的长度
	
	for(i=0;i<m;i++)
	{
        if (s[i] == '\\'||s[i]=='x')
        {
            continue;
        }
		if(s[i]>='A'&&s[i]<='F')//十六进制还要判断他是不是在A-F或者a-f之间a=10。。
			n=s[i]-'A'+10;
		else if(s[i]>='a'&&s[i]<='f')
			n=s[i]-'a'+10;
		else n=s[i]-'0';
			temp=temp*16+n;
	}
	
	return temp;
}
static long strOctToint(char *s)
{
	long i = 0;
	long m = 0;
	long temp=0;
	long n = 0;
	
	m=strlen(s);//8进制是按字符串传进来的，所以要获得他的长度
	
	for(i=0;i<m;i++)
	{
        if (s[i] == '\\')
        {
            continue;
        }
		n=s[i]-'0';
		temp=temp*8+n;
	}
	
	return temp;
}
enum puts_word_type{
    REDIRECT_TYPE,
};
#ifndef min_t
/**
 * 返回两个数值的最小值.
 *
 * @param type  类型
 * @param x     比较的数值
 * @param y	比较的数值
 * @return
 * 	x,y中的最小值
 */
#define min_t(type, x, y) ({			\
	type __min1 = (x);			\
	type __min2 = (y);			\
	__min1 < __min2 ? __min1 : __min2; })
#endif
/**
 * 相对glibc的版本,这个函数增加了目的地址长度保护,最多拷贝size个字符(包括\\0).
 *
 * @param dst	目的内存位置.
 * @param size	目的内存大小.
 * @param src	源内存位置.
 * @param n	拷贝的长度
 *
 * @return
 * 	拷贝到目的字符的字节大小.
 */
static inline int vs_memcpy(void *dst, int size, void *src, int n)
{
	int l;

	if (size < 1 || n < 1)
		return 0;

	l = min_t(int, size, n);

	memcpy(dst, src, l);

	return l;
}
int put_word_in_buf(char *buf, char *word, int word_len, int buf_len, int used)
{
  #define SAFE_COPY(dst, src, n, total, used)    do{if((total) - (used) > ((n)+1)){strncpy((dst), (src), (n)); (used) = (n+1);(dst[n]) = ' ';}else{return 0;}}while(0)
  SAFE_COPY(buf,word,word_len,buf_len,used);
  return used;
}
int  regex_match_cut(int word_param_num,char *word,char *line,int * num)
{
  int nmatch = 1;
  int ret  =0;
  int return_num = 0;
  regmatch_t pmatch[1];
  char * result = NULL;

  ret = regexec(&g_command_re_params[word_param_num].reg,word,nmatch,pmatch,0);
  if (ret == REG_NOERROR)
  {
        if (pmatch->rm_eo-pmatch->rm_so<1024)
        {
            strncpy(line,&word[pmatch->rm_so],pmatch->rm_eo-pmatch->rm_so);
            *num = pmatch->rm_eo-pmatch->rm_so;
            return_num = 1;
        }
          
        
  }
 
  return return_num;
}
char * regex_match(int word_param_num,char *word)
{
  int nmatch = 1;
  int ret  =0;
  regmatch_t pmatch[1];
  char * result = NULL;
  char * temp = NULL;
  memset(pmatch,0,sizeof(regmatch_t));

  ret = regexec(&g_command_re_params[word_param_num].reg,word,nmatch,pmatch,0);
  if (ret == REG_NOERROR)
      {
          result = g_command_re_params[word_param_num].sub;
  }
  if (temp)
  {
    free(temp);
  }
  return result;
}


/**
 * @brief 
 * 判断word节点的字符串是否为纯数字，数字包括三种形式，10进制，8进制，16进制，
 * 并再次判断数字是否为IP地址,IP地址范围为1.1.1.1-255.255.255.255
 * @param buf 用于存放拼接后的token的缓冲区
 * @param string 待判断的word节点的字符串
 * @param used 已经使用空间
 * @param feature 特征统计
 * @return int 
 */
int visit_number(char * buf,char *word,int used,int * feature)
{
  int nmatch = 1;
  regmatch_t pmatch[1];
  char * result  =NULL;
  int ret = 0;
  
  ret  = regexec(&g_command_re_params[IS_DECIMAL_OR_OCTAL_NUM].reg,word,nmatch,pmatch,0);
  if (ret == REG_NOERROR)
  {
      result = g_command_re_params[IS_DECIMAL_OR_OCTAL_NUM].sub;
      char * temp_result = NULL;
      temp_result = regex_match(IS_DECTIMAL_IP_ADDRESS_NUM,word);
      if (temp_result!=NULL)
      {
        result = temp_result;
      }
      goto return_result;

  }
    ret = regexec(&g_command_re_params[IS_OCTAL_NUM].reg,word,nmatch,pmatch,0);
    if (ret == REG_NOERROR){
              result = g_command_re_params[IS_OCTAL_NUM].sub;
              long int_ip = strOctToint(word);
              if (16843009<=int_ip&&int_ip<=4294967295)
            {
                    result = "IP_oct";
            }
            goto return_result;
    }
  
  
      ret  = regexec(&g_command_re_params[IS_HEX_NUM].reg,word,nmatch,pmatch,0);
      if (ret  == REG_NOERROR)
      {
          result = g_command_re_params[IS_HEX_NUM].sub;
          long int_ip = strHexToint(word);
          if (16843009<=int_ip&&int_ip<=4294967295)
          {
              result = "IP_hex";
          }
            goto return_result;
      }
      
  
return_result:
  
  if (result!=NULL)
  {
     used += put_word_in_buf(buf+used,result,strlen(result),BUF_LEN-used,used);
     return used;
  }
  return used;
  
 
}
/**
 * @brief 删除字符串中的指定字符
 *        Delete the specified character in the string
 * @param str 
 * @param c 
 */
void delchar( char *str, char c )
{
  int i,j;
  for(i=j=0;str[i]!='\0';i++)
  {
    if(str[i]!=c)//判断是否有和待删除字符一样的字符
    {
      str[j]=str[i];
      j++;
    }
  }
  str[j]='\0';//字符串结束
}
void remove_quote_mark(char * temp_str)
{ 
  delchar(temp_str,'\'');
  delchar(temp_str,'"');
  delchar(temp_str,'`');
  delchar(temp_str,'\\');
}
int visit_word_char(char * buf, char * token,char * word_char,int used,int *feature)
{
   int nmatch = 1;
  regmatch_t pmatch[1];
  char * result  =NULL;
  int ret = 0;
  char line[4096]={0};
  char temp_string[4096]={0};
  int temp_used = 0;
  if (strlen(word_char)>2048)
    return used;
  strcpy(temp_string,word_char);
  remove_quote_mark(temp_string);

 
   
  

  ret  = regexec(&g_command_re_params[NO_SPECIAL_SIGN].reg,temp_string,nmatch,pmatch,0);

  
  if (ret == REG_NOERROR)
  {
     
      word_len_token(word_char,line,token);
      used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
  
  }

  return used;
}
/**
 * @brief 长度泛化，当所有的泛化都失效时，就把它当作一个普通的字符串，进行长度泛化
 * 
 * @param buf 
 * @param token 
 * @param word_char 
 * @param used 
 * @return int 
 */
int word_length_generailzation(char * buf, char * token,char * word_char,int used)
{
    char line[4096]={0};
    word_len_token(word_char,line,token);
    used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used); 
    return used;
}
void get_paramname(char * word,char *param_name)
{
  int i  = 0;
  int len =  strlen(word);
  for(i=len-1; i>=0; i--)
        if(word[i] == '='){ //找到等号
            if (i<200)
            {
                strncpy(param_name,word,i);//获取参数名字
            }
            
            param_name[i] = '\0';
            break;    
        }  
}
/**
 * @brief 向参数列表中增加一个参数
 * 
 * @param word char* 待加入参数
 * @param param Param* 参数列表
 * @return int 
 */
int add_paramlist(char * word,Param * param)
{
  
  param->param_list[param->num] = (char *)malloc(strlen(word)*sizeof(char));
  memset(param->param_list[param->num],0,strlen(param->param_list[param->num]));
  memcpy(param->param_list[param->num],word,strlen(word));
  param->num++;
  return 0;
}
/**
 * @brief 清空参数列表
 * 
 * @param param Param* 参数列表
 */
void clean_paramlist(Param * param)
{
  int i;
  for (i  = 0; i < param->num;i++)
  {
     free(param->param_list[i]);
     param->param_list[i] =NULL;
  }
  param->num = 0;
}
/**
 * @brief 判断正在使用参数是否曾经被赋值，如果有，返回 0 ，没有返回 1
 * 
 * @param word char*正在使用参数
 * @param param Param*参数列表
 * @return int 
 */
int IF_word_in_paramlist(char * word,Param * param)
{
  int i = 0;
  for (i  = 0; i < param->num;i++)
  {
    if (!strcmp(word,param->param_list[i]))
    {
      return 1;
    }
  }
  return 0;
}


int find_brace(char *word)
{
  int i;
  int num = 0;
  int brace;
  int brace_start = 0;
  for (i=0;i<strlen(word);i++)
  {
      if (word[i]=='{')
      {
        brace_start = 1;
      }
      if (word[i]=='{'&&brace_start)
      {
        num++;
        brace_start = 0;
      }
  }
  return num;
}

int visit_qutoed(char * buf ,char *word,int used,int *feature)
{
        
        char temp_word[4096]={0};

        if (strlen(word)>2048)
        {
          return used;
        }

        strncpy(&temp_word,&word[1],strlen(word)-2);
        temp_word[strlen(word)-2] = '\0';
        if ((temp_word[strlen(word)-3]=='$')||strstr(temp_word,"$/"))
        {
          used+= put_word_in_buf(buf+used,"maybe_regex",strlen("maybe_regex"),BUF_LEN-used,used);

        }else
        {
          used = run_and_visit_command(&temp_word,buf,used,feature);
        }
        
        free(temp_word);

        return used;
       
}
/**
 * @brief 将引号中的字符串重新放入command中进行二次分析
 * 
 * @param word 
 * @param buf 
 * @param used 
 * @param feature 
 * @return int 
 */
int run_and_visit_command(char *word,char *buf ,int used ,int *feature)
{
    COMMAND *local_command = NULL;
     pthread_mutex_lock(&gMutex);
        // parse_and_execute (savestring (msg), "-c", SEVAL_NOHIST|SEVAL_RESETLINE);
        run_one_command(word);
        pthread_mutex_unlock(&gMutex);
        local_command = global_command;
        global_command = NULL;
        if (local_command){
             used  = token_command(local_command,buf,used,feature);  
        }
        pthread_mutex_lock(&cMutex);
        current_token  = 0;
        parse_and_execute_cleanup (-1);
        pthread_mutex_unlock(&cMutex);

        pthread_mutex_lock(&c2Mutex);
        dispose_command(local_command);
        pthread_mutex_unlock(&c2Mutex);
        return used;
}
/**
 * @brief  形如$X 的变量泛化
 *         如果这个变量曾经被赋值，也就是存在于已经赋值的变量列表assign_param_list里，
 *         那么泛化为$assigned，否则泛化为$unassigned
 *         
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int visit_parament(char * buf, char * word,int used,int * feature)
{
  char temp_word[4096]={0};
  if (word[0]=='$'&&strlen(word)>=2)
  {
    feature[num_of_parameter]++;
    
    if (strlen(word)>4&&word[1]=='{')
    {
      if (strlen(word)>2048)
      {
        return used;
      }
      strcpy(temp_word,word);
      temp_word[strlen(temp_word)-1] = '\0';
      if (visit_env_var(buf,&temp_word[2],used,ai_update_times))
      {
        used+=put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
        feature[num_of_param_is_env_var]++;
      } 
      goto finish;
    }
    if (IF_word_in_paramlist(&word[1],&assign_param_list))
    {
      used+=put_word_in_buf(buf+used,"$assigned",strlen("$assigned"),BUF_LEN-used,used);
      feature[num_of_assigned_param_call]++;
    }
    else
    {
      used+=put_word_in_buf(buf+used,"$unassigned",strlen("$unassigned"),BUF_LEN-used,used);
      feature[num_of_unassigned_param_call]++;
    }
  }
finish:
  return used;
}



/**
 * @brief 参数表达式中X=Y，对于值Y的泛化，主要分为两种
 *        第一种是环境变量泛化，当变量是已经存在的环境变量时，泛化为$env
 *        第二种是参数变量泛化，除开第一种情况，针对参数变量泛化，如果这个变量曾经被赋值，也就是存在于已经赋值的变量列表assign_param_list里，
 *         那么泛化为$assigned，否则泛化为$unassigned
 * 
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int visit_value(char * buf, char * word,int used,int * feature)
{
       int flag = 0;
       char * temp = word;
       if (word[0]!='$')
       {
         return used;
       }
       if (word[0]=='$'&&strlen(word)>2&&(word[1]=='\''||word[1]=='\"'))
       {
         used+=put_word_in_buf(buf+used,"$'escape'",strlen("$'escape'"),BUF_LEN-used,used);
         return used;
       }
       flag = visit_env_var(buf,&word[1],used,ai_update_times);
       if (flag)
       {
         used+=put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
         feature[num_of_param_is_env_var]++;
       }else{
           used = visit_parament(buf,word,used,feature);
       }
       return used;
}
/**
 * @brief 赋值表达式特征泛化
 *        主要分为两个部分，第一个部分是键泛化，第二个部分是值泛化，值泛化参考visit_value函数。
 *        键泛化，泛化为flag_assign reassign_env/assigned_param flag_assign = flag_assign_end
 *        其中=和flag_assign_end赋值结束符号之间的值泛化，根据值泛化函数，在一定情况下会存在。
 * 
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int visit_aissignment(char * buf, WORD_DESC * word,int used,int * feature)
{
        char param_name[1024]={0};
        assignment_flag = 1;
       get_paramname(word->word,&param_name);
       feature[num_of_assignment]++;
       used+=put_word_in_buf(buf+used,"flag_assign",strlen("flag_assign"),BUF_LEN-used,used);
       int flag = 0;

       flag = visit_env_var(buf,&param_name,used,ai_update_times);
       if (flag)
       {
         used+=put_word_in_buf(buf+used,"reassign_env",strlen("reassign_env"),BUF_LEN-used,used);
       }else if (IF_word_in_paramlist(&param_name,&assign_param_list)){
         used+=put_word_in_buf(buf+used,"assigned_param",strlen("assigned_param"),BUF_LEN-used,used);
         
       }else{
         used+=put_word_in_buf(buf+used,"unassigned_param",strlen("unassigned_param"),BUF_LEN-used,used);
         add_paramlist(&param_name,&assign_param_list);
       }

      used+=put_word_in_buf(buf+used,"=",strlen("="),BUF_LEN-used,used);
      char * temp_word=NULL;
      temp_word = strstr(word->word,"=");
      if (strlen(temp_word)>2)
        used = visit_value(buf,&temp_word[1],used,feature);
       
       
      return used;
}
/**
 * @brief Get the file from path object
 * 
 * @param path 路径 
 * @param dst 提取的文件名字
 * @return int 
 */
int get_file_from_path(char *path,char*dst)
{
  int len = strlen(path);
  int i;
  
  if (!strstr(path,"\\"))
  {

    return -1;
  }
  for(i=len-1; i>=0; i--)
        if(path[i] == '\\'&&strlen(&path[i+1])<1024){ //找到最后一个斜杠

            strcpy(dst,&path[i+1]);//获取文件名(不含路径,含后缀)
            break;    
        }  
  return 0;
}
/**
 * @brief 
 * 
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int visit_path(char * buf, char * string,int used,int * feature)
{
  int nmatch = 1;
  regmatch_t pmatch[1];
  char * result  =NULL;
  int ret = 0;
  

  if((strstr(string,".")&&strlen(string)==1))
  {
     result = "current_path";
  }
  char * temp_result = NULL;
  temp_result  = regex_match(IS_PATH,string);
  
  if (temp_result)
  {
      result = temp_result;
      ret = regexec(&g_command_re_params[COMMAND_USED].reg,string,nmatch,pmatch,0);
      if (ret == REG_NOERROR)
      {
          feature[num_of_used_sensitive_word]++;
      }
      if (strstr(string,"../"))
      {
        result = "parent_path";
      }else if (strstr(string,"./"))
      {
        result = "current_path";
      }
      char temp_file[1024]={0};

      if (get_file_from_path(string,&temp_file)==-1)
      {
        goto return_result;
      }
      
      ret = regexec(&g_command_re_params[IS_FILE].reg,&temp_file,nmatch,pmatch,0);

      if (ret == REG_NOERROR)
      {
          result = "file_path";
      }
      goto return_result;

  }

  return_result:
  
  if (result!=NULL)
  {
     used += put_word_in_buf(buf+used,result,strlen(result),BUF_LEN-used,used);
     return used;
  }
  return used;
}


int visit_no_change(char * buf, char * string,int used,int * feature)
{
  int ret = 0;
  char * result =NULL;
  char *temp_file = NULL;
  int nmatch = 1;
  char line[1024]={0};
  regmatch_t pmatch[1];
  int temp_used = 0;
  if ((strstr(string,"'")||strstr(string,"\"")||strstr(string,"`")||strstr(string,"\\"))&&!strstr(string," "))
      remove_quote_mark(string);
      //判断word是否是环境变量
  
  if (visit_env_var(buf,string,used,ai_update_times))
  {
    used += put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
    goto return_result;
  }
  //判断word->word字段是否是纯数字
  temp_used  = visit_number(buf,string,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto return_result;
   }
  //判断word->word字段是否属于敏感信息
   temp_used = visit_sens_information(buf,string,used,ai_update_times);
  if (used != temp_used)
   {
     used = temp_used;
     goto return_result;
   }


   //判断word->word字段是否是路径相关
  temp_used  = visit_path(buf,string,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto return_result;
   }
   //判断word->word字段是否是文件
   temp_used  = visit_file(buf,string,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto return_result;
   }
    //判断word->word字段是否是短横线开头的参数
    temp_used  = visit_command_parament(buf,string,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto return_result;
   }

    //参数泛化，如果该参数存在于assign_param_list列表中，那么该参数泛化为$assigned，否则泛化为$unassigned
  temp_used = visit_value(buf,string,used,feature);
  if (used != temp_used)
  {
     used = temp_used;
     goto return_result;
  }

    if (strlen(string)>6)
    {
      goto return_result;
    }
 
    ret = regexec(&g_command_re_params[IS_AL_LESS_SIX_SIZE].reg,string,nmatch,pmatch,0);
    if (ret == REG_NOERROR)
    {
      
      result = string;
      goto return_result;
    }
    ret = regexec(&g_command_re_params[IS_SIGN_LESS_THREE_SIZE].reg,string,nmatch,pmatch,0);
    if (ret == REG_NOERROR)
    {
      
      result = string;
      goto return_result;
    }
  
   
      
return_result:
  
  if (result!=NULL)
  {
     used += put_word_in_buf(buf+used,result,strlen(result),BUF_LEN-used,used);
     return used;
  }
  return used;
}

void quickSort(int a[],int l,int r) {
    if(l>=r)
        return;
    int i = l;
    int j = r;
    int key = a[l];//选择第一个数为key
    while(i<j) {
        while(i<j && a[j]>=key)//从右向左找第一个小于key的值
            j--;
        if(i<j) {
            a[i] = a[j];
            i++;
        }
        while(i<j && a[i]<key)//从左向右找第一个大于key的值
            i++;
        if(i<j) {
            a[j] = a[i];
            j--;
        }
    }
    a[i] = key;
    quickSort(a, l, i-1);//继续排左部分，递归调用
    quickSort(a, i+1, r);//继续排右部分，递归调用
}
/**
 * @brief 冒泡排序
 * 
 * @param a 
 * @param len 
 */
void sort(char *a,int len)
{ 
  int i=0;
  int j;
  int t;
  for(i=0;i<len-1;i++)
  {
      for(j=0;j<len-i-1;j++)
    {
      if(a[j]>a[j+1])
      {
        t=a[j];
        a[j]=a[j+1];
        a[j+1]=t;
      }
    }
  }
}
/**
 * @brief 命令行参数处理
 *        -单短杠
 *        全英文，排序后返回
 *        数字，保留返回
 *        英文混合数字，只保留数字前的英文部分排序后输出
 *        数字混合英文（这个应该过不了语法检测）
 *        --双短杠
 *        保留数字前英文
 * @param buf 
 * @param string 
 * @param used 
 * @param feature 
 * @return int 
 */
int visit_command_parament(char * buf, char * string,int used,int * feature)
{
  int ret = 0;
  char * result =NULL;
  char *temp_file = NULL;
  int nmatch = 1;
  char line[4096]={0};
  regmatch_t pmatch[1];
  int num = 0;

    ret = regexec(&g_command_re_params[IS_COMMAND_TWO_LINE].reg,string,nmatch,pmatch,0);
    if (ret == REG_NOERROR)
    {
      strncpy(line,&string[pmatch->rm_so],pmatch->rm_eo-pmatch->rm_so);
      result = line;
      goto return_result;
    }
    ret = regexec(&g_command_re_params[IS_COMMAND_ONE_LINE].reg,string,nmatch,pmatch,0);
    if (ret == REG_NOERROR)
    {
      ret = regex_match_cut(IS_COMMAND_ONE_LINE_ALPHA,string,line,&num);
      
      if (ret)
      {
        
          sort(line,num);
          result = line;
      }else{
        result = "-num";
      }
      
      goto return_result;
    }
      
return_result:
  if (result!=NULL)
  {
     used += put_word_in_buf(buf+used,result,strlen(result),BUF_LEN-used,used);
     return used;
  }
  return used;
}

int visit_file(char * buf, char * string,int used,int * feature)
{
  int ret = 0;
  char * result =NULL;
  char temp_file[1024]={0};
  int nmatch = 1;
  regmatch_t pmatch[1];

  
  if(get_file_from_path(string,&temp_file)==-1)
  {
    ret = regexec(&g_command_re_params[IS_FILE].reg,string,nmatch,pmatch,0);
  }else{
    ret = regexec(&g_command_re_params[IS_FILE].reg,&temp_file,nmatch,pmatch,0);
  }
    
  
  
  
  if (ret == REG_NOERROR)
  {
      result = "maybe_file";
  }
return_position:

  if (result!=NULL)
  {
     used += put_word_in_buf(buf+used,result,strlen(result),BUF_LEN-used,used);
     return used;
  }
  return used;
}

int deal_word(char * buf, WORD_DESC * word,int used,int * feature)
{
  int temp_used = 0;
   feature[num_of_word]++;
   char token_prefix[20] ={0};
   if (!word->word)
   {
     return used;
   }
   if (word->flags&W_QUOTED)
   {
     strcpy(token_prefix,"string");
   }else{
     strcpy(token_prefix,"word");
   }
  if(strstr(word->word,"~"))
  {
    feature[num_of_tilde]++;
    used += put_word_in_buf(buf+used,"~",strlen("~"),BUF_LEN-used,used);
    goto finish;
  }
   //判断word是否是环境变量
  if (visit_env_var(buf,word->word,used,ai_update_times))
  {
    used += put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
    goto finish;
  }
  //判断word->word字段是否是纯数字
  temp_used  = visit_number(buf,word->word,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }
  //判断word->word字段是否属于敏感信息
   temp_used = visit_sens_information(buf,word->word,used,ai_update_times);
  if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }


   //判断word->word字段是否是路径相关
  temp_used  = visit_path(buf,word->word,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }
   //判断word->word字段是否是文件
   temp_used  = visit_file(buf,word->word,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }
    //判断word->word字段是否是短横线开头的参数
    temp_used  = visit_command_parament(buf,word->word,used,feature);
   if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }

   
  //参数泛化，如果该参数存在于assign_param_list列表中，那么该参数泛化为$assigned，否则泛化为$unassigned
  temp_used = visit_value(buf,word->word,used,feature);
  if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }
  
  if (grep_flag)
  {
     temp_used = visit_word_char(buf,token_prefix,word->word,used,feature);
  }
  else
  {
     temp_used = visit_no_change(buf,word->word,used,feature);
  }
  if (used != temp_used)
   {
     used = temp_used;
     goto finish;
   }
  if (word->flags&&W_QUOTED \
        &&word->word[0]!='\'' \
        &&(word->word[0]=='`'))
    {
      used = visit_qutoed(buf,word->word,used,feature) ; 
      goto finish;
    }
  used  = word_length_generailzation(buf,token_prefix,word->word,used);
  
  finish:
  return used;
}
int deal_arith_substitution(char * buf, WORD_DESC * word,int used,int * feature)
{
  char * p =NULL;
  int len =0;
  char temp_word[4096]={0};
  if (temp_word>2048)
  {
    return used;
  }
  strncpy(temp_word,&word->word[3],strlen(word->word)-5);
  used+=put_word_in_buf(buf+used,"flag_arith_sub",strlen("flag_arith_sub"),BUF_LEN-used,used);
  used = run_and_visit_command(temp_word,buf,used,feature);
  used+=put_word_in_buf(buf+used,"flag_arith_sub_end",strlen("flag_arith_sub_end"),BUF_LEN-used,used);
}

/**
 * @brief 命令替换
 *        总共有三种情况：
 *        $() $`` ``
 *        其中第一种和第二种类似，只是第二种``里面的可以进行escape编码第一种不行
 *        ``第三种
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int deal_command_substitution(char * buf, WORD_DESC * word,int used,int * feature)
{
  
  char line[1024] = {0};
  char temp_word[4096] = {0};
  int num = 0;
  if(word->word[0]=='$')  
  {
    used+=put_word_in_buf(buf+used,"flag_word_parts",strlen("flag_word_parts"),BUF_LEN-used,used);
    if (word->word[1]=='`')
    {
          used+=put_word_in_buf(buf+used,"$'escape'",strlen("$'escape'"),BUF_LEN-used,used);
    }
    used+=put_word_in_buf(buf+used,"flag_cmdsub",strlen("flag_cmdsub"),BUF_LEN-used,used); 
    if (strlen(word->word)>3&&strlen(word->word)<2048)
      {

          strcpy(temp_word,word->word);
          temp_word[strlen(temp_word)-1]='\0';
          used = run_and_visit_command(temp_word+2,buf,used,feature);
      }
    used+=put_word_in_buf(buf+used,"flag_cmdsub_end",strlen("flag_cmdsub_end"),BUF_LEN-used,used);
    used+=put_word_in_buf(buf+used,"flag_word_parts_end",strlen("flag_word_parts_end"),BUF_LEN-used,used);
  }else{
    used+=put_word_in_buf(buf+used,"flag_cmdsub",strlen("flag_cmdsub"),BUF_LEN-used,used); 
    if (strlen(word->word)>2)
      {
          
          int ret = regex_match_cut(QUTOED_WORD,word->word,&line,&num);
          line[strlen(line)-1]='\0';
          if (ret)
          {
            used = run_and_visit_command(&line[1],buf,used,feature);
          }   
      }
    used+=put_word_in_buf(buf+used,"flag_cmdsub_end",strlen("flag_cmdsub_end"),BUF_LEN-used,used);
  }
 
  
finish:
  
  return used;
}
/**
 * @brief 变量替换 结构如下${},总共有三种类型%，#，：。
 * ${变量#关键词}		若变量内容从头开始的数据符合『关键词』，则将符合的最短数据初除 
 * ${变量##关键词} 	若变量内容从头开始的数据符合『关键词』，则将符合的最长数据初除 
 * ${变量%关键词} 	若变量内容从尾向前的数据符合『关键词』，则将符合的最短数据初除 
 * ${变量%%关键词} 	若变量内容从尾向前的数据符合『关键词』，则将符合的最长数据初除 
 * ${变量/旧字符串/新字符串} 		若变量内容符合『旧字符串』则『第一个旧字符串会被新字符串叏代』
 * ${变量//旧字符串/新字符串} 	若变量内容符合『旧字符串』则『全部的旧字符串会被新字符串叏代』
 * 变量替换还处理一种特殊情况也就是对变量的使用 ${变量：NUM_1：NUM_2}/${变量：NUM_1}   
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int deal_variable_substitution(char * buf, WORD_DESC * word,int used,int * feature)
{
  char * p =NULL;
  int len =0;
  char temp_word[4096]={0};
  if (strlen(word->word)>2048)
  {
    return used;
  }
  strncpy(temp_word,word->word,strlen(word->word));
  temp_word[strlen(word->word)-1] = '\0';
  if (strstr(word->word,":")){
    p = strtok(temp_word,":");
    
    goto deal;
  }
  if (strstr(word->word,"#")){
    p = strtok(temp_word,"#");
    goto deal;
  }
  if (strstr(word->word,"/")){
    p = strtok(temp_word,"/");
    goto deal;
  }
  if (strstr(word->word,"%")){
    p = strtok(temp_word,"%");
    goto deal;
  }
  
deal:
  if (p!=NULL)
  {
    if (visit_env_var(buf,&p[2],used,ai_update_times))
        {
          
          used+=put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
          feature[num_of_param_is_env_var]++;
          goto finish;
        }
    if (IF_word_in_paramlist(&p[2],&assign_param_list))
        {
          used+=put_word_in_buf(buf+used,"$assigned",strlen("$assigned"),BUF_LEN-used,used);
          feature[num_of_assigned_param_call]++;
          goto finish;
        }
        else
        {
          used+=put_word_in_buf(buf+used,"$unassigned",strlen("$unassigned"),BUF_LEN-used,used);
          feature[num_of_unassigned_param_call]++;
          goto finish;
        }
  }
  if (temp_word[0]=='$'&&temp_word[1]=='{'||temp_word[strlen(temp_word)-1]=='}')
  { 
      feature[num_of_parameter]++;
      
      temp_word[strlen(temp_word)-1] = '\0';
      if (visit_env_var(buf,&temp_word[2],used,ai_update_times))
        {
          
          used+=put_word_in_buf(buf+used,"$env",strlen("$env"),BUF_LEN-used,used);
          feature[num_of_param_is_env_var]++;
          goto finish;
        } 

        if (IF_word_in_paramlist(&temp_word[2],&assign_param_list))
        {
          used+=put_word_in_buf(buf+used,"$assigned",strlen("$assigned"),BUF_LEN-used,used);
          feature[num_of_assigned_param_call]++;
          goto finish;
        }
        else
        {
          used+=put_word_in_buf(buf+used,"$unassigned",strlen("$unassigned"),BUF_LEN-used,used);
          feature[num_of_unassigned_param_call]++;
          goto finish;
        }
      
  }
finish:
  return used;
}

/**
 * @brief 进程替换形式如 <(XXX) 或者 >(XXX) 其中XXX代表任意
 *    
 * @param buf 
 * @param word 
 * @param used 
 * @param feature 
 * @return int 
 */
int deal_process_substitution(char * buf, WORD_DESC * word,int used,int * feature)
{

    char * p = NULL;
    int len = 0;
    char temp_word[4096]={0};
    used+=put_word_in_buf(buf+used,"flag_procsub",strlen("flag_procsub"),BUF_LEN-used,used); 
    if (strlen(word->word)>2&&strlen(word->word)<2048)
      {
          strncpy(temp_word,&word->word[2],strlen(word->word)-3);
          used = run_and_visit_command(temp_word,buf,used,feature);
    
      }
    used+=put_word_in_buf(buf+used,"flag_procsub_end",strlen("flag_procsub_end"),BUF_LEN-used,used);
  return used;
}

/**
 * @brief 访问word的节点，针对bash语法树的word节点进行特征提取和泛化
 * 
 * @param buf 泛化结构存放位置
 * @param word word节点结构
 * @param used 已经使用的buf空间
 * @param feature 统计特征计数数组
 * @return int 
 */
int visit_word(char * buf, WORD_DESC * word,int used,int * feature)
{
  
   switch(word->self_flags)
   {
     case W_VARIABLE_SUBSTITUTION:
     {
      //  TODO:
      feature[num_of_expansioned_param_call]++;
      used = deal_variable_substitution(buf,word,used,feature);
      goto finish;
     }
     case W_PROCESS_SUBSTITUTION:
     {
       used = deal_process_substitution(buf,word,used,feature);
       feature[num_of_processsubstitution]++;
       goto finish;
     }
     case W_BRACE:
     {
       feature[num_of_expansioned_word_call]+=find_brace(word);
       break;
     }
     case W_COMMAND_SUBSTITUTION:
     {
       used = deal_command_substitution(buf,word,used,feature);
       feature[num_of_commandsubstitution]++;
       goto finish;
       
     }
     case W_ARITHMETIC_SUBSTITUTION:
     {
       
       used+=put_word_in_buf(buf+used,"arith_sub",strlen("arith_sub"),BUF_LEN-used,used);
       goto finish;
       
     }
     
     case W_ASSIGNMENT_SELF:
     {
       used  = visit_aissignment(buf,word,used,feature);
       goto finish;
       
     }
   }
   
   
   used = deal_word(buf,word,used,feature);
  
   
finish:
   return used;
}

int visit_heredoc_body(REDIRECT*redirect,char *buf,int used,int feature)
{
      used = visit_word(buf,redirect->redirectee.filename->word,used,feature);
      used = visit_word(buf,redirect->here_doc_eof,used,feature);

      return used;
}
void itoa(char *one_buf,int num,int len)
{
 
  snprintf(one_buf,len,"%d",num);
  
}
int visit_heredoc_header(REDIRECT*redirect,char *buf,int used,int *feature)
{

    int kill_leading;
    char *x;
    kill_leading = redirect->instruction == r_deblank_reading_until;
    char line[4096] = {0};
    // redirect结构通常分为三部分，首部，连接符号，尾部
    // 例如 1<&2, 其中1 是头部，<&是连接符号，2是尾部
    
    // 头部泛化
    if (redirect->rflags & REDIR_VARASSIGN)
    {
        used = visit_word(buf,redirect->redirector.filename->word,used,feature);
    }
    else if (redirect->redirector.dest != 0)
    {
     char temp[2] ={0};
     itoa(temp,redirect->redirector.dest,2);
     used+=put_word_in_buf(buf+used,temp,strlen(strlen(temp)),BUF_LEN-used,used);
    }

    // 连接符号泛化
    if(kill_leading)
    {
        used+=put_word_in_buf(buf+used,"<<-",strlen("<<-"),BUF_LEN-used,used);
    }else{
        used+=put_word_in_buf(buf+used,"<<",strlen("<<"),BUF_LEN-used,used);
    }
        

    // 尾部泛化
    if (redirect->redirectee.filename->flags & W_QUOTED)
    {
        feature[num_of_heredoc]++;
        snprintf(line, sizeof(line), "'%s'", redirect->here_doc_eof);
        used+= put_word_in_buf(buf+used,redirect->here_doc_eof,sizeof(redirect->here_doc_eof),BUF_LEN-used,used);
        memset(line,0,sizeof(line));
    }
    else
    {
        used+= put_word_in_buf(buf+used,redirect->here_doc_eof,sizeof(redirect->here_doc_eof),BUF_LEN-used,used);
    }
    return used;
        
      
}

int visit_redirection(char *buf,REDIRECT*redirect,int used,int* feature)
{

  int redirector, redir_fd;
  WORD_DESC *redirectee, *redir_word;
  char line[4096]={0};
  redirectee = redirect->redirectee.filename;
  redir_fd = redirect->redirectee.dest;

  redir_word = redirect->redirector.filename;
  redirector = redirect->redirector.dest;

  switch (redirect->instruction)
    {
    case r_input_direction:
      if (redirect->rflags & REDIR_VARASSIGN)
      {
        
        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 0)
      {  
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line));
      }
      snprintf(line, sizeof(line), "%s", "<");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_output_direction:
      if (redirect->rflags & REDIR_VARASSIGN)
      {
        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 1)
      {
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line)); 
      }
      snprintf(line, sizeof(line), "%s",">");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_inputa_direction:	/* Redirection created by the shell. */
      snprintf(line, sizeof(line), "%s", "&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      break;

    case r_output_force:
      if (redirect->rflags & REDIR_VARASSIGN)
      {
        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 1)
      {
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line)); 
      }
      snprintf(line, sizeof(line), "%s",">|");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_appending_to:
      if (redirect->rflags & REDIR_VARASSIGN)
      {
        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 1)
      {
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line)); 
      }
      snprintf(line, sizeof(line), "%s",">>");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_input_output:
      if (redirect->rflags & REDIR_VARASSIGN)
      {

        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 1)
      {
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line)); 
      }
      snprintf(line, sizeof(line), "%s","<>");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,line,used,feature);
      break;

    case r_deblank_reading_until:
    case r_reading_until:
      used = visit_heredoc_header (redirect,buf,used,feature);
      
      used = visit_heredoc_body (redirect,buf,used,feature);
      break;

    case r_reading_string:
      if (redirect->rflags & REDIR_VARASSIGN)
      {
        used = visit_word(buf,redir_word,used,feature);
      }
      else if (redirector != 0)
      {
          snprintf(line, sizeof(line), "%d", redirector);
          used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line)); 
      }
      snprintf(line, sizeof(line), "%s","<<<");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_duplicating_input:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
 
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      break;

    case r_duplicating_output:
       if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      break;

    case r_duplicating_input_word:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      break;

    case r_duplicating_output_word:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      break;

    case r_move_input:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d-",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d-",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      break;

    case r_move_output:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d-",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%d-",redir_fd);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
    }
      break;

    case r_move_input_word:
      if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s","<&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      break;

    case r_move_output_word:
       if (redirect->rflags & REDIR_VARASSIGN)
    {
      used = visit_word(buf,redir_word,used,feature);
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    }
      else
    {
      snprintf(line, sizeof(line), "%d",redirector);
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      snprintf(line, sizeof(line), "%s",">&");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      
      used = visit_word(buf,redirectee,used,feature);
    };
      break;

    case r_close_this:
      if (redirect->rflags & REDIR_VARASSIGN)
	{
            used = visit_word(buf,redir_word,used,feature);
            
            snprintf(line, sizeof(line), "%s",">&-");
            used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
            memset(line,0,sizeof(line));
      }
      else
	{
            snprintf(line, sizeof(line), "%d",redirector);
            used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
            memset(line,0,sizeof(line));
            
            snprintf(line, sizeof(line), "%s",">&-");
            used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
            memset(line,0,sizeof(line));
      }
      break;

    case r_err_and_out:
      snprintf(line, sizeof(line), "%s","&>");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;

    case r_append_err_and_out:
      snprintf(line, sizeof(line), "%s","&>>");
      used+=put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));

      used = visit_word(buf,redirectee,used,feature);
      break;
    }
    return used;
}
int visit_redirect(REDIRECT*redirects,char *buf,int used,int *feature)
{
    char *rw;
    used+=put_word_in_buf(buf+used,"flag_redirect",strlen("flag_redirect"),BUF_LEN-used,used);
    REDIRECT* redirect = redirects;
    while (redirect)
    {
            if(redirect->instruction == r_reading_until || redirects->instruction == r_deblank_reading_until)
        {
            used = visit_heredoc_header(redirect,buf,used,feature);
        }
        else if (redirects->instruction == r_duplicating_output_word && (redirects->flags & REDIR_VARASSIGN) == 0 && redirects->redirector.dest == 1)
        {
            rw = redirects->redirectee.filename->word;
	        if (rw && *rw != '-' && DIGIT (*rw) == 0 && EXPCHAR (*rw) == 0)
	            redirects->instruction = r_err_and_out;
	        used =  visit_redirection(buf,redirect,used,feature);
	        redirects->instruction = r_duplicating_output_word;
            
        }
        else
        {
            used =  visit_redirection(buf,redirect,used,feature);
        }
        redirect = redirect->next;
    }
   used+=put_word_in_buf(buf+used,"flag_redirect_end",strlen("flag_redirect_end"),BUF_LEN-used,used);
   return used;
}
int is_grep_type(char * string)
{
  if (!strcmp(string,"grep"))
  {
    return 1;
  }
  if (!strcmp(string,"pgrep"))
  {
    return 1;
  }
  if (!strcmp(string,"echo"))
  {
    return 1;
  }
  return 0;
}
int visit_word_list(char * buf,WORD_LIST * list,int used,int* feature,char * separate)
{
  WORD_LIST *w;
  int local_assignment_flag = 0;
  for (w = list; w; w = w->next)
  {
    grep_flag = is_grep_type(w->word);
    used = visit_word(buf,w->word,used,feature);
    if (strcmp(separate," ")!=0)
    {
      used+=put_word_in_buf(buf+used,separate,strlen(separate),BUF_LEN-used,used);
    }
    if (assignment_flag == 1)
    {
      local_assignment_flag = 1;
      assignment_flag = 0;
    }
  }
  if (local_assignment_flag == 1)
  {
    used+=put_word_in_buf(buf+used,"flag_assign_end",strlen("flag_assign_end"),BUF_LEN-used,used);
    local_assignment_flag = 0;
  }
  grep_flag = 0;
  return used;
}
int visit_command_string_internal(char * buf,COMMAND *command,int used,int * feature)
{
  if (command->flags & CMD_TIME_PIPELINE)
    {
      used = visit_no_change(buf,"time",used,feature);
      if (command->flags & CMD_TIME_POSIX)
        used = visit_no_change(buf,"-p",used,feature);
    }
  if (command->flags & CMD_INVERT_RETURN)
  {
    	 used = visit_no_change(buf,"!",used,feature);
  }
  
  used = token_command(command,buf,used,feature);
    return used;
}
int visit_command_for(FOR_COM *for_com_content,char *buf,int used,int* feature)
{
  feature[num_of_for]++;
  used += put_word_in_buf(buf+used,"for",strlen("for"),BUF_LEN-used,used);
  used =  visit_word(buf,for_com_content->name,used,feature);
  add_paramlist(for_com_content->name,&assign_param_list);
  feature[num_of_parameter]++;
  used += put_word_in_buf(buf+used,"in",strlen("in"),BUF_LEN-used,used);
  used =  visit_word_list(buf,for_com_content->map_list,used,feature," ");
  used += put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used += put_word_in_buf(buf+used,"do",strlen("do"),BUF_LEN-used,used);
  if (for_com_content->action){
      used = visit_command_string_internal(buf,for_com_content->action,used,feature);
  }
  return used;
}

int visit_command_case(CASE_COM *case_com_content,char *buf,int used,int* feature){
  feature[num_of_case]++;
  used += put_word_in_buf(buf+used,"case",strlen("case"),BUF_LEN-used,used);
  used = visit_word(buf,case_com_content->word,used,feature);
  used += put_word_in_buf(buf+used,"in",strlen("in"),BUF_LEN-used,used);
  PATTERN_LIST * clause = NULL;
  if (case_com_content->clauses)
  {
    clause = case_com_content->clauses;
    while(clause)
    {
      visit_word_list(buf,clause->patterns,used,feature,"|");
      used += put_word_in_buf(buf+used,")",strlen(")"),BUF_LEN-used,used);
      if (clause->action)
      {
        used = visit_command_string_internal(buf,clause->action,used,feature);
      }
      
      if (clause->flags & CASEPAT_FALLTHROUGH)
      {
        used += put_word_in_buf(buf+used,";&",strlen(";&"),BUF_LEN-used,used);
      }
      else if (clause->flags & CASEPAT_TESTNEXT)
      {
        used += put_word_in_buf(buf+used,";;&",strlen(";;&"),BUF_LEN-used,used);
      }
      else
      {
         used += put_word_in_buf(buf+used,";;",strlen(";;"),BUF_LEN-used,used);
      }
      clause = clause->next;
    }
    used += put_word_in_buf(buf+used,"esac",strlen("esac"),BUF_LEN-used,used);
  }
  return used;
}
int visit_until_or_while(WHILE_COM *while_com_content,char *buf,int used,int* feature,char *which)
{
  
  used += put_word_in_buf(buf+used,"while",strlen("while"),BUF_LEN-used,used);
  if (while_com_content->test)
    used = visit_command_string_internal(buf,while_com_content->test,used,feature);
  used += put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used += put_word_in_buf(buf+used,"do",strlen("do"),BUF_LEN-used,used);
  if (while_com_content->action)
    used = visit_command_string_internal(buf,while_com_content->test,used,feature);
  used += put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used += put_word_in_buf(buf+used,"done",strlen("done"),BUF_LEN-used,used);
  return used;
}
int visit_conmand_until(WHILE_COM *while_com_content,char *buf,int used,int* feature)
{
  feature[num_of_until]++;
    used = visit_until_or_while(while_com_content,buf,used,feature,"until");
    return used;
}
int visit_conmand_while(WHILE_COM *while_com_content,char *buf,int used,int* feature)
{
  feature[num_of_while]++;
    used = visit_until_or_while(while_com_content,buf,used,feature,"while");
  return used;
}
int visit_conmand_simple(SIMPLE_COM* simple_com_content,char *buf,int used, int * feature)
{
    used = visit_word_list(buf,simple_com_content->words,used,feature," ");
    if (simple_com_content->redirects)
    {
      used = visit_redirect(simple_com_content->redirects,buf,used,feature);
    }
    return used;
}
int visit_op_connector(char * buf,int op,int used,int * feature)
{
  char line[1024]={0};
  switch (op)
	    {
	    case '&':
        {
		      char* c = op;
          snprintf(line, sizeof(line), "op_%c",c);
          used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line));
          feature[num_of_operator]++;
	        break;
        }
	    case '|':
	      {
		      char* c = op;
          snprintf(line, sizeof(line), "pipe_%c",c);
          used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line));
          feature[num_of_pipe]++;
	        break;
        }
	    case AND_AND:
	      {
		      char* c = "&&";
          snprintf(line, sizeof(line), "op_%s",c);
          used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line));
          feature[num_of_operator]++;
	        break;
        }

	    case OR_OR:
	      {
		      char* c = "||";
          snprintf(line, sizeof(line), "op_%s",c);
          used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
          memset(line,0,sizeof(line));
          feature[num_of_operator]++;
	        break;
        }


	    case ';':
	      {         
          used += put_word_in_buf(buf+used,"op_;",strlen("op_;"),BUF_LEN-used,used);
	        break;
        }

	    default:
	      
	      break;
	    }
      return used;

}
int visit_conmand_connection(CONNECTION*connection_com_content,char*buf,int used,int *feature)
{
  feature[num_of_connection]++;
  if (connection_com_content->first)
  {
    used = visit_command_string_internal(buf,connection_com_content->first,used,feature);
  }
    
  used = visit_op_connector(buf,connection_com_content->connector,used,feature);
  
  if (connection_com_content->second)
  {
     used = visit_command_string_internal(buf,connection_com_content->second,used,feature);
  }

 
  return used;
}
int visit_conmand_group(GROUP_COM * group_com_content,char*buf,int used,int *feature)
{
  feature[num_of_group]++;
  used+=put_word_in_buf(buf+used,"{",strlen("{"),BUF_LEN-used,used);
  if (group_com_content->command)
    used = visit_command_string_internal (buf,group_com_content->command,used,feature);
  used+=put_word_in_buf(buf+used,"}",strlen("}"),BUF_LEN-used,used);
  return used;
}

int visit_conmand_arith(ARITH_COM * arith_com_content,char*buf,int used,int *feature)
{
  feature[num_of_arith]++;
  used+=put_word_in_buf(buf+used,"((",strlen("(("),BUF_LEN-used,used);
  used =visit_word_list(buf,arith_com_content->exp,used,feature," ");
  used+=put_word_in_buf(buf+used,"))",strlen("))"),BUF_LEN-used,used);
  return used;
}
int visit_cond_node(COND_COM * cond_com_content,char*buf,int used,int *feature)
{
  char line[1024]={0};
  if (cond_com_content->flags & CMD_INVERT_RETURN)
    used += put_word_in_buf(buf+used,"!",strlen("!"),BUF_LEN-used,used);

  if (cond_com_content->type == COND_EXPR)
    {
      used += put_word_in_buf(buf+used,"(",strlen("("),BUF_LEN-used,used);
      used = visit_cond_node (cond_com_content->left,buf,used,feature);
      used += put_word_in_buf(buf+used,")",strlen(")"),BUF_LEN-used,used);
    }
  else if (cond_com_content->type == COND_AND)
    {
      used = visit_cond_node (cond_com_content->left,buf,used,feature);
      char* c = "&&";
      snprintf(line, sizeof(line), "op_%s",c);
      used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      feature[num_of_operator]++;
      used = visit_cond_node (cond_com_content->right,buf,used,feature);
    }
  else if (cond_com_content->type == COND_OR)
    {
      used = visit_cond_node (cond_com_content->left,buf,used,feature);
      char* c = "||";
      snprintf(line, sizeof(line), "op_%s",c);
      used += put_word_in_buf(buf+used,line,strlen(line),BUF_LEN-used,used);
      memset(line,0,sizeof(line));
      feature[num_of_operator]++;
      used = visit_cond_node (cond_com_content->right,buf,used,feature);
    }
  else if (cond_com_content->type == COND_UNARY)
    {
      used = visit_word(buf,cond_com_content->op,used,feature);
      
      used = visit_cond_node (cond_com_content->left,buf,used,feature);
    }
  else if (cond_com_content->type == COND_BINARY)
    {
      used = visit_cond_node (cond_com_content->left,buf,used,feature);
      used = visit_word(buf,cond_com_content->op,used,feature);
      used = visit_cond_node (cond_com_content->right,buf,used,feature);
    }
  else if (cond_com_content->type == COND_TERM)
    {
      used = visit_word(buf,cond_com_content->op,used,feature);		/* need to add quoting here */
    }
  return used;
}
int visit_conmand_cond(COND_COM * cond_com_content,char*buf,int used,int *feature)
{
  feature[num_of_cond]++;
  used+=put_word_in_buf(buf+used,"[[",strlen("[["),BUF_LEN-used,used);
  used = visit_cond_node(cond_com_content,buf,used,feature);
  used+=put_word_in_buf(buf+used,"]]",strlen("]]"),BUF_LEN-used,used);
  return used;
}
int visit_conmand_arithfor(ARITH_FOR_COM * arith_for_com_content,char*buf,int used,int *feature)
{
  feature[num_of_arith_for]++;
  used+= put_word_in_buf(buf+used,"for ((",strlen("for (("),BUF_LEN-used,used);
  used = visit_word_list(buf,arith_for_com_content->init,used,feature," ");
  used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  visit_word_list (buf,arith_for_com_content->test,used,feature," ");
  used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  visit_word_list (buf,arith_for_com_content->step,used,feature," ");
  used+= put_word_in_buf(buf+used,"))",strlen("))"),BUF_LEN-used,used);
  used+= put_word_in_buf(buf+used,"do",strlen("do"),BUF_LEN-used,used);
  if (arith_for_com_content->action)
  {
     used = visit_command_string_internal (buf,arith_for_com_content->action,used,feature);
  }
   
  used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used+= put_word_in_buf(buf+used,"done",strlen("done"),BUF_LEN-used,used);
  return used;
}
int visit_conmand_subshell(SUBSHELL_COM * subshell_com_content,char*buf,int used,int *feature)
{
  feature[num_of_subshell]++;
  used+= put_word_in_buf(buf+used,"(",strlen("("),BUF_LEN-used,used);
  if (subshell_com_content->command)
  {
    used = visit_command_string_internal (buf,subshell_com_content->command,used,feature);
  }
    
  used+= put_word_in_buf(buf+used,")",strlen(")"),BUF_LEN-used,used);
  return used;
}
int visit_conmand_coproc(COPROC_COM * coproc_com_content,char*buf,int used,int *feature)
{
  feature[num_of_coproc]++;
  used+= put_word_in_buf(buf+used,"coproc",strlen("coproc"),BUF_LEN-used,used);
  used+= put_word_in_buf(buf+used,coproc_com_content->name,strlen(coproc_com_content->name),BUF_LEN-used,used);
  if (coproc_com_content->command)
  {
    	used = visit_command_string_internal (buf,coproc_com_content->command,used,feature);
  }

  return used;
}
int visit_conmand_functiondef(FUNCTION_DEF * functiondef_com_content,char*buf,int used,int *feature)
{
  COMMAND *cmdcopy;
  REDIRECT *func_redirects;
  feature[num_of_function]++;
  func_redirects = NULL;
  /* When in posix mode, print functions as posix specifies them. */
  if (posixly_correct == 0)
  {
    used+= put_word_in_buf(buf+used,"function",strlen("function"),BUF_LEN-used,used);
    used+= put_word_in_buf(buf+used,functiondef_com_content->name->word,strlen(functiondef_com_content->name->word),BUF_LEN-used,used);
    used+= put_word_in_buf(buf+used,"()",strlen("()"),BUF_LEN-used,used);
  }
  else
  {
    used+= put_word_in_buf(buf+used,functiondef_com_content->name->word,strlen(functiondef_com_content->name->word),BUF_LEN-used,used);
    used+= put_word_in_buf(buf+used,"()",strlen("()"),BUF_LEN-used,used);
  }
 


  used+= put_word_in_buf(buf+used,"{",strlen("{"),BUF_LEN-used,used);

  

  cmdcopy =  functiondef_com_content->command;
  if (cmdcopy->type == cm_group)
    {
      func_redirects = cmdcopy->redirects;
      cmdcopy->redirects = (REDIRECT *)NULL;
    }
    if (cmdcopy->value.Group->command||cmdcopy)
    {
        used = visit_command_string_internal (buf,
                cmdcopy->type == cm_group? cmdcopy->value.Group->command: cmdcopy,
                used,feature);
    }
    

  if (func_redirects)
    { /* { */
      used+= put_word_in_buf(buf+used,"}",strlen("}"),BUF_LEN-used,used);      
      used = visit_redirect(func_redirects,buf,used,feature);
      cmdcopy->redirects = func_redirects;
    }
  else
  {
    used+= put_word_in_buf(buf+used,"}",strlen("}"),BUF_LEN-used,used);
  }
  return used;
  
}
int visit_conmand_if(IF_COM  * if_com_content,char*buf,int used,int *feature)
{
  used += put_word_in_buf(buf+used,"if",strlen("if"),BUF_LEN-used,used);
  if (if_com_content->test)
  {
    visit_command_string_internal(buf,if_com_content->test,used,feature);
  }
  
  used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used+= put_word_in_buf(buf+used,"then",strlen("then"),BUF_LEN-used,used);

  if (if_com_content->true_case){
    visit_command_string_internal(buf,if_com_content->true_case,used,feature);
  }
  
  if (if_com_content->false_case)
    {
      used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
      used+= put_word_in_buf(buf+used,"else",strlen("else"),BUF_LEN-used,used);
    
     visit_command_string_internal(buf,if_com_content->false_case,used,feature);
    }
  used+= put_word_in_buf(buf+used,";",strlen(";"),BUF_LEN-used,used);
  used+= put_word_in_buf(buf+used,"fi",strlen("fi"),BUF_LEN-used,used);
  return used;
}

int token_command(COMMAND*command,char* buf,int used,int *feature){
  feature[num_of_command]++;
    switch(command->type){
    case cm_for:
        used = visit_command_for(command->value.For,buf,used,feature);
        break;
    case cm_case:
       
        
        used = visit_command_case(command->value.Case,buf,used,feature);
        break;
    case cm_while:
       
        
        used = visit_conmand_while(command->value.While,buf,used,feature);
        break;
    case cm_simple:
       
        
        used = visit_conmand_simple(command->value.Simple,buf,used,feature);
        break;
    case cm_connection:
        used = visit_conmand_connection(command->value.Connection,buf,used,feature);
        break;
    case cm_until:
       
        
        used = visit_conmand_until(command->value.While,buf,used,feature);
        break;
    case cm_group:
       
        
       used = visit_conmand_group(command->value.Group,buf,used,feature);
        break;
    case cm_arith:
       
       
        used = visit_conmand_arith(command->value.Arith,buf,used,feature);   
        break;
    case cm_cond:
       
        
        used = visit_conmand_cond(command->value.Cond,buf,used,feature);
        break;
    case cm_arith_for:
       
        
        used = visit_conmand_arithfor(command->value.ArithFor,buf,used,feature);
        break;
    case cm_subshell:
       
        
        used = visit_conmand_subshell(command->value.Subshell,buf,used,feature);
        break;
    case cm_coproc:
       
        used = visit_conmand_coproc(command->value.Coproc,buf,used,feature);
        
        break;
    case cm_function_def:
       
        used = visit_conmand_functiondef(command->value.Function_def,buf,used,feature);
        
        break;
    case cm_if:
       
        used = visit_conmand_if(command->value.If,buf,used,feature);
        
        break;
    default:
        break;
    }
    if (command->redirects){
        used = visit_redirect(command->redirects,buf,used,feature);
    }
    return used;
}
