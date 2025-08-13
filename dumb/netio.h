
#ifndef NETIO_H
#define NETIO_H

struct RemoteStation_struct;

typedef struct {
   const char *name;
   void (*init)(void);
   void (*reset)(void);
   int (*init_station)(struct RemoteStation_struct *rs);
   void (*reset_station)(struct RemoteStation_struct *rs);
   const unsigned char *(*recpkt)(size_t *size,int *station);
   int (*waitpkt)(int msec);
   void (*sendpkt)(struct RemoteStation_struct *rs,
		   const void *pkt,size_t size);
   void (*slavecast)(const unsigned char *pkt,size_t size);
   void (*broadcast)(const unsigned char *pkt,size_t size);
} NetDriver;

#define NETBUF_LEN 1024

void net_init(void);
void net_reset(void);
int net_initstation(int station, const char *name,int flags);
#define net_initslave(name) net_initstation(-1,name,RS_SLAVE)
#define net_initmaster(name) net_initstation(-1,name,RS_MASTER)

const unsigned char *net_recpkt(size_t *pktlen,int *station);
int net_waitpkt(struct RemoteStation_struct *rs,int msec);

typedef void (*NetSendFunc)(const void *,size_t);
void net_bufsend(NetSendFunc func,const void *pkt,size_t len);
void net_bufflush(void);

void net_sendto(struct RemoteStation_struct *rs,const void *pkt,size_t len);
void net_slavecast_nobuf(const void *pkt,size_t len);
void net_sendmaster_nobuf(const void *pkt,size_t len);

#define net_slavecast(pkt,len) net_bufsend(net_slavecast_nobuf,pkt,len)
#define net_sendmaster(pkt,len) net_bufsend(net_sendmaster_nobuf,pkt,len)
#define net_slavecast_flush() net_bufflush()
#define net_sendmaster_flush() net_bufflush()

#endif

