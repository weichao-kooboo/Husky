
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
}
void
hky_syslog_writer(hky_log_t *log,hky_uint_t level,hky_uchar *buf,size_t len){

}
