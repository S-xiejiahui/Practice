/**
 * @file head_list.c
 * @author xiejiahui
 * @brief 用 C 语言结构体实现类似于 C++ 类的调用
 * @version 0.2
 * @date 2022-05-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RECODE_INFO_LEN 64

struct list_info
{
    char record_info[RECODE_INFO_LEN]; // 记录的信息
    struct list_info *next;            // 指向结点的下一个结点
    struct list_info *prev;            // 指向结点的上一个结点
};

typedef struct
{
    int number;              // 链表结点个数
    struct list_info *first; // 链表第一个结点
    struct list_info *last;  // 链表最后一个结点
} info_head;

typedef void (*struct_function_void)(info_head *); // 定义函数指针类型
typedef char *(*struct_function_char)(info_head *, char *);
typedef int (*struct_function_int)(info_head *, char *);

typedef struct
{
    info_head data;
    struct_function_int insert_first; // 方法：插入新结点到表头
    struct_function_int insert_last;  // 方法：插入新结点到表尾
    struct_function_int delete;       // 方法：删除链表结点
    struct_function_int find;         // 方法：查找链表结点
    struct_function_void print;       // 方法：打印链表结点信息
    struct_function_void clean;       // 方法：清空链表
} List;

int insert_info_to_list_first(info_head *head, char *recode_info);
int insert_info_to_list_last(info_head *head, char *recode_info);
int delete_info_to_list(info_head *head, char *recode_info);
int find_info_from_list(info_head *head, char *recode_info);
void print_info_from_list(info_head *head);
void clean_all_info_from_list(info_head *head);

/**
 * @brief 初始化双向链表、初始化函数指针
 *
 * @param list_head
 * @return int
 */
int init_list_struct(List *list_head)
{
    if (NULL == list_head)
    {
        return -1;
    }
    list_head->data.first = NULL;
    list_head->data.last = NULL;
    list_head->data.number = 0;

    list_head->insert_first = insert_info_to_list_first;
    list_head->insert_last = insert_info_to_list_last;
    list_head->delete = delete_info_to_list;
    list_head->find = find_info_from_list;
    list_head->print = print_info_from_list;
    list_head->clean = clean_all_info_from_list;

    return 0;
}

/**
 * @brief 向链表的表头添加一个新的结点
 *
 * @param recode_info
 * @return int 成功返回 0，失败返回 -1
 */
int insert_info_to_list_first(info_head *head, char *recode_info)
{
    if (NULL == head || NULL == recode_info)
    {
        return -1;
    }

    struct list_info *new_node = (struct list_info *)malloc(sizeof(struct list_info));
    new_node->next = NULL;
    new_node->prev = NULL;
    snprintf(new_node->record_info, RECODE_INFO_LEN, "%s", recode_info);

    if (NULL == head->first) // 如果链表中还没有一个结点
    {
        head->first = new_node;
        head->last = new_node;
        head->number = 1;
    }
    else // 头插法
    {
        new_node->next = head->first;
        head->first->prev = new_node;
        head->first = new_node;
        head->number++;
    }

    return 0;
}

/**
 * @brief 向链表的表尾添加一个新的结点
 *
 * @param recode_info
 * @return int 成功返回 0，失败返回 -1
 */
int insert_info_to_list_last(info_head *head, char *recode_info)
{
    if (NULL == head || NULL == recode_info)
    {
        return -1;
    }

    struct list_info *new_node = (struct list_info *)malloc(sizeof(struct list_info));
    new_node->next = NULL;
    new_node->prev = NULL;
    snprintf(new_node->record_info, RECODE_INFO_LEN, "%s", recode_info);

    if (NULL == head->first)
    {
        head->first = new_node;
        head->last = new_node;
        head->number = 1;
    }
    else
    {
        head->last->next = new_node;
        new_node->prev = head->last;
        head->last = new_node;
        head->number++;
    }

    return 0;
}

/**
 * @brief 删除 recode_info 信息对应的结点
 *
 * @param head
 * @param recode_info
 * @return int
 */
int delete_info_to_list(info_head *head, char *recode_info)
{
    if (NULL == head || NULL == head->first || NULL == recode_info)
    {
        return -1;
    }

    struct list_info *ptr = head->first;

    while (NULL != ptr)
    {
        if (0 == strcmp(ptr->record_info, recode_info))
        {
            if (head->first == ptr) // 如果删除的是第一个结点
            {
                head->first = head->first->next;
                if (head->last == ptr) // 既是第一个结点也是最后一个结点
                {
                    head->last = head->last->prev;
                }
            }
            else if (head->last == ptr) // 如果删除的是最后一个结点
            {
                head->last = head->last->prev;
                head->last->next = NULL;
            }
            else // 删除的是中间结点
            {
                ptr->prev->next = ptr->next;
                ptr->next->prev = ptr->prev;
            }
            head->number--;
            ptr->next = NULL;
            ptr->prev = NULL;
            free(ptr);
            return 1;
        }
        ptr = ptr->next;
    }

    return 0;
}

/**
 * @brief 查找链表某个结点是否保存有 recode_info 信息
 *
 * @param head
 * @param recode_info
 * @return int 存在返回 索引idx，不存在返回 -1
 */
int find_info_from_list(info_head *head, char *recode_info)
{
    if (NULL == head || NULL == head->first || NULL == recode_info)
    {
        return -1;
    }

    int i = 0;
    struct list_info *ptr = head->first;

    while (NULL != ptr)
    {
        if (0 == strcmp(ptr->record_info, recode_info))
        {
            return i;
        }
        i++;
        ptr = ptr->next;
    }
    return -1;
}

/**
 * @brief 打印链表每个结点的信息
 *
 * @param head
 */
void print_info_from_list(info_head *head)
{
    if (NULL == head || NULL == head->first)
    {
        return;
    }

    int i = 0;
    struct list_info *ptr = head->first;

    printf("---------- first -> last ----------\n");
    while (NULL != ptr)
    {
        printf("[%d]: %s\n", i++, ptr->record_info);
        ptr = ptr->next;
    }
#if 0
    printf("---------- last -> first ----------\n");
    ptr = head->last;
    while (NULL != ptr)
    {
        printf("[%d]: %s\n", i++, ptr->record_info);
        ptr = ptr->prev;
    }
#endif
    return;
}

/**
 * @brief 清空链表中所以结点保存的信息
 *
 * @param head
 */
void clean_all_info_from_list(info_head *head)
{
    if (NULL == head || NULL == head->first)
    {
        return;
    }

    struct list_info *tmp = NULL;
    struct list_info *ptr = head->first;
    while (NULL != ptr)
    {
        if (NULL != ptr->next)
        {
            tmp = ptr;
            ptr = ptr->next;
            tmp->next = NULL;
            ptr->prev = NULL;
            free(tmp);
        }
        else
        {
            ptr->prev = NULL;
            free(ptr);
            break;
        }
    }
    head->first = NULL;
    head->last = NULL;
    head->number = 0;
    return;
}

int main(int argc, char const *argv[])
{
    List test;
    init_list_struct(&test);

// Test 01
    test.insert_first(&test.data, "123");
    test.insert_first(&test.data, "456");

    test.print(&test.data);
    test.clean(&test.data);

// Test 02
    test.insert_last(&test.data, "789");

    test.clean(&test.data);
    test.print(&test.data);

// Test 03
    test.insert_first(&test.data, "123");
    test.insert_first(&test.data, "789");
    test.insert_first(&test.data, "456");
    test.print(&test.data);
    test.delete(&test.data, "789");
    test.delete(&test.data, "123");

    test.print(&test.data);
    printf("[find]: id = %d\n", test.find(&test.data, "4567"));

    return 0;
}
