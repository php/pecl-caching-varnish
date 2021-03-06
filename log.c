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
extern zend_object_handlers default_varnish_log_handlers;
#else
extern zend_object_handlers default_varnish_handlers;
#endif

#if PHP_MAJOR_VERSION >= 7
void
php_varnish_log_obj_destroy(zend_object *obj)
{/*{{{*/
	struct ze_varnish_log_obj *zvlo = php_fetch_varnish_log_obj(obj);

	zend_object_std_dtor(&zvlo->zo);

	if (zvlo->zvc.ident_len > 0) {
		efree(zvlo->zvc.ident);
	}

	if (zvlo->vd) {
		zvlo->vd = NULL;
	}
}/*}}}*/
#else
void
php_varnish_log_obj_destroy(void *obj TSRMLS_DC)
{/*{{{*/
	struct ze_varnish_log_obj *zvlo = (struct ze_varnish_log_obj *)obj;

	zend_object_std_dtor(&zvlo->zo TSRMLS_CC);

	if (zvlo->zvc.ident_len > 0) {
		efree(zvlo->zvc.ident);
	}

	if (zvlo->vd) {
		zvlo->vd = NULL;
	}

	efree(zvlo);
}/*}}}*/
#endif

#if PHP_MAJOR_VERSION >= 7
zend_object *
php_varnish_log_obj_init(zend_class_entry *ze)
{/*{{{*/
	struct ze_varnish_log_obj *zvlo;

	zvlo = (struct ze_varnish_log_obj *)ecalloc(1, sizeof(struct ze_varnish_log_obj));

	zend_object_std_init(&zvlo->zo, ze);
	zvlo->zo.handlers = &default_varnish_log_handlers;

	zvlo->zvc.ident	    = NULL;
	zvlo->zvc.ident_len = 0;
	zvlo->vd            = NULL;

	return &zvlo->zo;
}/*}}}*/
#else
zend_object_value
php_varnish_log_obj_init(zend_class_entry *ze TSRMLS_DC)
{   /*{{{*/
	zend_object_value ret;
	struct ze_varnish_log_obj *zvlo;
#if PHP_VERSION_ID < 50399
	zval *tmp;
#endif

	zvlo = (struct ze_varnish_log_obj*)emalloc(sizeof(struct ze_varnish_log_obj));
	memset(&zvlo->zo, 0, sizeof(zend_object));

	zend_object_std_init(&zvlo->zo, ze TSRMLS_CC);
#if PHP_VERSION_ID < 50399
	zend_hash_copy(zvlo->zo.properties, &ze->default_properties, (copy_ctor_func_t) zval_add_ref,
					(void *) &tmp, sizeof(zval *));
#else
	object_properties_init(&zvlo->zo, ze);
#endif

	zvlo->zvc.ident	    = NULL;
	zvlo->zvc.ident_len = 0;
	zvlo->vd            = NULL;

	ret.handle = zend_objects_store_put(zvlo, NULL,
										(zend_objects_free_object_storage_t) php_varnish_log_obj_destroy,
										NULL TSRMLS_CC);

	ret.handlers = &default_varnish_handlers;

	return ret;
}/*}}}*/
#endif

/* {{{ proto void VarnishLog::__construct(array options)
 *  Varnish admin constructor */
PHP_METHOD(VarnishLog, __construct)
{
#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB >= 40
	 php_varnish_throw_win_unimpl_exception("VarnishLog functionality isn't available for Varnish >= 4" TSRMLS_CC);
	 return;
#elif !defined(PHP_WIN32)
	struct ze_varnish_log_obj *zvlo;
	zval *opts = NULL, **ident;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &opts) == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvlo = php_fetch_varnish_log_obj(Z_OBJ_P(getThis()));
#else
	zvlo = (struct ze_varnish_log_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	if (NULL == opts) {
		php_varnish_default_ident(&zvlo->zvc.ident, (int*)&zvlo->zvc.ident_len);
	} else {
#if PHP_MAJOR_VERSION >= 7
		zval *ident;

		if((ident = zend_hash_find(Z_ARRVAL_P(opts), zend_string_init("ident", sizeof("ident")-1, 0))) != NULL) {
			convert_to_string(ident);
			zvlo->zvc.ident = estrdup(Z_STRVAL_P(ident));
			zvlo->zvc.ident_len = Z_STRLEN_P(ident);
		} else {
			php_varnish_default_ident(&zvlo->zvc.ident, (int*)&zvlo->zvc.ident_len);
		}
#else
		zval **ident;

		if(zend_hash_find(Z_ARRVAL_P(opts), "ident", sizeof("ident"), (void**)&ident) != FAILURE) {
			convert_to_string(*ident);
			zvlo->zvc.ident = estrdup(Z_STRVAL_PP(ident));
			zvlo->zvc.ident_len = Z_STRLEN_PP(ident);
		} else {
			php_varnish_default_ident(&zvlo->zvc.ident, (int*)&zvlo->zvc.ident_len);
		}
#endif
	}

	zvlo->vd = VSM_New();
	VSL_Setup(zvlo->vd);

	if (zvlo->zvc.ident_len > 0) {
		if (-1 == VSL_Arg(zvlo->vd, 'n', zvlo->zvc.ident)) {
			zend_throw_exception_ex(
				VarnishException_ce,
				PHP_VARNISH_UNKNOWN_EXCEPTION TSRMLS_CC,
				"Failed to handle the ident"
			);
		}
	}

	if (VSL_Open(zvlo->vd, 1)) {
		zend_throw_exception_ex(
			VarnishException_ce,
			PHP_VARNISH_CANT_EXCEPTION TSRMLS_CC,
			"Could not open shared log"
		);

		return;
	}
#else
	 php_varnish_throw_win_unimpl_exception("VarnishLog functionality isn't available on windows" TSRMLS_CC);
	 return;
#endif
}
/* }}} */

#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB < 40
/* {{{ proto array VarnishLog::getLine(void)
 * Get the next log entry */
PHP_METHOD(VarnishLog, getLine)
{
	struct ze_varnish_log_obj *zvlo;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

#if PHP_MAJOR_VERSION >= 7
	zvlo = php_fetch_varnish_log_obj(Z_OBJ_P(getThis()));
#else
	zvlo = (struct ze_varnish_log_obj *) zend_object_store_get_object(getThis() TSRMLS_CC);
#endif

	array_init(return_value);
	(void)php_varnish_get_log(zvlo->vd, return_value TSRMLS_CC);
}
/* }}} */


/* {{{ proto string VarnishLog::getTagName(int index)
 Get tag name by its numerical index */
PHP_METHOD(VarnishLog, getTagName)
{
	char *ret;
	zend_long ind;
	int ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &ind) == FAILURE) {
		return;
	}

	php_varnish_log_get_tag_name((int)ind, &ret, &ret_len TSRMLS_CC);

#if PHP_MAJOR_VERSION >= 7
	RETURN_STRINGL(ret, ret_len);
#else
	RETURN_STRINGL(ret, ret_len, 0);
#endif
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
