#include <stdio.h>
#include <string.h>

/**
 * @brief Get the serial port output object
 * 
 * @param cmd 输入的命令
 * @param result 返回输出结果
 * @param result_len 数组长度
 * @return int 
 */
int get_serial_port_output(const char *cmd, char *result, unsigned int result_len)
{
    if (NULL == cmd || NULL == result)
    {
        return 0;
    }

    char output_buf[1024] = {0}, cmd_string[1024] = {0};

    snprintf(cmd_string, sizeof(cmd_string), "%s", cmd);
    memset(result, 0x00, result_len);

    FILE *ptr = popen(cmd_string, "r");
    if (NULL != ptr)
    {
        while (fgets(output_buf, 1024, ptr) != NULL)
        {
            if ('\n' == output_buf[strlen(output_buf) - 1])
                output_buf[strlen(output_buf) - 1] = ' ';
            if (strlen(output_buf) + strlen(result) >= result_len) // 防止越界
            {
                result[result_len - 1] = '\0';
                break;
            }
            strcat(result, output_buf);
        }
        result[strlen(result) - 1] = '\0';
        pclose(ptr);
        return 1;
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    char result[128] = {0};
    get_serial_port_output("ls", result, sizeof(result));
    printf("result = %s\n", result);
    return 0;
}
