#ifndef _HASH_TOKEN_H_
#define _HASH_TOKEN_H_




#define KEYWORD_MAX_SIZE    64
#define KW_REPLACE_MAX_SIZE 64
#define KEYWORD_MAX_NUM    4096

/**
 * 计算hash表数组的大小
 */
#define hash_table_size(t) (sizeof((t)->arr) / sizeof((t)->arr[1]))
/**
 * hash表的元素
 */
typedef struct hash_keywords {
    /**
     * 标记
     * 标记此元素位置是否被占用
     */
    int flag;

    /**
     * 关键字
     */
    char key[KEYWORD_MAX_SIZE];

    /**
     * 关键字的长度
     */
    int key_size;

	/**
	  * 需要替换的keyword对应的g_keyword_replace_array索引
	*/ 
	char kw_rep[KW_REPLACE_MAX_SIZE];
}keywords_t;
/**
 * 为分词设计的简单hash表
 */
typedef struct tokenize_hash {
    /**
     * 执行插入时，重新计算下标的最大次数
     * 用来评估hash表的碰撞率
     */
    int insert_deepth;

    /**
     * 执行查找时，重新计算下标的最大次数
     * 用来评估hash表的碰撞率
     */
    int find_deepth;

    /**
     * 存放关键字的数组
     */
    keywords_t arr[KEYWORD_MAX_NUM * 2];
}token_hash_t;
typedef struct search_list{
    char * search_string;
    struct search_list *next;
}SEARCH_LIST;
/**
 * 初始化hash表
 */
#define token_hash_table_init(t) memset(t, 0, sizeof(token_hash_t))
#define token_search_table_init(t) memset(t, 0, sizeof(SEARCH_LIST))
extern int init_hash_words(int times);
extern token_hash_t g_token_env_hash;
extern SEARCH_LIST g_token_fuzzy_suffix_hash;
extern SEARCH_LIST g_token_key_path_hash;
extern SEARCH_LIST g_token_key_word_file_hash;
extern token_hash_t g_token_keywords_hash;
#define AI_LOOP_NUM 2

#define COMMAND_INJECTION_KEYWORD_FILE "./conf/sensitive_words.txt"
#define COMMAND_INJECTION_ENV_FILE "./conf/env_words.txt"
#define COMMAND_INJECTION_FUZZY_SUFFIX_FILE "./conf/fuzzy_suffix.txt"
#define COMMAND_INJECTION_KEY_PATH_FILE "./conf/key_path.txt"
#define COMMAND_INJECTION_KEY_WORD_FILE_FILE "./conf/key_word_file.txt"
// #define COMMAND_INJECTION_KEYWORD_FILE "/home/hxg/work/test_c/sensitive_words.txt"
// #define COMMAND_INJECTION_ENV_FILE "/home/hxg/work/test_c/env_words.txt"
// #define COMMAND_INJECTION_FUZZY_SUFFIX_FILE "/home/hxg/work/test_c/fuzzy_suffix.txt"
// #define COMMAND_INJECTION_KEY_PATH_FILE "/home/hxg/work/test_c/key_path.txt"
// #define COMMAND_INJECTION_KEY_WORD_FILE_FILE "/home/hxg/work/test_c/key_word_file.txt"
extern int visit_sens_information(char * buf, char * token,int used,int times);
extern int visit_env_var(char * buf, char * token,int used,int times);
#endif // _HASH_TOKEN_H_