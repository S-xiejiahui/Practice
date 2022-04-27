#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief 把 int 类型的数据转为 2 进制字符串
 * 
 * @param decimal_num 十进制数
 * @param binary_buf 返回二进制字符串数组
 * @param buf_len 数组长度
 */
void decimal_to_binary(int decimal_num, char *binary_buf, int buf_len)
{
    if (NULL == binary_buf)
    {
        return;
    }
    int i = 0;
    char buf[64] = {0}, *p = NULL;

    // 十进制转二进制
    for (i = 0; i < 32; i++)
    {
        if (decimal_num & (0x01 << i))
        {
            buf[31-i] = '1';
        }
        else
        {
            buf[31-i] = '0';
        }
    }

    // 去掉最前面多余的 0
    p = strchr(buf, '1');
    if (p != NULL)
    {
        snprintf(binary_buf, buf_len, "%s", p);
    }
    else
    {
        snprintf(binary_buf, buf_len, "0");
    }
    return;
}

int main(int argc, char const *argv[])
{
    int number = 65536;
    char buf[64] = {0};

    if (argc == 2)
    {
        number = atoi(argv[1]);
    }
    else
    {
        printf("Usage: ./a.out + number\n\n");
    }

    decimal_to_binary(number, buf, sizeof(buf));
    printf("%s\n", buf);

    return 0;
}
