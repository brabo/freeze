#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define ICMP_ECHO       8
#define INTV_DEFAULT    1
#define CNTFLAG         (1 << 0)

struct icmp_hdr {
    uint8_t type;
    uint8_t code;
    uint16_t chksum;
    uint16_t id;
    uint16_t seq;
};

struct ping {
    int sock;
    char *dest;
    uint8_t flags;
    int cnt;
    int max;
    int intv;
};

static char out[1024];
static char in[1024];

static void usage(char *name)
{
    printf("Usage: %s [-c count] [-i interval] destination\n",
            name);
}

static int socket_setup(void)
{
    int s;
    struct protoent *proto;

    if (!(proto = getprotobyname("icmp"))) {
        fprintf(stderr, "ping: unknown protocol icmp.\n");
        exit(1);
    }

    if ((s = socket(AF_INET, SOCK_DGRAM, proto->p_proto)) < 0) {
        perror("ping: socket");
        exit(2);
    }

    return s;
}

static unsigned short checksum(void *b, int len)
{   unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

static int mk_ping(uint8_t *buf)
{
    int len = 0;
    static int seq = 1;

    struct icmp_hdr *icmp = NULL;

    icmp = (struct icmp_hdr *)buf;

    icmp->type = ICMP_ECHO;
    icmp->code = htons(0);
    icmp->chksum = htons(0);
    icmp->id   = (unsigned short) htons(getpid());
    icmp->seq  = seq++;

    len += 8;

    gettimeofday((struct timeval *)&buf[8],
            (struct timezone *)NULL);

    len += 8;
    icmp->chksum = checksum((u_short *)icmp, len);

    return len;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_addr(struct ping *ping)
{
        struct addrinfo hints, *p;
        int rv;
        char s[INET6_ADDRSTRLEN];

        memset(&hints, 0, sizeof hints);
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        char port[] = "80";

        if ((rv = getaddrinfo(ping->dest, port, &hints, &p)) != 0) {
            fprintf(stderr, "ping: getaddrinfo: %s", gai_strerror(rv));
            exit(3);
        }

        if (p) {
            inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof (s));
            memcpy(ping->dest, s, (strlen(s) + 1));
        } else {
            fprintf(stderr, "ping: getaddrinfo: no go!");
        }

        freeaddrinfo(p); // all done with this structure

}

int print_hex(uint8_t *buf, int len)
{
    char *bugger = malloc(sizeof(uint8_t) * ((len * 2) + 3));
    char *ptr = bugger;
    uint8_t *ptr2 = buf;
    sprintf(ptr++, "[");

    for (int i = 0; i < len; i++) {
        sprintf(ptr, "%02X", *ptr2++);
        ptr += 2;
    }

    sprintf(ptr++, "]");
    printf("Received: %s\n\n", bugger);

    free(bugger);

    return 0;
}


static int pinger(struct ping *ping)
{
    int outlen, i;
    uint8_t out[1024];
    char *hostname = NULL;
    struct sockaddr_in dest = {};

    outlen = mk_ping(out);

    dest.sin_family = AF_INET;
    if (!inet_aton(ping->dest, &dest.sin_addr)) {
        get_addr(ping);
        inet_aton(ping->dest, &dest.sin_addr);
    }

    i = sendto(ping->sock, (char *)&out, outlen, 0, (struct sockaddr *)&dest,
        sizeof(dest));
    return 0;
}

static void tvsub(register struct timeval *out, register struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0) {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

static int parse_ping(int inlen, struct sockaddr_in dest)
{
    struct icmp_hdr *icmp;

    struct timeval tv, *tp;
    u_long triptime = 0;
    uint16_t ident = htons(getpid());

    gettimeofday(&tv, (struct timezone *)NULL);

    int hlen;
    struct iphdr *ip;

    static long tmin = 0;    /* minimum round trip time */
    static long tmax = LONG_MAX;       /* maximum round trip time */
    static u_long tsum;     /* sum of all times, for doing average */
    /* Check the IP header */
    ip = (struct iphdr *)in;
    hlen = ip->ihl << 2;


    /* Now the ICMP part */
    inlen -= hlen;
    icmp = (struct icmp_hdr *)(in + hlen);
    if (icmp->type == ICMP_ECHOREPLY) {
        if (icmp->id != ident) {
            return 1;           /* 'Twas not our ECHO */
        }
        tp = (struct timeval *)(in + hlen + sizeof (struct icmp_hdr));
        tvsub(&tv, tp);
        triptime = tv.tv_sec * 10000 + (tv.tv_usec / 100);
        tsum += triptime;
        if (triptime < tmin)
            tmin = triptime;
        if (triptime > tmax)
            tmax = triptime;
    }
    printf("%d bytes from %s: icmp_seq=%d", inlen,
        inet_ntoa(*(struct in_addr *)&dest.sin_addr.s_addr),
        icmp->seq);
    printf(" ttl=%d", ip->ttl);
    printf(" time=%ld.%ld ms\n", triptime/10,
        triptime%10);
    return 0;

}

static int listener(struct ping *ping)
{
    int i, inlen, maxin = 1024;
    struct sockaddr_in dest = {};

    while(2 > 1)
    {
        int fromlen = sizeof (dest);

        if ((inlen = recvfrom(ping->sock, (char *)in, maxin, 0,
            (struct sockaddr *)&dest, (socklen_t *)&fromlen)) < 0) {
            if (errno == EINTR) {
                perror("ping: recvfrom");
                break;
            }
            perror("ping: recvfrom");
            continue;
        } else {
            if (!parse_ping(inlen, dest)) {
                return 0;
            }
        }
    }
}

static int parse_opts(int argc, char **argv, struct ping *ping)
{
    char c;
    extern int optind, optopt;
    extern char *optarg;


    if (argc < 1) {
        fprintf(stderr, "Usage: ping [OPTION]... [-c count] [-i interval] destination\n");
        exit(1);
    }

    ping->flags = 0;
    ping->cnt = 0;
    ping->max = 0;
    ping->intv = INTV_DEFAULT;
    while ((c = getopt(argc, argv, "c:i:")) != -1) {
        switch (c) {
        case 'c':
            ping->flags |= CNTFLAG;
            ping->max = atoi(optarg);
            if (ping->max <= 0) {
                fprintf(stderr, "ping: bad number of packets to transmit.\n");
                exit(2);
            }
            break;
        case 'i':
            ping->intv = atoi(optarg);
            if (ping->intv <= 0) {
                fprintf(stderr, "ping: bad interval.\n");
                exit(2);
            }
            break;
        default:
            usage(argv[0]);
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if (!argc) {
        usage(argv[0]);
    }

    int len = strlen(argv[argc - 1]) + 1;
    ping->dest = malloc(64 * sizeof (char));
    memcpy(ping->dest, argv[argc - 1], len);
}

int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    struct ping *ping = malloc(sizeof (struct ping));
    parse_opts(argc, argv, ping);

    ping->sock = socket_setup();
    while (2 > 1) {
        pinger(ping);
        listener(ping);
        if (ping->max) {
            ping->cnt++;
            if (ping->cnt >= ping->max) {
                exit(0);
            }
        }
        sleep(ping->intv);
    }
    exit(0);
}
