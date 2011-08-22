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

#include <varnishapi.h>

#include "varnish_lib.h"
#include "exception.h"


void
php_varnish_log_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_varnish_log_obj *zvlo = (struct ze_varnish_log_obj *)obj;

	zend_object_std_dtor(&zvlo->zo TSRMLS_CC);

	if (zvlo->zvc.ident_len > 0) {
		efree(zvlo->zvc.ident);
	}

	if (zvlo->format_len > 0) {
		efree(zvlo->format);
	}

	efree(zvlo);
}/*}}}*/

zend_object_value
php_varnish_log_obj_init(zend_class_entry *ze TSRMLS_DC)
{   /*{{{*/
		zend_object_value ret;
		struct ze_varnish_log_obj *zvlo;
		zval *tmp;

		zvlo = (struct ze_varnish_log_obj*)emalloc(sizeof(struct ze_varnish_log_obj));
		memset(&zvlo->zo, 0, sizeof(zend_object));

		zend_object_std_init(&zvlo->zo, ze TSRMLS_CC);
		zend_hash_copy(zvlo->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
						(void *) &tmp, sizeof(zval *));

		zvlo->zvc.ident	    = NULL;
		zvlo->zvc.ident_len = 0;
		zvlo->format	    = NULL;
		zvlo->format_len    = 0;

		ret.handle = zend_objects_store_put(zvlo, NULL,
											(zend_objects_free_object_storage_t) php_varnish_log_obj_destroy,
											NULL TSRMLS_CC);
		ret.handlers = zend_get_std_object_handlers();
		ret.handlers->clone_obj = NULL;

		return ret;
}/*}}}*/

/* {{{ proto void VarnishLog::__construct(array options)
 *  Varnish admin constructor */
PHP_METHOD(VarnishLog, __construct)
{
	struct ze_varnish_log_obj *zvlo;
	zval *opts, **ident, **format;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &opts) == FAILURE) {
		RETURN_NULL();
		return;
	}

	zvlo = (struct ze_varnish_log_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if(zend_hash_find(Z_ARRVAL_P(opts), "ident", sizeof("ident"), (void**)&ident) != FAILURE) {
		zvlo->zvc.ident = estrdup(Z_STRVAL_PP(ident));
		zvlo->zvc.ident_len = Z_STRLEN_PP(ident);
	}

	if(zend_hash_find(Z_ARRVAL_P(opts), "format", sizeof("format"), (void**)&format) != FAILURE) {
		zvlo->format = estrdup(Z_STRVAL_PP(format));
		zvlo->format_len = Z_STRLEN_PP(format);
	}
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
