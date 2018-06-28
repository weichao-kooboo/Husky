
#include "hky_config.h"
#include "hky_core.h"

#define HKY_SYSLOG_MAX_STR													\
		HKY_MAX_ERROR_STR + sizeof("<255>Jan 01 00:00:00 ")-1				\
		+(HKY_MAXHOSTNAMELEN-1)+1											\
		+32+2

static char *hky_syslog_parse_args(hky_conf_t *cf,hky_syslog_peer_t *peer);
static hky_int_t hky_syslog_init_peer(hky_syslog_peer_t *peer);
static void hky_syslog_cleanup(void *data);
static char  *facilities[] = {
    "kern", "user", "mail", "daemon", "auth", "intern", "lpr", "news", "uucp",
    "clock", "authpriv", "ftp", "ntp", "audit", "alert", "cron", "local0",
    "local1", "local2", "local3", "local4", "local5", "local6", "local7",
    NULL
};

static char  *severities[] = {
    "emerg", "alert", "crit", "error", "warn", "notice", "info", "debug", NULL
};

static	hky_log_t	hky_syslog_dummy_log;
static	hky_event_t	hky_syslog_dummy_event;

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

hky_uchar *
hky_syslog_add_header(hky_syslog_peer_t *peer, hky_uchar *buf) {
	hky_uint_t pri;
	pri = peer->facility * 8 + peer->severity;
	if (peer->nohostname)
	{
		return hky_sprintf(buf, "<%ui>%V %V: ", pri, &hky_cached_syslog_time,
			&peer->tag);
	}
	return hky_sprintf(buf, "<%ui>%V %V %V: ", pri, &hky_cached_syslog_time,
		&hky_cycle->hostname, &peer->tag);
}

void
hky_syslog_writer(hky_log_t *log,hky_uint_t level,hky_uchar *buf,size_t len){
	hky_uchar	*p, msg[HKY_SYSLOG_MAX_STR];
	hky_uint_t	head_len;
	hky_syslog_peer_t	*peer;

	peer = log->wdata;
	if (peer->busy) {
		return;
	}
	peer->busy = 1;
	peer->severity = level - 1;

	p = hky_syslog_add_header(peer, msg);
	head_len = p - msg;

	len -= HKY_LINEFEED_SIZE;

	if (len > HKY_SYSLOG_MAX_STR - head_len) {
		len = HKY_SYSLOG_MAX_STR - head_len;
	}

	p = hky_snprintf(p, len, "%s", buf);

	(void)hky_syslog_send
}
ssize_t 
hky_syslog_send(hky_syslog_peer_t *peer, hky_uchar *buf, size_t len) {
	ssize_t	n;

	if (peer->conn.fd == (hky_socket_t)-1) {
		if (hky_syslog_init_peer(peer)!=HKY_OK)
		{
			return HKY_ERROR;
		}
	}
	peer->conn.log = hky_cycle->log;

	if (hky_send) {
		n = hky_send(&peer->conn, buf, len);
	}
	else {
		n = hky_os_io.send(&peer->conn, buf, len);
	}
}
static hky_int_t 
hky_syslog_init_peer(hky_syslog_peer_t *peer) {
	hky_socket_t fd;
	hky_pool_cleanup_t *cln;

	peer->conn.read = &hky_syslog_dummy_event;
	peer->conn.write = &hky_syslog_dummy_event;

	hky_syslog_dummy_event.log = &hky_syslog_dummy_log;

	fd = hky_socket(peer->server.sockaddr->sa_family, SOCK_DGRAM, 0);
	if (fd == (hky_socket_t)-1) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_socket_errno,
			hky_socket_n " failed");
		return HKY_ERROR;
	}
	if (hky_nonblocking(fd) == -1) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_socket_errno,
			hky_nonblocking_n " failed");
		goto failed;
	}
	if (connect(fd, peer->server.sockaddr, peer->server.socklen) == -1) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_socket_errno,
			"connect() failed");
		goto failed;
	}

	cln = hky_pool_cleanup_add(peer->pool, 0);
	if (cln == NULL) {
		goto failed;
	}
	cln->data = peer;
	cln->handler = hky_syslog_cleanup;

	peer->conn.fd = fd;

	peer->conn.write->ready = 1;

	return HKY_OK;
failed:
	if (hky_close_socket(fd)==-1)
	{
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_socket_errno,
			hky_close_socket_n " failed");
	}
	return HKY_ERROR;
}
static void 
hky_syslog_cleanup(void *data) {
	hky_syslog_peer_t *peer = data;

	peer->busy = 1;

	if (peer->conn.fd == (hky_socket_t)-1) {
		return;
	}
	if (hky_close_socket(peer->conn.fd) == -1) {
		hky_log_error(HKY_LOG_ALERT, hky_cycle->log, hky_socket_errno,
			hky_close_socket_n " failed");
	}
}
