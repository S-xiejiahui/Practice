
/**
 * @file arp_check_ip.c
 * @author your name (you@domain.com)
 * @brief 1、免费ARP--用于检测IP地址是否冲突.
 * @brief 2、ARP查询--用于查询当前 IP 与哪个 MAC 绑定
 * @version 0.1
 * @date 2022-04-13
 *
 * @copyright Copyright (c) 2022
 *
 * @difference：
 *     1、免费ARP：源 IP、目的 IP 是同一个，
 *                源 MAC 是本身、目的 MAC 是广播
 *     2、ARP查询：源 IP 是本身、目的 IP 是要查询的 IP
 *                源 MAC 是本身、目的 MAC 可以是广播、也可以是单播(向单个MAC查询)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <netinet/ip.h>

#define FAILURE -1
#define SUCCESS 0
#define MAC_ADDR_LEN 6
#define IP_ADDR_LEN 4

static char *g_ifname = "ens33";
unsigned char src_ip[IP_ADDR_LEN] = {192, 168, 146, 128};                       // 要检测的主机IP地址
unsigned char src_mac[MAC_ADDR_LEN] = {0x00, 0x0c, 0x29, 0x6c, 0x33, 0xd9};     // 要检测的主机的MAC地址
unsigned char dst_ip[IP_ADDR_LEN] = {192, 168, 146, 129};                       // 目标IP地址
unsigned char dst_mac[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};     // ARP广播地址
unsigned char inquire_mac[MAC_ADDR_LEN] = {0x00, 0x0c, 0x29, 0x6c, 0x33, 0xd9}; // 要检测 MAC 是否拥有该 IP

int send_inquire_arp(int sockfd, struct sockaddr_ll *peer_addr);
int send_free_arp(int sockfd, struct sockaddr_ll *peer_addr);
int recv_free_arp(int sockfd, struct sockaddr_ll *peer_addr);

/* ARP封装包 */
typedef struct _tagARP_PACKET
{
    struct ether_header eh;
    struct ether_arp arp;
} ARP_PACKET_OBJ, *ARP_PACKET_HANDLE;

/**
 * @brief 主函数：发送免费ARP，用于检测IP冲突
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    int sockfd;
    int rtval = -1;
    struct sockaddr_ll peer_addr;
    //创建socket
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0)
    {
        fprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sll_family = AF_PACKET;
    struct ifreq req;
    bzero(&req, sizeof(struct ifreq));
    strcpy(req.ifr_name, g_ifname);
    if (ioctl(sockfd, SIOCGIFINDEX, &req) != 0)
    {
        perror("ioctl()");
    }
    peer_addr.sll_ifindex = req.ifr_ifindex;
    peer_addr.sll_protocol = htons(ETH_P_ARP);
    // peer_addr.sll_family = AF_PACKET;
    while (1)
    {
        rtval = send_free_arp(sockfd, &peer_addr);
        if (FAILURE == rtval)
        {
            fprintf(stderr, "Send arp socket failed: %s\n", strerror(errno));
        }
        rtval = recv_free_arp(sockfd, &peer_addr);
        if (rtval == SUCCESS)
        {
            printf("Get packet from peer and IP conflicts!\n");
        }
        else if (rtval == FAILURE)
        {
            fprintf(stderr, "Recv arp IP not conflicts: %s\n", strerror(errno));
        }
        else
        {
            fprintf(stderr, "Recv arp socket failed: %s\n", strerror(errno));
        }
        // sleep(1);
    }
    return 0;
}

/**
 * @brief 填充ARP数据包报文并发送出去
 *
 * @param sockfd 创建的socket描述符
 * @param peer_addr 对端的IP信息
 * @return int 成功: SUCCESS, 失败: FAILURE
 */
int send_free_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;
    memset(&frame, 0x00, sizeof(ARP_PACKET_OBJ));

    //填充以太网头部
    memcpy(frame.eh.ether_dhost, dst_mac, MAC_ADDR_LEN); // 目的MAC地址
    memcpy(frame.eh.ether_shost, src_mac, MAC_ADDR_LEN); // 源MAC地址
    frame.eh.ether_type = htons(ETH_P_ARP);              // 协议

    //填充ARP报文头部
    frame.arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);    // 硬件类型
    frame.arp.ea_hdr.ar_pro = htons(ETHERTYPE_IP);    // 协议类型 ETHERTYPE_IP | ETH_P_IP
    frame.arp.ea_hdr.ar_hln = MAC_ADDR_LEN;           // 硬件地址长度
    frame.arp.ea_hdr.ar_pln = IP_ADDR_LEN;            // 协议地址长度
    frame.arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);    // ARP请求操作
    memcpy(frame.arp.arp_sha, src_mac, MAC_ADDR_LEN); // 源MAC地址
    memcpy(frame.arp.arp_spa, src_ip, IP_ADDR_LEN);   // 源IP地址
    memcpy(frame.arp.arp_tha, dst_mac, MAC_ADDR_LEN); // 目的MAC地址
    memcpy(frame.arp.arp_tpa, src_ip, IP_ADDR_LEN);   // 目的IP地址

    rtval = sendto(sockfd, &frame, sizeof(ARP_PACKET_OBJ), 0, (struct sockaddr *)peer_addr, sizeof(struct sockaddr_ll));
    if (rtval < 0)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * @brief 接收ARP回复数据报文并判断是不是对免费ARP的回复
 *        若是对免费arp请求的回复则返回:SUCCESS
 * @param sockfd 创建的socket描述符
 * @param peer_addr 对端的IP信息
 * @return int 成功: SUCCESS, 失败: FAILURE;
 */
int recv_free_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;

    memset(&frame, 0, sizeof(ARP_PACKET_OBJ));
    rtval = recvfrom(sockfd, &frame, sizeof(frame), 0, NULL, NULL);
    //判断是否接收到数据并且是否为回应包
    if (htons(ARPOP_REPLY) == frame.arp.ea_hdr.ar_op && rtval > 0)
    {
        //判断源地址是否为冲突的IP地址
        if (memcmp(frame.arp.arp_spa, src_ip, IP_ADDR_LEN) == 0)
        {
            fprintf(stdout, "IP address is common~\n");
            return SUCCESS;
        }
    }

    return FAILURE;
}

/**
 * @brief 发送 arp 查询当前 IP 被分配的 MAC 地址
 *
 * @param sockfd
 * @param peer_addr
 * @return int
 */
int send_inquire_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;
    memset(&frame, 0x00, sizeof(ARP_PACKET_OBJ));

    //填充以太网头部
    memcpy(frame.eh.ether_dhost, dst_mac, MAC_ADDR_LEN); // 目的MAC地址
    memcpy(frame.eh.ether_shost, src_mac, MAC_ADDR_LEN); // 源MAC地址
    frame.eh.ether_type = htons(ETH_P_ARP);              // 协议

    //填充ARP报文头部
    frame.arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);    // 硬件类型
    frame.arp.ea_hdr.ar_pro = htons(ETHERTYPE_IP);    // 协议类型 ETHERTYPE_IP | ETH_P_IP
    frame.arp.ea_hdr.ar_hln = MAC_ADDR_LEN;           // 硬件地址长度
    frame.arp.ea_hdr.ar_pln = IP_ADDR_LEN;            // 协议地址长度
    frame.arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);    // ARP请求操作
    memcpy(frame.arp.arp_sha, src_mac, MAC_ADDR_LEN); // 源MAC地址
    memcpy(frame.arp.arp_spa, src_ip, IP_ADDR_LEN);   // 源IP地址
    memset(frame.arp.arp_tha, 0x00, MAC_ADDR_LEN);    // 目的MAC地址
    memcpy(frame.arp.arp_tpa, dst_ip, IP_ADDR_LEN);   // 目的IP地址

    rtval = sendto(sockfd, &frame, sizeof(ARP_PACKET_OBJ), 0, (struct sockaddr *)peer_addr, sizeof(struct sockaddr_ll));
    if (rtval < 0)
    {
        return FAILURE;
    }
    return SUCCESS;
}

int recv_inquire_arp(int sockfd, struct sockaddr_ll *peer_addr)
{
    int rtval;
    ARP_PACKET_OBJ frame;

    memset(&frame, 0, sizeof(ARP_PACKET_OBJ));
    rtval = recvfrom(sockfd, &frame, sizeof(frame), 0, NULL, NULL);
    //判断是否接收到数据并且是否为回应包
    if (htons(ARPOP_REPLY) == frame.arp.ea_hdr.ar_op && rtval > 0)
    {
        //判断源地址是否为冲突的IP地址
        if (memcmp(frame.arp.arp_tha, inquire_mac, MAC_ADDR_LEN) == 0)
        {
            fprintf(stdout, "IP used by Mac(%s)\n", inquire_mac);
            return SUCCESS;
        }
    }

    return FAILURE;
}