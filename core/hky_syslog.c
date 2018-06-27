
#include "hky_config.h"
#include "hky_core.h"

static char *hky_syslog_parse_args(hky_conf_t *cf,hky_syslog_peer_t *peer);
static char  *facilities[] = {
    "kern", "user", "mail", "daemon", "auth", "intern", "lpr", "news", "uucp",
    "clock", "authpriv", "ftp", "ntp", "audit", "alert", "cron", "local0",
    "local1", "local2", "local3", "local4", "local5", "local6", "local7",
    NULL
};

static char  *severities[] = {
    "emerg", "alert", "crit", "error", "warn", "notice", "info", "debug", NULL
};
char *
hky_syslog_process_conf(hky_conf_t *cf,hky_syslog_peer_t *peer){
    peer->pool=cf->pool;
    peer->facility=HKY_CONF_UNSET_UINT;
    peer->severity=HKY_CONF_UNSET_UINT;
    
    if (hky_syslog_parse_args(cf, peer)!=HKY_CONF_OK) {
        return HKY_CONF_ERROR;
    }
    if (peer->server.sockaddr==NULL) {
        hky_conf_log_error(HKY_LOG_EMERG, cf, 0, "no syslog server specified");
        return HKY_CONF_ERROR;
    }
    
    if (peer->facility==HKY_CONF_UNSET_UINT) {
        peer->facility=23;
    }
    if (peer->severity==HKY_CONF_UNSET_UINT) {
        peer->severity=6;
    }
    if (peer->tag.data==NULL) {
        hky_str_set(&peer->tag, "husky");
    }
    peer->conn.fd=(hky_socket_t)-1;
    return HKY_CONF_OK;
}


static char *
hky_syslog_parse_args(hky_conf_t *cf,hky_syslog_peer_t *peer){
    
}

void
hky_syslog_writer(hky_log_t *log,hky_uint_t level,hky_uchar *buf,size_t len){

}
