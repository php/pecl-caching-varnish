/*-
 * Copyright (c) 2011 Anatoliy Belsky
 * All rights reserved.
 *
 * Author: Anatoliy Belsky <ab@php.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_varnish.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include <fcntl.h>

#include "sha2.h"
#include "varnish_lib.h"
#include "exception.h"

/*{{{ const struct php_varnish_param PHP_VarnishParam[] = { */
const struct php_varnish_param PHP_VarnishParam[] = {
	{PHP_VARNISH_PARAM_ACCEPTOR_SLEEP_DECAY, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_ACCEPTOR_SLEEP_INCR, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_ACCEPTOR_SLEEP_MAX, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_AUTO_RESTART, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_BAN_DUPS, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_BAN_LURKER_SLEEP, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_BETWEEN_BYTES_TIMEOUT, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_CC_COMMAND, PHP_VARNISH_PARAM_QUOTED_STRING},
	{PHP_VARNISH_PARAM_CLI_BUFFER, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_CLI_TIMEOUT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_CLOCK_SKEW, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_CONNECT_TIMEOUT, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_CRITBIT_COOLOFF, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_DEFAULT_GRACE, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_DEFAULT_KEEP, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_DEFAULT_TTL, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_DIAG_BITMAP, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_ESI_SYNTAX, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_EXPIRY_SLEEP, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_FETCH_CHUNKSIZE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_FETCH_MAXCHUNKSIZE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_FIRST_BYTE_TIMEOUT, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_GROUP, PHP_VARNISH_PARAM_STRING},
	{PHP_VARNISH_PARAM_GZIP_LEVEL, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_GZIP_STACK_BUFFER, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_GZIP_TMP_SPACE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_HTTP_GZIP_SUPPORT, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_HTTP_MAX_HDR, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_HTTP_RANGE_SUPPORT, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_HTTP_REQ_HDR_LEN, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_HTTP_REQ_SIZE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_HTTP_RESP_HDR_LEN, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_HTTP_RESP_SIZE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_LISTEN_ADDRESS, PHP_VARNISH_PARAM_STRING},
	{PHP_VARNISH_PARAM_LISTEN_DEPTH, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_LOG_HASHSTRING, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_LOG_LOCAL_ADDRESS, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_LRU_INTERVAL, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_MAX_ESI_DEPTH, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_MAX_RESTARTS, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_PING_INTERVAL, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_PIPE_TIMEOUT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_PREFER_IPV6, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_QUEUE_MAX, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_RUSH_EXPONENT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SAINTMODE_THRESHOLD, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SEND_TIMEOUT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SESS_TIMEOUT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SESS_WORKSPACE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SESSION_LINGER, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SESSION_MAX, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SHM_RECLEN, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SHM_WORKSPACE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_SHORTLIVED, PHP_VARNISH_PARAM_FLOAT},
	{PHP_VARNISH_PARAM_SYSLOG_CLI_TRAFFIC, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_THREAD_POOL_ADD_DELAY, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_ADD_THRESHOLD, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_FAIL_DELAY, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_MAX, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_MIN, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_PURGE_DELAY, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_STACK, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_TIMEOUT, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOL_WORKSPACE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_POOLS, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_THREAD_STATS_RATE, PHP_VARNISH_PARAM_INT},
	{PHP_VARNISH_PARAM_USER, PHP_VARNISH_PARAM_STRING},
	{PHP_VARNISH_PARAM_VCC_ERR_UNREF, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_VCL_DIR, PHP_VARNISH_PARAM_STRING},
	{PHP_VARNISH_PARAM_VCL_TRACE, PHP_VARNISH_PARAM_BOOL},
	{PHP_VARNISH_PARAM_VMOD_DIR, PHP_VARNISH_PARAM_STRING},
	{PHP_VARNISH_PARAM_WAITER, PHP_VARNISH_PARAM_STRING}
};
/*}}}*/

static void
php_varnish_parse_add_param(zval *arr, char *line)
{/*{{{*/
	int param_cnt = sizeof(PHP_VarnishParam)/sizeof(struct php_varnish_param);
	int i, j, k, ival;
	float dval;
	char key[96], sval[160], *p;

	line++; /* ??? */
	p = strchr(line, ' ');
	k = (p - line < sizeof key) ? p - line : sizeof(key) - 1; 
	/* XXX check length */
	memcpy(key, line, k);
	key[k] = '\0';

	for (i = 0; i < param_cnt; i++) {
		if (!strncmp(key, PHP_VarnishParam[i].param_name, k)) {
			break;
		}
	}

	switch (PHP_VarnishParam[i].param_type) {
		case PHP_VARNISH_PARAM_STRING:
		case PHP_VARNISH_PARAM_FLOAT: /* thats unclean, i know */
			j = sscanf(line, "%s %s\n", key, sval);
			add_assoc_string(arr, key, sval, 1);
			break;
		case PHP_VARNISH_PARAM_QUOTED_STRING:
			/* ignore for now */
			break;
		case PHP_VARNISH_PARAM_INT:
			j = sscanf(line, "%s %d\n", key, &ival);
			add_assoc_long(arr, key, ival);
			break;
		case PHP_VARNISH_PARAM_BOOL:
			j = sscanf(line, "%s %s\n", key, sval);
			if (!strncmp("on", sval, 2)) {
				add_assoc_bool(arr, key, 1);
			} else {
				add_assoc_bool(arr, key, 0);
			}
			break;
	}
}/*}}}*/

static int
php_varnish_consume_bytes(int sock, char *ptr, int len, int tmo)
{/*{{{*/
	int got_now, i = len, j;
	char *p = ptr;
	struct pollfd pfd;

	pfd.fd = sock;
	pfd.events = POLLIN;

	j = poll(&pfd, 1, tmo);
	if (j == 0) {
		zend_throw_exception_ex(
			VarnishException_ce,
			PHP_VARNISH_TMO_EXCEPTION TSRMLS_CC,
			"Read operation timed out"
		);
		/* XXX throw timed out */
		return -1;
	}

	while(i > 0) {
		got_now = recv(sock, p, i, 0);
		if (got_now == -1) {
			return -1;
		}
		i -= got_now;
		p += got_now;
	}
#ifdef PHP_VARNISH_DEBUG
php_printf("received ''%s''\n", ptr);
#endif
	return len;
}/*}}}*/

static int
php_varnish_send_bytes(int sock, char *ptr, int len)
{/*{{{*/
	int sent, i = len;
	char *p = ptr;

	while(i > 0) {
		sent = send(sock, p, i, 0);
		if (sent == -1) {
			return -1;
		}
		i -= sent;
		p += sent;
	}
#ifdef PHP_VARNISH_DEBUG
php_printf("sent ''%s''\n", ptr);
#endif
	return len;
}/*}}}*/

static int
php_varnish_read_line0(int sock, int *status, int *content_len, int tmo)
{/*{{{*/
	char line0[PHP_VARNISH_LINE0_MAX_LEN];
	int numbytes, j;

	if((numbytes = php_varnish_consume_bytes(sock, line0, PHP_VARNISH_LINE0_MAX_LEN, tmo)) != PHP_VARNISH_LINE0_MAX_LEN) {
		zend_throw_exception_ex(
			VarnishException_ce,
			PHP_VARNISH_HANDSHAKE_EXCEPTION TSRMLS_CC,
			"Invalid status line length"
		);
		return -1;
	}
	line0[PHP_VARNISH_LINE0_MAX_LEN-1] = '\0';

	if (NULL == content_len) {
		j = sscanf(line0, "%d ", status);
		/* XXX j == 1 */
	} else {
		j = sscanf(line0, "%d %d\n", status, content_len);
		/* XXX j == 2 */
	}

	return numbytes;
}/*}}}*/

static int
php_varnish_invoke_command(int sock, char *command, int command_len, int *status, char **answer, int *answer_len, int tmo)
{/*{{{*/
	int numbytes;
	char *cmd;
	/* XXX check auth */
	/* one can use this to just forward the in stream */
	if (command_len) {
		cmd = emalloc(command_len+2);
		snprintf(cmd, command_len+2, "%s\n", command);

		if((numbytes = php_varnish_send_bytes(sock, cmd, command_len+1)) != command_len+1) {
			php_varnish_throw_comm_exception();
			return -1;
		}
	}

	if((numbytes = php_varnish_read_line0(sock, status, answer_len, tmo)) != PHP_VARNISH_LINE0_MAX_LEN) {
		php_varnish_throw_comm_exception();
		return 0;
	}
	(*answer) = emalloc(*answer_len+1);
	(*answer)[*answer_len+1] = '\0';
	numbytes = php_varnish_consume_bytes(sock, *answer, *answer_len+1, tmo);
	if(numbytes < 0) {
		php_varnish_throw_comm_exception();
		return 0;
	}

	/* forward all the leading whitespaces */
	while (*(*answer)) {
		if (!isspace(*(*answer))) {
			break;
		}
		(*answer)++;
		(*answer_len)--;
	}

	return 1;
}/*}}}*/

int
php_varnish_sock_ident(const char *ident, char *addr, int *port, int tmo, int *status TSRMLS_DC)
{/*{{{*/
	int sock = -1, j;
	struct VSM_data *vsd;
	char *t_arg, *t_start, *p, tmp_addr[41];

	vsd = VSM_New();
	if (VSM_n_Arg(vsd, ident)) {
		if (VSM_Open(vsd, 1)) {
			/* XXX throw could not open */
			return sock;
		}

		p = VSM_Find_Chunk(vsd, "Arg", "-T", "", NULL);
		if (NULL == p) {
			/* XXX throw no t_arg */
			return sock;
		}
		t_start = t_arg = estrdup(p);


	} else {
		/* XXX throw invalid param */
		return sock;
	}

	while(*t_arg) {
		p = strchr(t_arg, '\n');
		if (NULL == p) {
			return sock;
		}
		*p = '\0';
		j = sscanf(t_arg, "%s %d", tmp_addr, port);
		/* XXX j == 2 */
		sock = php_varnish_sock(tmp_addr, *port, tmo, status);
		if (sock > -1) {
			addr = estrdup(tmp_addr);	
			break;
		}
		t_arg = ++p;
	}

	efree(t_start);

	return sock;
}/*}}}*/

int
php_varnish_sock(const char *addr, int port, int tmo, int *status TSRMLS_DC)
{/*{{{*/
	int sock = -1, rc;
	struct in6_addr serveraddr;
	struct addrinfo hints, *res = NULL;
	char port_buf[8];

	do {
		memset(&hints, 0, sizeof hints);
		hints.ai_flags = AI_NUMERICSERV;
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		rc = inet_pton(AF_INET, addr, &serveraddr);
		if (1 == rc) {
			hints.ai_family = AF_INET;
			hints.ai_flags |= AI_NUMERICHOST;
		} else {
			rc = inet_pton(AF_INET6, addr, &serveraddr);
			if (1 == rc) {
				hints.ai_family = AF_INET6;
				hints.ai_flags |= AI_NUMERICHOST;
			}
		}

		snprintf(port_buf, 7, "%d", port);
		rc = getaddrinfo(addr, port_buf, &hints, &res);
		if (0 != rc) {
			if (rc == EAI_SYSTEM) {
				zend_throw_exception_ex(
					VarnishException_ce,
					PHP_VARNISH_SOCK_EXCEPTION TSRMLS_CC,
					"Could not translate address '%s'",
					addr
				);
			} else {
				zend_throw_exception_ex(
					VarnishException_ce,
					PHP_VARNISH_SOCK_EXCEPTION TSRMLS_CC,
					"Host '%s' not found. %s",
					addr,
					gai_strerror(rc)
				);
			}
			return sock;
		}

		if((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
			zend_throw_exception_ex(
				VarnishException_ce,
				PHP_VARNISH_SOCK_EXCEPTION TSRMLS_CC,
				"Unable to create socket"
			);
			*status = PHP_VARNISH_STATUS_COMMS;
			return sock;
		} 

		if(connect(sock, res->ai_addr, res->ai_addrlen) == -1) {
			zend_throw_exception_ex(
				VarnishException_ce,
				PHP_VARNISH_CONN_EXCEPTION TSRMLS_CC,
				"Unable to connect to '%s' on port '%d'",
				addr, port
			);
			*status = PHP_VARNISH_STATUS_COMMS;
			return -1;
		}

	} while (0);

	return sock;
}/*}}}*/

int
php_varnish_auth_ident(int sock, const char *ident, int tmo, int *status TSRMLS_DC)
{/*{{{*/
	struct VSM_data *vsd;
	char *p, *s_arg, *answer = NULL;
	int fd;
	char buf[CLI_AUTH_RESPONSE_LEN + 1];

	(void)VCLI_ReadResult(sock, status, &answer, tmo);
	if (PHP_VARNISH_STATUS_AUTH == *status) {
		vsd = VSM_New();
		if (VSM_n_Arg(vsd, ident)) {
			if (VSM_Open(vsd, 1)) {
				/* XXX throw could not open */
				return sock;
			}

			p = VSM_Find_Chunk(vsd, "Arg", "-S", "", NULL);
			if (p != NULL) {
				s_arg = estrdup(p);
				fd = open(s_arg, O_RDONLY);
				if (fd < 0) {
					zend_throw_exception_ex(
						VarnishException_ce,
						PHP_VARNISH_AUTH_EXCEPTION TSRMLS_CC,
						"Unable to open secret file at '%s'",
						s_arg
					);
					return 0;
				}
				efree(s_arg);
			}
			VCLI_AuthResponse(fd, answer, buf);
			close(fd);
			free(answer);
			
			if(-1 == php_varnish_send_bytes(sock, "auth ", strlen("auth "))) {
				php_varnish_throw_comm_exception();
			}
			if(-1 == php_varnish_send_bytes(sock, buf, CLI_AUTH_RESPONSE_LEN)) {
				php_varnish_throw_comm_exception();
			}
			if(-1 == php_varnish_send_bytes(sock, "\n", strlen("\n"))) {
				php_varnish_throw_comm_exception();
			}

			(void)VCLI_ReadResult(sock, status, &answer, tmo);
			free(answer);
		}
	}

	return 1;
}/*}}}*/

int
php_varnish_auth(int sock, char *secret, int secret_len, int *status, int tmo TSRMLS_DC)
{/*{{{*/
	char challenge[PHP_VARNISH_CHALLENGE_LEN+1], buf[SHA256_DIGEST_STRING_LENGTH], *content, *auth_seq;
	int numbytes, content_len, auth_seq_len;
	SHA256_CTX ctx256;

	if((numbytes = php_varnish_read_line0(sock, status, &content_len, tmo)) != PHP_VARNISH_LINE0_MAX_LEN) {
		zend_throw_exception_ex(
			VarnishException_ce,
			PHP_VARNISH_HANDSHAKE_EXCEPTION TSRMLS_CC,
			"Initial handshake failed"
		);
		return 0;
	}

	if (PHP_VARNISH_STATUS_AUTH == *status) {
		content = emalloc(content_len+1);
		content[content_len+1] = '\0';
		numbytes = php_varnish_consume_bytes(sock, content, content_len, tmo);
		if(numbytes < 0) {
			php_varnish_throw_comm_exception();
			return 0;
		}
		/* XXX use content directly if there is no logging requirement */
		memcpy(challenge, content, PHP_VARNISH_CHALLENGE_LEN);
		challenge[PHP_VARNISH_CHALLENGE_LEN+1] = '\0';

		SHA256_Init(&ctx256);
		SHA256_Update(&ctx256, (unsigned char*)estrdup(challenge), PHP_VARNISH_CHALLENGE_LEN);
		SHA256_Update(&ctx256, (unsigned char*)"\n", 1);
		SHA256_Update(&ctx256, (unsigned char*)estrdup(secret), secret_len);
		SHA256_Update(&ctx256, (unsigned char*)"\n", 1);
		SHA256_Update(&ctx256, (unsigned char*)estrdup(challenge), PHP_VARNISH_CHALLENGE_LEN);
		SHA256_Update(&ctx256, (unsigned char*)"\n", 1);
		SHA256_End(&ctx256, buf);

		if(-1 == php_varnish_send_bytes(sock, "auth ", strlen("auth "))) {
			php_varnish_throw_comm_exception();
		}
		if(-1 == php_varnish_send_bytes(sock, buf, SHA256_DIGEST_STRING_LENGTH-1)) {
			php_varnish_throw_comm_exception();
		}
		if(-1 == php_varnish_send_bytes(sock, "\n", strlen("\n"))) {
			php_varnish_throw_comm_exception();
		}
		//efree(content);

		/* forward to the end of the varnish out */
		php_varnish_invoke_command(sock, NULL, 0, status, &content, &content_len, tmo);
		//efree(content);
	}

	return 1;
}/*}}}*/

int
php_varnish_get_params(int sock, int *status, zval *storage, int tmo TSRMLS_DC)
{/*{{{*/
	int i = 0, content_len, len;
	char *content, *p0, *p1, buf[256];
	char key[64], val[64];

	php_varnish_invoke_command(sock, "param.show", 10, status, &content, &content_len, tmo);
	
	array_init(storage);

	p0 = p1 = content;
	while(i < content_len) {
		do {
			p1++;
		} while(*p1 != '\0' && *p1 != '\n');

		len = p1 - p0;
		memcpy(buf, p0, (len > 255 ? 255 : len));
		buf[len] = '\0';
		sscanf(buf, "%s %s", key, val); /* = 2 */
		php_varnish_parse_add_param(storage, buf);
		//add_assoc_string(storage, key, val, 1);
		p0 = p1;
		i += len;
	}

	//efree(content);
	return 1;
}/*}}}*/

int
php_varnish_set_param(int sock, int *status, char *key, int key_len, char *param, int param_len, int tmo TSRMLS_DC)
{/*{{{*/
	char *content, buf[128];
	int content_len;

	snprintf(buf, 128, "param.set %s \"%s\"", key, param);

	return php_varnish_invoke_command(sock, buf, key_len + param_len + 13, status, &content, &content_len, tmo);
}/*}}}*/

int
php_varnish_stop(int sock, int *status, int tmo)
{/*{{{*/
	char *content;
	int content_len;

	return php_varnish_invoke_command(sock, "stop", 4, status, &content, &content_len, tmo TSRMLS_DC);
}/*}}}*/ 


int
php_varnish_start(int sock, int *status, int tmo)
{/*{{{*/
	char *content;
	int content_len;

	return php_varnish_invoke_command(sock, "start", 5, status, &content, &content_len, tmo TSRMLS_DC);
}/*}}}*/

int
php_varnish_ban_url(int sock, int *status, char *reg, int reg_len, int tmo TSRMLS_DC)
{/*{{{*/
	char *content, buf[1024];
	int content_len;

	snprintf(buf, 1023, "ban.url \"%s\"", reg);

	return php_varnish_invoke_command(sock, buf, reg_len+10, status, &content, &content_len, tmo TSRMLS_DC);
}/*}}}*/

static int
php_varnish_snap_stats_cb(void *priv, const struct VSC_point const *pt)
{/*{{{*/
	uint64_t val;
	int f0, f1;
	zval *storage, *current;
	char buf0[128];

	storage = priv;

	val = *(const volatile uint64_t*)pt->ptr;

	f0 = strcmp(pt->class, "");
	f1 = strcmp(pt->ident, "");
	snprintf(buf0, 128, "%s%s%s%s%s",
			(f0 ? pt->class: ""),
			(f0 ? "." : ""),
			(f1 ? pt->ident : ""),
			(f1 ? "." : ""),
			pt->name);

	add_assoc_long(storage, buf0, val);

	return 0;
}/*}}}*/

int
php_varnish_snap_stats(zval *storage, const char *ident TSRMLS_DC)
{/*{{{*/
	struct VSM_data *vd;
	const struct VSC_C_main *vcm;

	vd = VSM_New();
	VSC_Setup(vd);

	VSC_Arg(vd, 'n', ident);

	if (VSC_Open(vd, 1))
		exit(1);

	vcm = VSC_Main(vd);

	(void)VSC_Iter(vd, php_varnish_snap_stats_cb, storage);

}/*}}}*/

int
php_varnish_is_running(int sock, int *status, int tmo TSRMLS_DC)
{/*{{{*/
	char *content, *msg;
	int content_len, ret = 0, msg_len;

	/* must parse the content message here, as the status would be always 200 in varnish 3.0 at least */
	msg = "Child in state running";
	msg_len = strlen(msg);

	ret = php_varnish_invoke_command(sock, "status", 6, status, &content, &content_len, tmo TSRMLS_CC);
	if (ret) {
		ret = !memcmp(msg, content, msg_len);
	}

	return ret;
}/*}}}*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
