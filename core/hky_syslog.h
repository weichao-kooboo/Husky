#ifndef HKY_SYSLOG_H_INCLUDED
#define HKY_SYSLOG_H_INCLUDED

typedef struct{
    hky_pool_t *pool;
    hky_uint_t facility;
    hky_uint_t severity;
    hky_str_t tag;

    hky_addr_t server;
    hky_connection_t conn;
    unsigned  busy:1;
    unsigned  nohostname:1;
} hky_syslog_peer_t;

char *hky_syslog_process_conf(hky_conf_t *cf,hky_syslog_peer_t *peer);
void hky_syslog_writer(hky_log_t *log,hky_uint_t level,hky_uchar *buf,size_t len);

#endif // HKY_SYSLOG_H_INCLUDED
