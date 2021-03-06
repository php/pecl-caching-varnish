/*-
 * Copyright (c) 2011-2018 Anatol Belski
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

#include "varnish_lib.h"
#include "exception.h"

#if PHP_MAJOR_VERSION >= 7
extern zend_object_handlers default_varnish_stat_handlers;
#else
extern zend_object_handlers default_varnish_handlers;
#endif

#if PHP_MAJOR_VERSION >= 7
void
php_varnish_stat_obj_destroy(zend_object *obj)
{/*{{{*/
	struct ze_varnish_stat_obj *zvso = php_fetch_varnish_stat_obj(obj);

	zend_object_std_dtor(&zvso->zo);

	if (zvso->zvc.ident_len > 0) {
		efree(zvso->zvc.ident);
	}
}/*}}}*/
#else
void
php_varnish_stat_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_varnish_stat_obj *zvso = (struct ze_varnish_stat_obj *)obj;

	zend_object_std_dtor(&zvso->zo TSRMLS_CC);

	if (zvso->zvc.ident_len > 0) {
		efree(zvso->zvc.ident);
	}

	efree(zvso);
}/*}}}*/
#endif

#if PHP_MAJOR_VERSION >= 7
zend_object *
php_varnish_stat_obj_init(zend_class_entry *ze)
{/*{{{*/
	struct ze_varnish_stat_obj *zvso;

	zvso = (struct ze_varnish_stat_obj *)ecalloc(1, sizeof(struct ze_varnish_stat_obj));

	zend_object_std_init(&zvso->zo, ze);
	zvso->zo.handlers = &default_varnish_stat_handlers;

	zvso->zvc.ident	 = NULL;
	zvso->zvc.ident_len = 0;

	return &zvso->zo;
}/*}}}*/
#else
zend_object_value
php_varnish_stat_obj_init(zend_class_entry *ze TSRMLS_DC)
{   /*{{{*/
	zend_object_value ret;
	struct ze_varnish_stat_obj *zvso;
#if PHP_VERSION_ID < 50399
	zval *tmp;
#endif

	zvso = (struct ze_varnish_stat_obj*)emalloc(sizeof(struct ze_varnish_stat_obj));
	memset(&zvso->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zvso->zo, ze TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(zvso->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zvso->zo, ze);
#endif

	zvso->zvc.ident	 = NULL;
	zvso->zvc.ident_len = 0;

	ret.handle = zend_objects_store_put(zvso, NULL,
										(zend_objects_free_object_storage_t) php_varnish_stat_obj_destroy,
										NULL TSRMLS_CC);

	ret.handlers = &default_varnish_handlers;

	return ret;
}/*}}}*/
#endif

/* {{{ proto void VarnishStat::__construct(array options)
 *  Varnish admin constructor */
PHP_METHOD(VarnishStat, __construct)
{
#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB >= 40
	 php_varnish_throw_win_unimpl_exception("VarnishStat functionality isn't available for Varnish >= 4" TSRMLS_CC);
	 return;
#elif !defined(PHP_WIN32)
	struct ze_varnish_stat_obj *zvso;
	zval *opts = NULL;
#if PHP_MAJOR_VERSION >= 7
	zval *ident;
#else
	zval **ident;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &opts) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvso = php_fetch_varnish_stat_obj(Z_OBJ_P(getThis()));
#else
	zvso = (struct ze_varnish_stat_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (NULL == opts) {
		php_varnish_default_ident(&zvso->zvc.ident, (int*)&zvso->zvc.ident_len);
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	if((ident = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("ident", sizeof("ident")-1, 0))) != NULL) {
		convert_to_string(ident);
		zvso->zvc.ident = estrdup(Z_STRVAL_P(ident));
		zvso->zvc.ident_len = Z_STRLEN_P(ident);
	} else {
		php_varnish_default_ident(&zvso->zvc.ident, (int*)&zvso->zvc.ident_len);
	}
#else
	if(zend_hash_find(Z_ARRVAL_P(opts), "ident", sizeof("ident"), (void**)&ident) != FAILURE) {
		convert_to_string(*ident);
		zvso->zvc.ident = estrdup(Z_STRVAL_PP(ident));
		zvso->zvc.ident_len = Z_STRLEN_PP(ident);
	} else {
		php_varnish_default_ident(&zvso->zvc.ident, (int*)&zvso->zvc.ident_len);
	}
#endif

#else 
	 php_varnish_throw_win_unimpl_exception("VarnishStat functionality isn't available on windows" TSRMLS_CC);
	 return;
#endif
}
/* }}} */

#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB < 40
/* {{{ proto array VarnishStat::getSnapshot(void)
  Get a statistics snapshot */
PHP_METHOD(VarnishStat, getSnapshot)
{
	struct ze_varnish_stat_obj *zvso;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvso = php_fetch_varnish_stat_obj(Z_OBJ_P(getThis()));
#else
	zvso = (struct ze_varnish_stat_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	/* Ensure the ctor was called properly and we have an ident to connect to */
	if (!zvso->zvc.ident) {
		zend_throw_exception_ex(
			VarnishException_ce,
			PHP_VARNISH_COMM_EXCEPTION TSRMLS_CC,
			"Missing ident to connect to"
		);
		return;
	}

	array_init(return_value);
	(void)php_varnish_snap_stats(return_value, zvso->zvc.ident TSRMLS_CC);
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
