#define max_file_size 100 * 1024 * 1024
#define packet_size 524
#define data_size 512
#define MSS 512
#define MAX_retran 20
#define MAX_SEQ_NUMBER 25600 
#define MAX_CONNECTIONS 10

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

typedef struct header header;

struct packet
{
    header pack_header;
    char data[data_size];
};

typedef struct packet packet;