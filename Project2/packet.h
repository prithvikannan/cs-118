#define max_file_size 100 * 1024 * 1024
#define packet_size 524
#define data_size 512
#define MSS 512
#define MAX_retran 20
#define MAX_SEQ_NUMBER 25600 
#define MAX_CONNECTIONS 10

#define NONE 0
#define FIN 1
#define SYN 2
#define ACK 4
#define ACK_FIN 5
#define SYN_ACK 6


struct header
{
    uint32_t seq;
    uint32_t ack;
    uint16_t id;
    uint16_t flags;
};

struct packet
{
    header pack_header;
    char data[data_size];
};

typedef struct header header;
typedef struct packet packet;

// network to host byte order transfer
void ntoh_reorder(packet &pack)
{
    pack.pack_header.ack = ntohl(pack.pack_header.ack);
    pack.pack_header.seq = ntohl(pack.pack_header.seq);
    pack.pack_header.id = ntohs(pack.pack_header.id);
    pack.pack_header.flags = ntohs(pack.pack_header.flags);
}

// host to network byte order transfer
void hton_reorder(packet &pack)
{
    pack.pack_header.ack = htonl(pack.pack_header.ack);
    pack.pack_header.seq = htonl(pack.pack_header.seq);
    pack.pack_header.id = htons(pack.pack_header.id);
    pack.pack_header.flags = htons(pack.pack_header.flags);
}

// set headers directly
void set_header(packet &pack, uint32_t S, uint32_t A, uint16_t I, uint16_t F)
{
    pack.pack_header.seq = htonl(S);
    pack.pack_header.ack = htonl(A);
    pack.pack_header.id = htons(I);
    pack.pack_header.flags = htons(F);
}