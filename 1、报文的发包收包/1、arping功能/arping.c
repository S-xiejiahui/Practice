/**
 * @file arping.c
 * @author XJH
 * @brief 
 *     arping命令能够测试一个ip地址是否是在网络上已经被使用，
 *     并能够获取更多设备信息，功能类似于ping。 
 *     由于arping命令基于ARP广播机制，
 *     所以arping命令只能测试同一网段或子网的网络主机的连通性，
 *     ping命令则是基于ICMP协议，是可以路由的，
 *     所以使用ping命令可以测试任意网段的主机网络连通性。
 * @version 0.1
 * @date 2022-04-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define src_addr "192.168.146.128"
#define device "ens33"
#define fill_buf "aaaaaaaaaaaa"

static int socket_id = -1;
static int send_count = 0;
static int recv_count = 0;
static char *target = src_addr;

struct in_addr src, dst;
struct sockaddr_ll me, he;
struct timeval send_time, recv_time;

struct in_addr get_src_ip(char *devices)
{
    struct sockaddr_in saddr;
    int sock_id = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_id < 0)
    {
        perror("socket");
        exit(2);
    }

    if (NULL == devices || setsockopt(sock_id, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device) + 1) == -1)
    {
        perror("WARNING: interface is ignored");
    }

    int alen = sizeof(saddr);
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_port = htons(0x1000);
    saddr.sin_family = AF_INET;

    if (connect(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) == -1)
    {
        perror("connect");
        exit(2);
    }

    if (getsockname(sock_id, (struct sockaddr *)&saddr, &alen) == -1)
    {
        perror("getsockname");
        exit(2);
    }
    close(sock_id);
    return saddr.sin_addr;
}

/**
 * @brief 
 * 
 * @param if_dev 
 * @param ss 
 * @return int 
 */
int check_device(char *if_dev, int ss)
{
    int ifindex = -1;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, if_dev, IFNAMSIZ - 1);

    if (ioctl(ss, SIOCGIFINDEX, &ifr) < 0)
    {
        fprintf(stderr, "arping: unknown iface %s\n", if_dev);
        exit(2);
    }

    ifindex = ifr.ifr_ifindex;
    if (ioctl(ss, SIOCGIFFLAGS, (char *)&ifr))
    {
        perror("ioctl(SIOCGIFFLAGS)");
        exit(2);
    }

    if (!(ifr.ifr_flags & IFF_UP))
    {
        printf("Interface \"%s\" is down\n", if_dev);
        exit(2);
    }

    if (ifr.ifr_flags & (IFF_NOARP | IFF_LOOPBACK))
    {
        printf("Interface \"%s\" is not ARPable\n", if_dev);
        exit(2);
    }
    return ifindex;
}

/**
 * @brief 
 * 
 * @return int 
 */
int socket_init()
{
    int s, s_errno;
    s = socket(PF_PACKET, SOCK_DGRAM, 0);
    s_errno = errno;
    me.sll_family = AF_PACKET;
    me.sll_ifindex = check_device(device, s);
    me.sll_protocol = htons(ETH_P_ARP);

    if (bind(s, (struct sockaddr *)&me, sizeof(me)) == -1)
    {
        perror("bind");
        exit(2);
    }

    int alen = sizeof(me);
    if (getsockname(s, (struct sockaddr *)&me, &alen) == -1)
    {
        perror("getsockname");
        exit(2);
    }

    if (me.sll_halen == 0)
    {
        printf("Interface \"%s\" is not ARPable (no ll address)\n", device);
        exit(2);
    }
    he = me;
    memset(he.sll_addr, -1, he.sll_halen); // set dmac addr FF:FF:FF:FF:FF:FF
    return s;
}

/**
 * @brief Create a pkt object
 * 
 * @param buf 
 * @param src 
 * @param dst 
 * @param FROM 
 * @param TO 
 * @return int 
 */
int create_pkt(unsigned char *buf, struct in_addr src, struct in_addr dst, struct sockaddr_ll *FROM, struct sockaddr_ll *TO)
{
    struct arphdr *ah = (struct arphdr *)buf;
    unsigned char *p = (unsigned char *)(ah + 1);
    ah->ar_hrd = htons(FROM->sll_hatype);
    if (ah->ar_hrd == htons(ARPHRD_FDDI))
        ah->ar_hrd = htons(ARPHRD_ETHER);
    ah->ar_pro = htons(ETH_P_IP);
    ah->ar_hln = FROM->sll_halen;
    ah->ar_pln = 4;
    ah->ar_op = htons(ARPOP_REQUEST);
    memcpy(p, &FROM->sll_addr, ah->ar_hln);
    p += FROM->sll_halen;
    memcpy(p, &src, 4);
    p += 4;
    memcpy(p, &TO->sll_addr, ah->ar_hln);
    p += ah->ar_hln;
    memcpy(p, &dst, 4);
    p += 4;
    memcpy(p, fill_buf, strlen(fill_buf));
    p += 12;
    return (p - buf);
}

/**
 * @brief 
 * 
 */
void send_pkt()
{
    unsigned char send_buf[256];
    int pkt_size = create_pkt(send_buf, src, dst, &me, &he);
    gettimeofday(&send_time, NULL);
    int cc = sendto(socket_id, send_buf, pkt_size, 0, (struct sockaddr *)&he, sizeof(he));
    if (cc == pkt_size)
        send_count++;
    alarm(1);
}

/**
 * @brief 
 * 
 * @param buf 
 * @param FROM 
 * @return int 
 */
int chk_recv_pkt(unsigned char *buf, struct sockaddr_ll *FROM)
{
    struct arphdr *ah = (struct arphdr *)buf;
    unsigned char *p = (unsigned char *)(ah + 1);
    struct in_addr src_ip, dst_ip;
    if (ah->ar_op != htons(ARPOP_REQUEST) && ah->ar_op != htons(ARPOP_REPLY))
        return 0;
    if (ah->ar_pro != htons(ETH_P_IP) || ah->ar_pln != 4 || ah->ar_hln != me.sll_halen)
        return 0;
    memcpy(&src_ip, p + ah->ar_hln, 4);
    memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);
    if (src_ip.s_addr != dst.s_addr || src.s_addr != dst_ip.s_addr)
        return 0;
    return (p - buf);
}

/**
 * @brief 
 * 
 * @param received 
 * @param dst 
 * @param msecs 
 * @param usecs 
 * @param from 
 */
void disp_info(int received, struct in_addr dst, int msecs, int usecs, struct sockaddr_ll from)
{
    printf("%03d ", received);
    printf("%s ", from.sll_pkttype == PACKET_HOST ? "Unicast" : "Broadcast");
    printf("%s from %s", "reply", inet_ntoa(dst));
    printf(" [%02X:%02X:%02X:%02X:%02X:%02X] ", from.sll_addr[0], from.sll_addr[1],
           from.sll_addr[2], from.sll_addr[3], from.sll_addr[4], from.sll_addr[5]);
    printf(" %ld.%ld ms\n", (long int)msecs, (long int)usecs);
    fflush(stdout);
}

/**
 * @brief 
 * 
 */
void finish()
{
    printf("\nSent %d ARP probe packet(s) \n", send_count);
    printf("Received %d response(s)", recv_count);
    printf("\n\n");
    fflush(stdout);
    exit(!recv_count);
}

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
    // 设置进程为 root 进程
    uid_t uid = getuid();
    setuid(uid);

    if (argc != 2)
    {
        printf("Usage: ./a.out + ipaddr\n");
        return -1;
    }

    // 获取输入的域名或 IP
    target = argv[1];
    if (inet_aton(target, &dst) != 1)
    {
        struct hostent *hp;
        hp = gethostbyname2(target, AF_INET);
        printf("\ntarget = %s \n", target);
        if (!hp)
        {
            fprintf(stderr, "arping: unknown host %s\n", target);
            exit(2);
        }
        memcpy(&dst, hp->h_addr, 4);
    }

    // 获取本地 IP
    src = get_src_ip(device);
    if (!src.s_addr)
    {
        fprintf(stderr, "arping: no source address in not-DAD mode\n");
        exit(2);
    }
    socket_id = socket_init();

    printf("\nARPING %s ", inet_ntoa(dst));
    printf("from %s %s\n\n", inet_ntoa(src), device ?: "");

    signal(SIGINT, finish);
    signal(SIGALRM, send_pkt);

    send_pkt();
    while (1)
    {
        struct sockaddr_ll from;
        int alen = sizeof(from);
        char recv_buf[0x1000];

        int recv_size = recvfrom(socket_id, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&from, &alen);
        if (recv_size < 0)
        {
            perror("arping: recvfrom");
            continue;
        }
        gettimeofday(&recv_time, NULL);

        if (chk_recv_pkt(recv_buf, &from) > 0)
        {
            memcpy(he.sll_addr, from.sll_addr, he.sll_halen);
            long usecs, msecs;
            if (recv_time.tv_sec)
            {
                usecs = (recv_time.tv_sec - send_time.tv_sec) * 1000000 + recv_time.tv_usec - send_time.tv_usec;
                msecs = (usecs + 500) / 1000;
                usecs -= msecs * 1000 - 500;
            }
            recv_count++;
            disp_info(recv_count, dst, msecs, usecs, from);
        }
    }
    return 0;
}