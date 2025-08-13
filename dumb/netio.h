
#ifndef NETIO_H
#define NETIO_H

struct RemoteStation_struct;

#define NETBUF_LEN 1024

void net_init(void);
void net_reset(void);
int net_initstation(int station, const char *name,int flags);
#define net_initslave(name) net_initstation(-1,name,RS_SLAVE)
#define net_initmaster(name) net_initstation(-1,name,RS_MASTER)

void net_getmyhost(char *myname,size_t l);

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

