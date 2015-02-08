/*-
 * Copyright (c) 2011-2014 Anatol Belski
 * All rights reserved.
 *
 * Author: Anatol Belski <ab@php.net>
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
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"
#include "php_varnish.h"

#ifdef PHP_WIN32
#include <winsock2.h>
#else 
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_NET_INET_H
#include <netinet/in.h>
#endif
#endif

#include "varnish_lib.h"
#include "exception.h"

#if PHP_MAJOR_VERSION >= 7
extern zend_object_handlers default_varnish_adm_handlers;
#else
extern zend_object_handlers default_varnish_handlers;
#endif

#if PHP_MAJOR_VERSION >= 7
void
php_varnish_adm_obj_destroy(zend_object *obj)
{/*{{{*/
	struct ze_varnish_adm_obj *zvao = php_fetch_varnish_adm_obj(obj);

	zend_object_std_dtor(&zvao->zo);

	if (zvao->zvc.host_len > 0) {
		efree(zvao->zvc.host);
	}
	if (zvao->zvc.ident_len > 0) {
		efree(zvao->zvc.ident);
	}
	if (zvao->zvc.secret_len > 0) {
		efree(zvao->zvc.secret);
	}
	if (zvao->zvc.sock >= 0) {
#ifdef PHP_WIN32
		closesocket(zvao->zvc.sock);
#else
		close(zvao->zvc.sock);
#endif
	}
}/*}}}*/
#else
void
php_varnish_adm_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_varnish_adm_obj *zvao = (struct ze_varnish_adm_obj *)obj;

	zend_object_std_dtor(&zvao->zo TSRMLS_CC);

	if (zvao->zvc.host_len > 0) {
		efree(zvao->zvc.host);
	}
	if (zvao->zvc.ident_len > 0) {
		efree(zvao->zvc.ident);
	}
	if (zvao->zvc.secret_len > 0) {
		efree(zvao->zvc.secret);
	}
	if (zvao->zvc.sock >= 0) {
#ifdef PHP_WIN32
		closesocket(zvao->zvc.sock);
#else
		close(zvao->zvc.sock);
#endif
	}
	efree(zvao);
}/*}}}*/
#endif

#if PHP_MAJOR_VERSION >= 7
zend_object *
php_varnish_adm_obj_init(zend_class_entry *ze)
{/*{{{*/
	struct ze_varnish_adm_obj *zvao;

	zvao = (struct ze_varnish_adm_obj *)ecalloc(1, sizeof(struct ze_varnish_adm_obj));

	zend_object_std_init(&zvao->zo, ze);
	zvao->zo.handlers = &default_varnish_adm_handlers;

	zvao->zvc.host_len	 = 0;
	zvao->zvc.host		 = NULL;
	zvao->zvc.port	     = -1;
	zvao->zvc.secret_len = 0;
	zvao->zvc.secret	 = NULL;
	zvao->zvc.timeout	 = 0;
	zvao->zvc.sock	     = -1;
	zvao->zvc.ident_len  = 0;
	zvao->zvc.ident	     = NULL;
	zvao->zvc.authok     = 0;
	zvao->status		 = PHP_VARNISH_STATUS_CLOSE;
	zvao->compat		 = PHP_VARNISH_COMPAT_3;

	return &zvao->zo;
}/*}}}*/
#else
zend_object_value
php_varnish_adm_obj_init(zend_class_entry *ze TSRMLS_DC)
{/*{{{*/
	zend_object_value ret;
	struct ze_varnish_adm_obj *zvao;
#if PHP_VERSION_ID < 50399
	zval *tmp;
#endif

	zvao = (struct ze_varnish_adm_obj*)emalloc(sizeof(struct ze_varnish_adm_obj));
	memset(&zvao->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zvao->zo, ze TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(zvao->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zvao->zo, ze);
#endif

	zvao->zvc.host_len	 = 0;
	zvao->zvc.host		 = NULL;
	zvao->zvc.port	     = -1;
	zvao->zvc.secret_len = 0;
	zvao->zvc.secret	 = NULL;
	zvao->zvc.timeout	 = 0;
	zvao->zvc.sock	     = -1;
	zvao->zvc.ident_len  = 0;
	zvao->zvc.ident	     = NULL;
	zvao->zvc.authok     = 0;
	zvao->status		 = PHP_VARNISH_STATUS_CLOSE;
	zvao->compat		 = PHP_VARNISH_COMPAT_3;

	ret.handle = zend_objects_store_put(zvao, NULL,
										(zend_objects_free_object_storage_t) php_varnish_adm_obj_destroy,
										NULL TSRMLS_CC);

	ret.handlers = &default_varnish_handlers;

	return ret;
}/*}}}*/
#endif

void
php_varnish_throw_diag_f_exception(void *priv /* fd to output, ignored here */, const char *fmt, ...)
{/*{{{*/
/*	va_list list;
	char buff[1024];
	
	TSRMLS_FETCH();
	va_start(list, fmt);

	spprintf(&buff, 1024, estrdup(fmt), list);
	zend_throw_exception_ex(
			VarnishException_ce,
			0 TSRMLS_CC,
			estrdup(fmt),
			estrdup(buff)
	);

	va_end(list);*/
}/*}}}*/

/* {{{ proto void VarnishAdmin::__construct(array options)
 Varnish admin constructor */
PHP_METHOD(VarnishAdmin, __construct)
{
	struct ze_varnish_adm_obj *zvao;
	zval *opts = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &opts) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (NULL == opts) {
		php_varnish_default_ident(&zvao->zvc.ident, (int*)&zvao->zvc.ident_len);
	} else {
		/* read config options */
#if PHP_MAJOR_VERSION >= 7
		zval *secret, *addr, *port, *timeout, *ident, *compat;

		if((addr = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("host", sizeof("host")-1, 0))) != NULL) {
			convert_to_string(addr);
			zvao->zvc.host = estrdup(Z_STRVAL_P(addr));
			zvao->zvc.host_len = (int)Z_STRLEN_P(addr);
		}

		if((ident = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("ident", sizeof("ident")-1, 0))) != NULL) {
			convert_to_string(ident);
			zvao->zvc.ident = estrdup(Z_STRVAL_P(ident));
			zvao->zvc.ident_len = (int)Z_STRLEN_P(ident);
		}

		if((port = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("port", sizeof("port")-1, 0))) != NULL) {
			convert_to_long(port);
			zvao->zvc.port = (int)Z_LVAL_P(port);
		}

		if (zvao->zvc.ident_len > 0 && (zvao->zvc.host_len > 0 || zvao->zvc.port > -1)) {
			php_varnish_throw_ident_vs_host_exception(TSRMLS_C);
			return;
		}

		if (0 == zvao->zvc.ident_len) {
			if (zvao->zvc.host_len > 0 && zvao->zvc.port < 0) {
				zvao->zvc.port = 2000;
			} else if (0 == zvao->zvc.host_len || zvao->zvc.port < 0) {
				php_varnish_default_ident(&zvao->zvc.ident, (int*)&zvao->zvc.ident_len);
			}
		}

		if((timeout = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("timeout", sizeof("timeout")-1, 0))) != NULL) {
			convert_to_long(timeout);
			zvao->zvc.timeout = (int)Z_LVAL_P(timeout);
		}

		if((compat = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("compat", sizeof("compat")-1, 0))) != NULL) {
			convert_to_long(compat);
			zvao->compat = (int)Z_LVAL_P(compat);
		}
		if (!php_varnish_check_compat(zvao->compat)) {
			return;
		}

		if((secret = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("secret", sizeof("secret")-1, 0))) != NULL) {
			convert_to_string(secret);
			zvao->zvc.secret = estrdup(Z_STRVAL_P(secret));
			zvao->zvc.secret_len = (int)Z_STRLEN_P(secret);
		}
#else
		zval **secret, **addr, **port, **timeout, **ident, **compat;

		if(zend_hash_find(Z_ARRVAL_P(opts), "host", sizeof("host"), (void**)&addr) != FAILURE) {
			convert_to_string(*addr);
			zvao->zvc.host = estrdup(Z_STRVAL_PP(addr));
			zvao->zvc.host_len = Z_STRLEN_PP(addr);
		}

		if(zend_hash_find(Z_ARRVAL_P(opts), "ident", sizeof("ident"), (void**)&ident) != FAILURE) {
			convert_to_string(*ident);
			zvao->zvc.ident = estrdup(Z_STRVAL_PP(ident));
			zvao->zvc.ident_len = Z_STRLEN_PP(ident);
		}

		if(zend_hash_find(Z_ARRVAL_P(opts), "port", sizeof("port"), (void**)&port) != FAILURE) {
			convert_to_long(*port);
			zvao->zvc.port = (int)Z_LVAL_PP(port);
		}

		if (zvao->zvc.ident_len > 0 && (zvao->zvc.host_len > 0 || zvao->zvc.port > -1)) {
			php_varnish_throw_ident_vs_host_exception(TSRMLS_C);
			return;
		}

		if (0 == zvao->zvc.ident_len) {
			if (zvao->zvc.host_len > 0 && zvao->zvc.port < 0) {
				zvao->zvc.port = 2000;
			} else if (0 == zvao->zvc.host_len || zvao->zvc.port < 0) {
				php_varnish_default_ident(&zvao->zvc.ident, (int*)&zvao->zvc.ident_len);
			}
		}

		if(zend_hash_find(Z_ARRVAL_P(opts), "timeout", sizeof("timeout"), (void**)&timeout) != FAILURE) {
			convert_to_long(*timeout);
			zvao->zvc.timeout = (int)Z_LVAL_PP(timeout);
		}

		if(zend_hash_find(Z_ARRVAL_P(opts), "compat", sizeof("compat"), (void**)&compat) != FAILURE) {
			convert_to_long(*compat);
			zvao->compat = (int)Z_LVAL_PP(compat);
		}
		if (!php_varnish_check_compat(zvao->compat TSRMLS_CC)) {
			return;
		}

		if(zend_hash_find(Z_ARRVAL_P(opts), "secret", sizeof("secret"), (void**)&secret) != FAILURE) {
			convert_to_string(*secret);
			zvao->zvc.secret = estrdup(Z_STRVAL_PP(secret));
			zvao->zvc.secret_len = Z_STRLEN_PP(secret);
		}
#endif
	}
}
/* }}} */

/* {{{ proto boolean VarnishAdmin::connect()
 Connect to a varnish instance */
PHP_METHOD(VarnishAdmin, connect)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	/* get the socket */
	if (zvao->zvc.ident_len > 0) {
#ifndef PHP_WIN32
		zvao->zvc.sock = php_varnish_sock_ident(zvao->zvc.ident, &zvao->zvc.host, (int*)&zvao->zvc.host_len, &zvao->zvc.port,
										zvao->zvc.timeout, &zvao->status TSRMLS_CC);
		if (zvao->zvc.sock < 0) {
			RETURN_FALSE;
			return;
		}
#else
	 php_varnish_throw_win_unimpl_exception("Connection using identity is not supported on windows" TSRMLS_CC);
	 return;
#endif
	} else if (zvao->zvc.host_len > 0) {
		zvao->zvc.sock = php_varnish_sock(zvao->zvc.host, zvao->zvc.port,
										zvao->zvc.timeout, &zvao->status TSRMLS_CC);
		if (zvao->zvc.sock < 0) {
			RETURN_FALSE;
			return;
		}
	}

	RETURN_BOOL((zvao->zvc.sock > -1) && (PHP_VARNISH_STATUS_OK == zvao->status));
}

/* }}} */

/* {{{ proto boolean VarnishAdmin::auth()
 Authenticate on a varnish instance */
PHP_METHOD(VarnishAdmin, auth)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	zvao->status = PHP_VARNISH_STATUS_AUTH;

	/* authenticate */
	if (zvao->zvc.sock > -1) {
		if (zvao->zvc.ident_len > 0) {
#ifndef PHP_WIN32
			if(!php_varnish_auth_ident(zvao->zvc.sock, zvao->zvc.ident,
						zvao->zvc.timeout, &zvao->status TSRMLS_CC)) {
				RETURN_FALSE;
				return;
			}
#else
			php_varnish_throw_win_unimpl_exception("Authentication using identity is not supported on windows" TSRMLS_CC);
			return;
#endif
		} else if (zvao->zvc.secret_len > 0) {
			if(!php_varnish_auth(zvao->zvc.sock, zvao->zvc.secret,
				(int)zvao->zvc.secret_len, &zvao->status, zvao->zvc.timeout TSRMLS_CC)) {
				RETURN_FALSE;
				return;
			}
		}
	} else {
		php_varnish_throw_conn_exception(TSRMLS_C);
		zvao->zvc.authok = 0;
		return;
	}

	zvao->zvc.authok = (PHP_VARNISH_STATUS_OK == zvao->status);

	RETURN_BOOL(zvao->zvc.authok);
}
/* }}} */

/* {{{ proto int VarnishAdmin::getParams()
 Get varnish instance configuration */
PHP_METHOD(VarnishAdmin, getParams)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	array_init(return_value);
	php_varnish_get_params(zvao->zvc.sock, &zvao->status, return_value, zvao->zvc.timeout TSRMLS_CC);
}
/* }}} */

/* {{{ proto int VarnishAdmin::setParam(string name, mixed value)
 Set a varnish instance configuration */
PHP_METHOD(VarnishAdmin, setParam)
{
	zval *val, *val_str = NULL;
	char *name, *param;
#if PHP_MAJOR_VERSION >= 7
	size_t name_len;
#else
	int name_len;
#endif
	int param_len;
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &val) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	switch (Z_TYPE_P(val)) {
#if PHP_MAJOR_VERSION >= 7
		case IS_TRUE:
			param = "on";
			param_len = 2;
			break;

		case IS_FALSE:
			param = "off";
			param_len = 3;
			break;

		default: {
			zval val_str;

			ZVAL_COPY(val, &val_str);
			convert_to_string(val);

			param = Z_STRVAL(val_str);
			param_len = Z_STRLEN(val_str);

			zval_dtor(&val_str);
		}
#else
		case IS_BOOL:
			if (Z_BVAL_P(val)) {
				param = "on";
				param_len = 2;
			} else {
				param = "off";
				param_len = 3;
			}
			break;

		default:
			if (Z_TYPE_P(val) != IS_STRING) {
				ALLOC_INIT_ZVAL(val_str);
				*val_str = *val;
				INIT_PZVAL(val_str);
				zval_copy_ctor(val_str);
				convert_to_string(val);
			} else {
				val_str = val;
			}

			param = Z_STRVAL_P(val);
			param_len = Z_STRLEN_P(val);
#endif
	}

	(void)php_varnish_set_param(zvao->zvc.sock, &zvao->status, name, name_len, param, param_len, zvao->zvc.timeout TSRMLS_CC);

#if PHP_MAJOR_VERSION < 7
	if (val_str && val != val_str) {
		zval_ptr_dtor(&val_str);
	}
#endif

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::stop()
 Stop the varnish instance we are connected to */
PHP_METHOD(VarnishAdmin, stop)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	(void)php_varnish_stop(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::start()
 Start the varnish instance we are connected to */
PHP_METHOD(VarnishAdmin, start)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	(void)php_varnish_start(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::banUrl(string regexp)
 Ban all the objects where the URL matches the regexp  */
PHP_METHOD(VarnishAdmin, banUrl)
{
	char *regex;
#if PHP_MAJOR_VERSION >= 7
	size_t regex_len;
#else
	int regex_len;
#endif
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &regex, &regex_len) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	php_varnish_ban(zvao->zvc.sock, &zvao->status, regex, regex_len, zvao->zvc.timeout, PHP_VARNISH_BAN_URL_COMMAND, zvao->compat TSRMLS_CC);	

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::ban(string regexp)
 Ban all objects where the conditions match */
PHP_METHOD(VarnishAdmin, ban)
{
	char *regex;
#if PHP_MAJOR_VERSION >= 7
	size_t regex_len;
#else
	int regex_len;
#endif
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &regex, &regex_len) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	php_varnish_ban(zvao->zvc.sock, &zvao->status, regex, regex_len, zvao->zvc.timeout, PHP_VARNISH_BAN_COMMAND, zvao->compat TSRMLS_CC);	

	RETURN_LONG(zvao->status);
}
/* }}} */

/*{{{ proto boolean VarnishAdmin::isRunning(void)
 Get the status of the current status instance*/
PHP_METHOD(VarnishAdmin, isRunning)
{
	struct ze_varnish_adm_obj *zvao;
	int ret;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	ret = php_varnish_is_running(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);

	RETURN_BOOL(ret > 0 && PHP_VARNISH_STATUS_OK == zvao->status);
}
/*}}}*/

/* {{{ proto string VarnishAdmin::getPanic()
   Get the last panic message from the instance we are connected to */
PHP_METHOD(VarnishAdmin, getPanic)
{
	struct ze_varnish_adm_obj *zvao;
	char *content;
	int content_len;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	php_varnish_get_panic(zvao->zvc.sock, &zvao->status, &content, &content_len, zvao->zvc.timeout TSRMLS_CC);

#if PHP_MAJOR_VERSION >= 7
	RETURN_STRINGL(content, content_len);
#else
	RETURN_STRINGL(content, content_len, 0);
#endif
}
/* }}} */

/* {{{ proto integer VarnishAdmin::clearPanic()
   Clear the last panic message on the instance we are connected to */
PHP_METHOD(VarnishAdmin, clearPanic)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	php_varnish_clear_panic(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);	

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto void VarnishAdmin::setHost(string host)
 Set the host configuration option */
PHP_METHOD(VarnishAdmin, setHost)
{
	struct ze_varnish_adm_obj *zvao;
	char *host;
#if PHP_MAJOR_VERSION >= 7
	size_t host_len;
#else
	int host_len;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &host, &host_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zvao->zvc.host_len > 0) {
		efree(zvao->zvc.host);
	}
	zvao->zvc.host     = estrdup(host);
	zvao->zvc.host_len = host_len;
	zvao->zvc.authok   = 0;
}
/* }}} */

/* {{{ proto void VarnishAdmin::setIdent(string host)
 Set the ident configuration option */
PHP_METHOD(VarnishAdmin, setIdent)
{
	struct ze_varnish_adm_obj *zvao;
	char *ident;
#if PHP_MAJOR_VERSION >= 7
	size_t ident_len;
#else
	int ident_len;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ident, &ident_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zvao->zvc.ident_len > 0) {
		efree(zvao->zvc.ident);
	}
	zvao->zvc.ident     = estrdup(ident);
	zvao->zvc.ident_len = ident_len;
	zvao->zvc.authok    = 0;
}
/* }}} */

/* {{{ proto void VarnishAdmin::setSecret(string secret)
 Set the secret configuration option */
PHP_METHOD(VarnishAdmin, setSecret)
{
	struct ze_varnish_adm_obj *zvao;
	char *secret;
#if PHP_MAJOR_VERSION >= 7
	size_t secret_len;
#else
	int secret_len;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &secret, &secret_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (zvao->zvc.secret_len > 0) {
		efree(zvao->zvc.secret);
	}
	zvao->zvc.secret     = estrdup(secret);
	zvao->zvc.secret_len = secret_len;
	zvao->zvc.authok     = 0;
}
/* }}} */

/* {{{ proto void VarnishAdmin::setTimeout(int tmo)
 Set the timeout configuration option */
PHP_METHOD(VarnishAdmin, setTimeout)
{
	struct ze_varnish_adm_obj *zvao;
	zval *tmo;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &tmo) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	convert_to_long(tmo);
	zvao->zvc.timeout = (int)Z_LVAL_P(tmo);
}
/* }}} */

/* {{{ proto void VarnishAdmin::setPort(int port)
 Set the port configuration option */
PHP_METHOD(VarnishAdmin, setPort)
{
	struct ze_varnish_adm_obj *zvao;
	zval *port;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &port) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	convert_to_long(port);
	zvao->zvc.port = (int)Z_LVAL_P(port);
	zvao->zvc.authok    = 0;
}
/* }}} */

/* {{{ proto void VarnishAdmin::setCompat(int compat)
 Set the varnish compatibility */
PHP_METHOD(VarnishAdmin, setCompat)
{
	struct ze_varnish_adm_obj *zvao;
	zval *compat;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &compat) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	convert_to_long(compat);
	zvao->compat = (int)Z_LVAL_P(compat);

	php_varnish_check_compat(zvao->compat TSRMLS_CC);
}
/* }}} */

/*{{{ proto array VarnishAdmin::getVclList(void)
 Get VCLs loaded into the varnish instance */
PHP_METHOD(VarnishAdmin, getVclList)
{
	struct ze_varnish_adm_obj *zvao;

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (!php_varnish_adm_can_go(zvao TSRMLS_CC)) {
		return;
	}

	array_init(return_value);
	php_varnish_get_vcl_list(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout, return_value TSRMLS_CC);
}
/*}}}*/

/*{{{ proto boolean VarnishAdmin::vclUse(string config_name)
 Activate the given config */
PHP_METHOD(VarnishAdmin, vclUse)
{
	struct ze_varnish_adm_obj *zvao;
	char *conf_name;
#if PHP_MAJOR_VERSION >= 7
	size_t conf_name_len;
#else
	int conf_name_len;
#endif
	int ret;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &conf_name, &conf_name_len) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	/*ret = php_varnish_vcl_use(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout, conf_name, conf_name_len TSRMLS_CC);*/
	ret = php_varnish_vcl_use(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout, conf_name, conf_name_len TSRMLS_CC);

	RETURN_BOOL(ret > 0 && PHP_VARNISH_STATUS_OK == zvao->status);
}
/*}}}*/

/*{{{ proto boolean VarnishAdmin::disconnect(void)
 Close connection to a varnish instance */
PHP_METHOD(VarnishAdmin, disconnect)
{
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvao = php_fetch_varnish_adm_obj(Z_OBJ_P(getThis()));
#else
	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

#ifdef PHP_WIN32
	RETURN_BOOL(zvao->zvc.sock < 0 ? 0 : (closesocket(zvao->zvc.sock) == 0));
#else
	RETURN_BOOL(zvao->zvc.sock < 0 ? 0 : (close(zvao->zvc.sock) == 0));
#endif
}
/*}}}*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
