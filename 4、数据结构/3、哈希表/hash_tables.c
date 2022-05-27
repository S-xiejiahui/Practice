/**
 * @file hash_tables.c
 * @author Xiejiahui
 * @brief 使用拉链法解决 HashMap 冲突
 * @version 0.1
 * @date 2022-05-27
 * 
 * @copyright myself(xjh)
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define VALUESTRINGMAX 128       // 最大字符串存储大小
#define HASHTABLESMAX 100        // 哈希表的大小

struct data_list
{
    char valuestring[VALUESTRINGMAX];
    struct data_list *next;
};

struct Hash_Table
{
    unsigned int data_count;
    struct data_list *head;
};

struct Hash_Table hash_tables[HASHTABLESMAX] = {0};

/**
 * @brief 计算自定义算法的哈希值
 *
 * @param key 输入的字符串
 * @return int 返回 100 以内的 哈希值作为数组下标
 */
static int hash_calculation(char *key)
{
    if (NULL == key)
    {
        return -1;
    }
    unsigned int key_sum = 0;
    char *ptr = key;
    while (*ptr != '\0')
    {
        key_sum += *ptr % 10;
        ptr++;
    }

    return key_sum % 100;
}

/**
 * @brief 向哈希表中添加一个新的字符串
 *
 * @param key 输入的字符串
 * @return int
 */
static int hash_push(char *key)
{
    if (NULL == key)
    {
        return 0;
    }

    struct data_list *new_node = (struct data_list *)malloc(sizeof(struct data_list));
    new_node->next = NULL;
    snprintf(new_node->valuestring, VALUESTRINGMAX, "%s", key);

    int hash_value = hash_calculation(key);
    if (0 == hash_tables[hash_value].data_count || NULL == hash_tables[hash_value].head)
    {
        hash_tables[hash_value].head = new_node;
    }
    else
    {
        struct data_list *ptr = hash_tables[hash_value].head;
        while (ptr && ptr->next != NULL)
        {
            ptr = ptr->next;
        }

        ptr->next = new_node;
    }
    hash_tables[hash_value].data_count++;

    return 1;
}

/**
 * @brief 向哈希表中查找是否有该字符串
 *
 * @param key 待查找的字符串
 * @return int
 */
static int find_key_value(char *key)
{
    int hash_value = hash_calculation(key);
    if (hash_tables[hash_value].data_count > 0)
    {
        struct data_list *ptr = hash_tables[hash_value].head;
        while (ptr != NULL)
        {
            if (0 == strcmp(hash_tables[hash_value].head->valuestring, key))
            {
                return 1;
            }
            ptr = ptr->next;
        }
    }
    return 0;
}

/**
 * @brief 清空哈希表保存的所有信息
 *
 * @return int
 */
static int clean_hash_tables(void)
{
    int i = 0;
    struct data_list *del = NULL;
    struct data_list *ptr = NULL;
    for (i = 0; i < HASHTABLESMAX; i++)
    {
        if (NULL != hash_tables[i].head)
        {
            hash_tables[i].data_count = 0;
            ptr = hash_tables[i].head;
            while (ptr != NULL)
            {
                del = ptr;
                ptr = ptr->next;

                del->next = NULL;
                free(del);
            }
        }
    }
    return 1;
}

int main(int argc, char const *argv[])
{
    char *key = "118gdadfghfgjkfddsdf";
    hash_push(key);
    hash_push("123456");
    printf("%d\n", find_key_value(key));
    clean_hash_tables();
    printf("%d\n", find_key_value(key));
    printf("%d\n", hash_calculation(key));
    return 0;
}
