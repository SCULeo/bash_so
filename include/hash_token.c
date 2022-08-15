#include "hash_token.h"
#include <stdio.h>
#include "compile_regex.h"
#include "bash_detect_demo.h"
#include<stdlib.h>
#include<string.h>
SEARCH_LIST * insert_search_list(const char *key, int key_size, SEARCH_LIST *hash)
{
    
    hash->search_string = (char *)malloc(sizeof(char)*key_size);
    strncpy(hash->search_string,key,key_size);
    hash->next = NULL;
    
}
int init_search_list(char *path,SEARCH_LIST *HEAD)
{
    FILE* fp = fopen(path, "r");
    char line[512] = {0};
    int i =0;
    SEARCH_LIST * temp;
    temp = HEAD;
    char key[KEYWORD_MAX_SIZE];
    char * p = NULL;
    if (NULL == fp)
    {
        return 0;
    }else
    {
         while( i<KEYWORD_MAX_NUM && !feof(fp) )
        {
			int length = 0;
			int length_have_rep = 0;
			memset(key, 0, sizeof(key));

            //按行读取文件
            p = fgets(key, KEYWORD_MAX_SIZE, fp);
            if(p == NULL)
                break;

            length = strlen(key);

            //放入hash表之前应去除换行符
            if(key[length-1] == '\n')
            {
                length-=1;
                if (key[length - 1] == '\r')
                {
                    length -= 1;
                }
            }
            
			temp->next =  (SEARCH_LIST * )malloc(sizeof(SEARCH_LIST));
			insert_search_list(key,length,temp->next); 
            temp = temp->next;		
            i++;
        }
        fclose(fp);
    }

    temp->next = NULL;
    return 1;

}
int find_in_search_list(char * string,SEARCH_LIST *HEAD)
{
    SEARCH_LIST * temp =NULL;
    temp  = HEAD->next;
    while(temp!=NULL)
    {
        if (strstr(string,temp->search_string))
        {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

int free_search_list(SEARCH_LIST *HEAD)
{
    SEARCH_LIST * temp =NULL;
    while(HEAD->next)
    {
        temp = HEAD->next;
        HEAD->next =temp->next;
        free(temp);
    }
    return 0;
}

/**
 * @brief 
 * 
 * @param path 
 * @param token_hash 
 * @return int 1 is success,0 is failed
 */
static int read_tokenize_word(char * path,token_hash_t * token_hash)
{
  int key_num;
  key_num = read_keywords_and_insert_puncs(path, token_hash);
	if (key_num >= 0)
	{
		printf("g_token_env_hash_use Found %d keywords.\n", key_num);
	}
  else
	{
		printf("Error occur when initial keywords hash table.\n");
		return 0;
	}
  return 1;
}
/**
 * 计算hash中的下标
 * 
 * 本函数采用两字节累计的方式来计算下标，通常该计算方式碰撞率较高，但由于本场景中进
 * 行key比较和下标修正相对比较简单，所以一定程度的碰撞率可以接受。如果要把本hash表
 * 挪做他用，该函数很可能将不再适用。
 * 
 * @param key
 *      用来计算下标的数据
 * 
 * @param key_size
 *      @p key 的长度
 * 
 * @hash
 *      hash表指针
 * 
 * @return
 *      返回下标值
 */
static unsigned int hash_get_index(const char *key, int key_size, token_hash_t *hash)
{
    unsigned int index = 0;
    unsigned int sum = 0;
    int short_num = key_size / 2;

    for(int i=0; i<short_num; i++)
    {
        sum += (((unsigned short)key[i]) << 8) + (unsigned short)key[i+1];
    }

    if(key_size % 2 != 0)
    {
        sum += key[key_size-1];
    }

    sum *= *key;
	sum += (sum >> 16);

	index = sum % hash_table_size(hash);
	return index;
}
/**
 * 将元素插入到hash表中
 * 
 * 插入时，先计算下标，如果下标处已经有元素，则使用key的第一个字节值来修正下标
 * 
 * @param key
 *      关键字元素的数据
 * 
 * @param key_size
 *      @p key 的长度
 * 
 * @param hash
 *      hash表指针
 * 
 * @return
 *      返回实际插入的下标
 */
static int hash_insert(const char *key, int key_size, token_hash_t *hash, char *kw_rep, int kw_rep_size)
{
	int index = hash_get_index(key, key_size, hash);
	int loop = 0;
	int table_size = hash_table_size(hash);
	while(loop < table_size)   //要防止hash表已满的情况
	{
		loop++;
		if(hash->arr[index].flag == 0)
		{
			hash->arr[index].flag = 1;
			strncpy(hash->arr[index].key, key, key_size);
      hash->arr[index].key_size = key_size;
			if(kw_rep)
			{
				strncpy(hash->arr[index].kw_rep,kw_rep,kw_rep_size);
				hash->arr[index].kw_rep[kw_rep_size] = '\0';
			}
			else
			{
				hash->arr[index].kw_rep[0] = '\0';
			}
			break;
		}
		else
		{
			//使用key的第一个字节修正index值
			index += *key;
			
			if(hash->insert_deepth < loop)
				hash->insert_deepth = loop;
			//防止index越界
			index %= table_size;
		}
	}
	return index;
}

/**
 * 从文件中读取关键字和额外的符号一起放入hash表
 * 
 * 关键字文件中，每行表示一个关键字
 * 
 * @param path
 *      关键字文件的路径。
 * 
 * @param hash
 *      hash表
 * 
 * @return
 *      成功返回插入hash表中关键字的个数，否则返回小于0的值
 */
int read_keywords_and_insert_puncs(char *path, token_hash_t *hash)
{
    FILE *fp = NULL;
    int i = 0;
    
    char key[KEYWORD_MAX_SIZE];
    char * p = NULL;
	  char *def_word_ptr = NULL;
	  char *kw_replace_ptr = NULL;
	  int kw_replace_len = 0;    
   

    //读取关键字文件
    fp = fopen(path, "r");
    if(fp)
    {
        while( i<KEYWORD_MAX_NUM && !feof(fp) )
        {
			int length = 0;
			int length_have_rep = 0;
			memset(key, 0, sizeof(key));

            //按行读取文件
            p = fgets(key, KEYWORD_MAX_SIZE, fp);
            if(p == NULL)
                break;

            length = strlen(key);

            //放入hash表之前应去除换行符
            if(key[length-1] == '\n')
            {
                length-=1;
                if (key[length - 1] == '\r')
                {
                    length -= 1;
                }
            }
			
				  hash_insert(key, length, hash, NULL, 0);
			
            i++;
        }
        fclose(fp);
    }
    else
    {
        printf("Failed to open xssql keywords file:%s\n", path);
    }


    return i;
}


/**
 * @brief 初始化分词hash表，共有5个分词表,其中两个是hash表，三个是链表用于查找信息
 * 
 */
int init_hash_words(int times)
{
  token_hash_t *g_token_env_hash_use;
  SEARCH_LIST *g_token_fuzzy_suffix_hash_use;
  SEARCH_LIST *g_token_key_path_hash_use;
  SEARCH_LIST *g_token_key_word_file_hash_use;
  token_hash_t *g_token_keywords_hash_use;

  int key_num = -1;

    if(times % AI_LOOP_NUM)
	{
    
	}
	else
	{
		g_token_env_hash_use = &g_token_env_hash;
    g_token_fuzzy_suffix_hash_use = &g_token_fuzzy_suffix_hash;
    g_token_key_path_hash_use = &g_token_key_path_hash;
    g_token_key_word_file_hash_use = &g_token_key_word_file_hash;
    g_token_keywords_hash_use = &g_token_keywords_hash;
	}
    token_hash_table_init(g_token_env_hash_use);
    token_search_table_init(g_token_fuzzy_suffix_hash_use);
    token_search_table_init(g_token_key_path_hash_use);
    token_search_table_init(g_token_key_word_file_hash_use);
    token_hash_table_init(g_token_keywords_hash_use);
  
  key_num = key_num &read_tokenize_word(COMMAND_INJECTION_ENV_FILE, g_token_env_hash_use);
	key_num = key_num &init_search_list(COMMAND_INJECTION_FUZZY_SUFFIX_FILE, g_token_fuzzy_suffix_hash_use);
  key_num = key_num &init_search_list(COMMAND_INJECTION_KEY_PATH_FILE, g_token_key_path_hash_use);
  key_num = key_num &init_search_list(COMMAND_INJECTION_KEY_WORD_FILE_FILE, g_token_key_word_file_hash_use);
  key_num = key_num &read_tokenize_word(COMMAND_INJECTION_KEYWORD_FILE, g_token_keywords_hash_use);
  if (!key_num)
  {
    return -1;
  }
  return 0;

}  



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
/**
 * 释放一个替换参数
 * 
 * 注意：本函数不释放替换参数自身，只释放其内部的部分，要释放参数本身，可以额外调用 @ref free() 来实现
 * 
 * @param param
 *      待释放的替换参数
 */
static inline void free_single_replace_param(struct replace_word_param *param)
{
    
    if (&param->reg)
      regfree(&param->reg); 
    
}
int init_regex(struct replace_word_param *params,const int size)
{
    int i = 0, ret = 0;

    for(i=0; i<size; i++)
    {
        //依次调用初始化接口
        ret = init_single_replace_param(params);
        if(ret != 0)
        {
            //需要对已经完成初始化的部分进行释放
            goto err;
        }
        params++;
    }
    return 0;
err:
    for(i-=1; i>=0; i--)
    {
        free_single_replace_param(params);
        params--;
    }

    return -1;
}


/**
 * 在hash表中查找
 * 
 * 进行查找时，计算index后，需要对比实际数据是否相同。如果不同，则需要对index进行修
 * 正，直到找到正确的数据
 * 
 * @param key
 *      待查找的关键字数据
 * 
 * @param key_size
 *      @p key 的大小
 * 
 * @param hash
 *      hash表指针
 * 
 * @return
 *      找到返回1，否则返回0值
 */
static int hash_find(const char *key, int key_size, token_hash_t *hash)
{
    int index = hash_get_index(key, key_size, hash);
    int loop = 0;
    int table_size = hash_table_size(hash);
    while(hash->arr[index].flag && loop < table_size)
    {
        loop++;
        if(key_size == hash->arr[index].key_size && strncmp(key, hash->arr[index].key, key_size) == 0 && hash->arr[index].key[key_size] == '\0')
        {
			
            return 1;
        }
        else
        {
            index += *key;
            if(hash->find_deepth < loop)
                hash->find_deepth = loop;

            index %= table_size;
        }
    }

    return 0;
}
/**
 * @brief 判断token这个字符串是否在已经列出的环境变量列表中，通过hash查找
 * 
 * @param buf 
 * @param token 
 * @param used 
 * @param times 
 * @return int 
 */
int visit_env_var(char * buf, char * token,int used,int times)
{
    token_hash_t *g_token_env_hash_use = NULL;
  if(times % AI_LOOP_NUM)
	{
    
	}
	else
	{
	g_token_env_hash_use = &g_token_env_hash;
   
	}

  if (hash_find(token,strlen(token),g_token_env_hash_use))
  {
    return 1;
  }
  
  return 0;

  
}
int visit_sens_information(char * buf, char * token,int used,int times)
{

  SEARCH_LIST *g_token_fuzzy_suffix_hash_use = NULL;
  SEARCH_LIST *g_token_key_path_hash_use = NULL;
  SEARCH_LIST *g_token_key_word_file_hash_use= NULL;
  if(times % AI_LOOP_NUM)
	{
    
	}
	else
	{

    g_token_fuzzy_suffix_hash_use = &g_token_fuzzy_suffix_hash;
    g_token_key_path_hash_use = &g_token_key_path_hash;
    g_token_key_word_file_hash_use = &g_token_key_word_file_hash;

	}

  if (find_in_search_list(token,g_token_key_path_hash_use))
  {
    used+=put_word_in_buf(buf+used,"sens_path",strlen("sens_path"),BUF_LEN-used,used);
  }else if (find_in_search_list(token,g_token_key_word_file_hash_use)) {
      used+=put_word_in_buf(buf,"sens_file",strlen("sens_file"),BUF_LEN-used,used);
  }else if (find_in_search_list(token,g_token_fuzzy_suffix_hash_use)) {
    used+=put_word_in_buf(buf,"sens_ext",strlen("sens_ext"),BUF_LEN-used,used);
  }

  return used;
}