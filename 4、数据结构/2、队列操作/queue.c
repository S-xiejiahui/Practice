#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define _Num_toSting(str) #str
#define Num_toString(str)  _Num_toSting(str)

#define _PASTE_STRING(str1, str2) (str1##str2)
#define PASTE_STRING(str1, str2) _PASTE_STRING(str1, str2)

enum data_type {
    TYPE_NUMBER,    // 数据类型：整型
    TYPE_STRING,    // 数据类型：字符串
};

union data {
    int number;     // 共用体：整型
    char *string;   // 公用体：字符串
};

struct queue_node {
    union data data;            // 数据
    enum data_type type;        // 数据类型
    struct queue_node *next;    // next 指针
    struct queue_node *prev;    // prev 指针
};

struct queue_head {
    int node_number;                 // 结点个数
    struct queue_node *queue_head;   // 第一个结点
    struct queue_node *queue_tail;   // 最后一个结点
};

typedef void (*struct_function_void)(struct queue_head*, void *);
typedef int  (*struct_function_int)(struct queue_head*, void *);

struct queue_class {
    struct queue_head head;

    struct_function_int push;
    struct_function_int pop;
    struct_function_int renew;
    struct_function_int find;
    struct_function_int print;
    struct_function_int clean;
};

int push_data_into_queue(struct queue_head *head, void *data);
int pop_data_from_queue(struct queue_head *head, void *data);
int renew_data_into_queue(struct queue_head *head, void *data);
int find_data_from_queue(struct queue_head *head, void *data);
int print_data_from_queue(struct queue_head *head, void *data);
int clean_queue(struct queue_head *head, void *data);

/**
 * @brief 
 * 
 * @param queue 
 * @return int 
 */
int init_queue_struct(struct queue_class *queue)
{
    if (NULL == queue)
    {
        return -1;
    }
    queue->head.node_number = 0;
    queue->head.queue_head = NULL;
    queue->head.queue_tail = NULL;

    queue->push = push_data_into_queue;
    queue->pop  = pop_data_from_queue;
    queue->renew = renew_data_into_queue;
    queue->find = find_data_from_queue;
    queue->print = print_data_from_queue;
    queue->clean  = clean_queue;
    return 0;
}

/**
 * @brief 
 * 
 * @param head 
 * @param data 
 * @return int 
 */
int push_data_into_queue(struct queue_head *head, void *data)
{
    if (NULL == head)
    {
        return -1;
    }

    struct queue_node *new_node = (struct queue_node *)malloc(sizeof(struct queue_node));
    new_node->next = NULL;
    new_node->prev = NULL;
    if (0 <= *(char *)data && *(char *)data <= 9)
    {
        new_node->type = TYPE_NUMBER;
    }
    
    
    return 0;
}

int pop_data_from_queue(struct queue_head *head, void *data)
{

}
int renew_data_into_queue(struct queue_head *head, void *data)
{

}
int find_data_from_queue(struct queue_head *head, void *data)
{

}
int print_data_from_queue(struct queue_head *head, void *data)
{

}
int clean_queue(struct queue_head *head, void *data)
{

}

#define AAA(x) #x

int main(int argc, char const *argv[])
{
    struct queue_class queue;
    init_queue_struct(&queue);

    char *p = "1234";
    
    int a = -257687646;

    void *b = (void*)p;

    unsigned char *c = (char *)b;

    int d = c[3] << 24 | c[2] << 16 | c[1] << 8 | c[0];
    printf("a = %d, d =  %d\n", a, d);
    printf("%x, %x, %x, %x\n", *c, c[1], c[2], c[3]);

    printf("%s\n", AAA("123456"));
    printf("%s\n", Num_toString(454645));
    return 0;
}