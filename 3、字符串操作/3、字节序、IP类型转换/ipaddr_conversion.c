#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/**
 * @brief 由 IP 地址和掩码，计算 IP 所在网段
 * 
 * @param ipaddr IP地址
 * @param netmask 子网掩码
 * @param output_segment 接收网段字符串的数组
 * @param output_size 数组大小
 */
void computing_network_segment(const char *ipaddr, const char *netmask, char *output_segment, int output_size)
{
    if (NULL == ipaddr || NULL == netmask || NULL == output_segment)
    {
        return;
    }

    struct in_addr addr;
    in_addr_t ip =  inet_addr(ipaddr);
    in_addr_t mask =  inet_addr(netmask);

    addr.s_addr = ip & mask;
    char *segment = inet_ntoa(addr);

    snprintf(output_segment, output_size, "%s", segment);
    return;
}

int main(int argc, char const *argv[])
{
    char segment[64] = {0};
    const char *ip = "192.168.1.1";
    const char *netmask = "255.255.0.0";

    if (argc == 3)
    {
        ip = argv[1];
        netmask = argv[2];
    }
    else
    {
        printf("Usage: ./a.out + ip + netmask\n\n");
    }

    computing_network_segment(ip, netmask, segment, sizeof(segment));
    printf("segment = %s\n", segment);
    return 0;
}
