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
 * $Id$
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

#include <varnishapi.h>

#include "varnish_lib.h"
#include "exception.h"

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
	efree(zvao);
}/*}}}*/

zend_object_value
php_varnish_adm_obj_init(zend_class_entry *ze TSRMLS_DC)
{/*{{{*/
	zend_object_value ret;
	struct ze_varnish_adm_obj *zvao;
	zval *tmp;
	
	zvao = (struct ze_varnish_adm_obj*)emalloc(sizeof(struct ze_varnish_adm_obj));
	memset(&zvao->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zvao->zo, ze TSRMLS_CC);
	zend_hash_copy(zvao->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));

	zvao->zvc.host_len	= 0;
	zvao->zvc.host		= NULL;
	zvao->zvc.port	   = 2000;
	zvao->zvc.secret_len = 0;
	zvao->zvc.secret	 = NULL;
	zvao->zvc.timeout	= 0;
	zvao->zvc.sock	   = -1;
	zvao->zvc.ident_len  = 0;
	zvao->zvc.ident	  = NULL;
	zvao->status		 = PHP_VARNISH_STATUS_CLOSE;

	ret.handle = zend_objects_store_put(zvao, NULL,
										(zend_objects_free_object_storage_t) php_varnish_adm_obj_destroy,
										NULL TSRMLS_CC);
	ret.handlers = zend_get_std_object_handlers();
	ret.handlers->clone_obj = NULL;

	return ret;
}/*}}}*/

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
	zval *opts, **secret, **addr, **port, **timeout, **ident;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &opts) == FAILURE) {
		return;
	}

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	/* read config options */
	if(zend_hash_find(Z_ARRVAL_P(opts), "host", sizeof("host"), (void**)&addr) != FAILURE) {
		zvao->zvc.host = estrdup(Z_STRVAL_PP(addr));
		zvao->zvc.host_len = Z_STRLEN_PP(addr);
	}

	if(zend_hash_find(Z_ARRVAL_P(opts), "ident", sizeof("ident"), (void**)&ident) != FAILURE) {
		zvao->zvc.ident = estrdup(Z_STRVAL_PP(ident));
		zvao->zvc.ident_len = Z_STRLEN_PP(ident);
	}

	if(zend_hash_find(Z_ARRVAL_P(opts), "port", sizeof("port"), (void**)&port) != FAILURE) {
		zvao->zvc.port = (int)Z_LVAL_PP(port);
	}

	if(zend_hash_find(Z_ARRVAL_P(opts), "timeout", sizeof("timeout"), (void**)&timeout) != FAILURE) {
		zvao->zvc.timeout = (int)Z_LVAL_PP(timeout);
	}

	if(zend_hash_find(Z_ARRVAL_P(opts), "secret", sizeof("secret"), (void**)&secret) != FAILURE) {
		zvao->zvc.secret = estrdup(Z_STRVAL_PP(secret));
		zvao->zvc.secret_len = Z_STRLEN_PP(secret);
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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
	
	/* XXX check status */

	/* get the socket */
	if (zvao->zvc.ident_len > 0) {
		zvao->zvc.sock = php_varnish_sock_ident(zvao->zvc.ident, zvao->zvc.host, &zvao->zvc.port,
										zvao->zvc.timeout, &zvao->status TSRMLS_CC);
		if (zvao->zvc.sock < 0) {
			RETURN_FALSE;
			return;
		}
	} else if (zvao->zvc.host_len > 0) {
		zvao->zvc.sock = php_varnish_sock(zvao->zvc.host, zvao->zvc.port,
										zvao->zvc.timeout, &zvao->status TSRMLS_CC);
		if (zvao->zvc.sock < 0) {
			RETURN_FALSE;
			return;
		}
	}

	RETURN_BOOL(zvao->zvc.sock > -1);
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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	zvao->status = PHP_VARNISH_STATUS_AUTH;

	/* authenticate */
	if (zvao->zvc.sock > -1) {
		if (zvao->zvc.ident_len > 0) {
			if(!php_varnish_auth_ident(zvao->zvc.sock, zvao->zvc.ident,
						zvao->zvc.timeout, &zvao->status TSRMLS_CC)) {
				RETURN_FALSE;
				return;
			}
		} else if (zvao->zvc.secret_len > 0) {
			if(!php_varnish_auth(zvao->zvc.sock, zvao->zvc.secret,
				(int)zvao->zvc.secret_len, &zvao->status, zvao->zvc.timeout TSRMLS_CC)) {
				RETURN_FALSE;
				return;
			}
		}
	}

	RETURN_BOOL(PHP_VARNISH_STATUS_OK == zvao->status);
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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_varnish_get_params(zvao->zvc.sock, &zvao->status, return_value, zvao->zvc.timeout TSRMLS_CC);

	/*RETURN_LONG(zvao->status);*/
}
/* }}} */

/* {{{ proto int VarnishAdmin::setParam(string name, mixed value)
 Set a varnish instance configuration */
PHP_METHOD(VarnishAdmin, setParam)
{
	zval *val, *val_str = NULL;
	char *name, *param;
	long name_len, param_len;
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &val) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	switch (Z_TYPE_P(val)) {
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
	}

	(void)php_varnish_set_param(zvao->zvc.sock, &zvao->status, name, name_len, param, param_len, zvao->zvc.timeout TSRMLS_CC);

	if (val_str && val != val_str) {
		zval_ptr_dtor(&val_str);
	}

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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	(void)php_varnish_start(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::banUrl(string regexp)
 Ban all the objects where the URL matches the regexp  */
PHP_METHOD(VarnishAdmin, banUrl)
{
	char *regex;
	long regex_len;
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &regex, &regex_len) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_varnish_ban(zvao->zvc.sock, &zvao->status, regex, regex_len, zvao->zvc.timeout, PHP_VARNISH_BAN_URL_COMMAND TSRMLS_CC);	

	RETURN_LONG(zvao->status);
}
/* }}} */

/* {{{ proto int VarnishAdmin::ban(string regexp)
 Ban all objects where the conditions match */
PHP_METHOD(VarnishAdmin, ban)
{
	char *regex;
	long regex_len;
	struct ze_varnish_adm_obj *zvao;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &regex, &regex_len) == FAILURE) {
		RETURN_LONG(-1);
		return;
	}

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_varnish_ban(zvao->zvc.sock, &zvao->status, regex, regex_len, zvao->zvc.timeout, PHP_VARNISH_BAN_COMMAND TSRMLS_CC);	

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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	ret = php_varnish_is_running(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);

	RETURN_BOOL(ret && PHP_VARNISH_STATUS_OK == zvao->status);
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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_varnish_get_panic(zvao->zvc.sock, &zvao->status, &content, &content_len, zvao->zvc.timeout TSRMLS_CC);

	RETURN_STRINGL(content, content_len, 0);
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

	zvao = (struct ze_varnish_adm_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	php_varnish_clear_panic(zvao->zvc.sock, &zvao->status, zvao->zvc.timeout TSRMLS_CC);	

	RETURN_LONG(zvao->status);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
