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

/* Compatibility with PHP < 8 */
#if PHP_VERSION_ID < 70200
#undef  ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX
#endif

#ifndef ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX
#define ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, return_reference, required_num_args, type, allow_null) \
        ZEND_BEGIN_ARG_INFO_EX(name, 0, return_reference, required_num_args)
#endif

#ifndef ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE
#define ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(pass_by_ref, name, type_hint, allow_null, default_value) \
        ZEND_ARG_TYPE_INFO(pass_by_ref, name, type_hint, allow_null)
#endif 

/* Arginfo generated with PHP 8 */ 
#include "varnish_arginfo.h"

/*ZEND_DECLARE_MODULE_GLOBALS(varnish)*/

/* True global resources - no need for thread safety here
static int le_varnish;
*/

/* Class entry definition */
zend_class_entry *VarnishAdmin_ce;
zend_class_entry *VarnishStat_ce;
zend_class_entry *VarnishLog_ce;
zend_class_entry *VarnishException_ce;

#if PHP_MAJOR_VERSION >= 7
zend_object_handlers default_varnish_adm_handlers;
zend_object_handlers default_varnish_log_handlers;
zend_object_handlers default_varnish_stat_handlers;
#else
zend_object_handlers default_varnish_handlers;
#endif

/* {{{ varnish_functions[]
 */
const zend_function_entry varnish_functions[] = {
	{NULL, NULL, NULL}	
};
/* }}} */

/* {{{ VarnishAdmin_methods[]
 */
const zend_function_entry VarnishAdmin_methods[] = {
	PHP_ME(VarnishAdmin, __construct, arginfo_class_VarnishAdmin___construct, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, connect, arginfo_class_VarnishAdmin_connect, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, auth, arginfo_class_VarnishAdmin_auth, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, getParams, arginfo_class_VarnishAdmin_getParams, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setParam, arginfo_class_VarnishAdmin_setParam, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, stop, arginfo_class_VarnishAdmin_stop, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, start, arginfo_class_VarnishAdmin_start, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, banUrl, arginfo_class_VarnishAdmin_banUrl, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, ban, arginfo_class_VarnishAdmin_ban, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, isRunning, arginfo_class_VarnishAdmin_isRunning, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, getPanic, arginfo_class_VarnishAdmin_getPanic, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, clearPanic, arginfo_class_VarnishAdmin_clearPanic, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setHost, arginfo_class_VarnishAdmin_setHost, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setIdent, arginfo_class_VarnishAdmin_setIdent, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setTimeout, arginfo_class_VarnishAdmin_setTimeout, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setPort, arginfo_class_VarnishAdmin_setPort, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setSecret, arginfo_class_VarnishAdmin_setSecret, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, setCompat, arginfo_class_VarnishAdmin_setCompat, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, getVclList, arginfo_class_VarnishAdmin_getVclList, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, vclUse, arginfo_class_VarnishAdmin_vclUse, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishAdmin, disconnect, arginfo_class_VarnishAdmin_disconnect, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ VarnishStat_methods{} */
const zend_function_entry VarnishStat_methods[] = {
	PHP_ME(VarnishStat, __construct, arginfo_class_VarnishStat___construct, ZEND_ACC_PUBLIC)
#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB < 40
	PHP_ME(VarnishStat, getSnapshot, arginfo_class_VarnishStat_getSnapshot, ZEND_ACC_PUBLIC)
#endif
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ VarnishLog_methods{} */
const zend_function_entry VarnishLog_methods[] = {
	PHP_ME(VarnishLog, __construct, arginfo_class_VarnishLog___construct, ZEND_ACC_PUBLIC)
#if defined(HAVE_VARNISHAPILIB) && HAVE_VARNISHAPILIB < 40
	PHP_ME(VarnishLog, getLine, arginfo_class_VarnishLog_getLine, ZEND_ACC_PUBLIC)
	PHP_ME(VarnishLog, getTagName, arginfo_class_VarnishLog_getTagName, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#endif
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ varnish_deps */
static const zend_module_dep varnish_deps[] = {
	ZEND_MOD_REQUIRED("hash")
	{NULL, NULL, NULL}
};/*}}}*/

/* {{{ varnish_module_entry
 */
zend_module_entry varnish_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	varnish_deps,
	"varnish",
	varnish_functions,
	PHP_MINIT(varnish),
	PHP_MSHUTDOWN(varnish),
	NULL,
	NULL,
	PHP_MINFO(varnish),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_VARNISH_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_VARNISH
ZEND_GET_MODULE(varnish)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("varnish.global_value",	  "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_varnish_globals, varnish_globals)
	STD_PHP_INI_ENTRY("varnish.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_varnish_globals, varnish_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_varnish_globals_ctor
 */
/*static void php_varnish_globals_ctor(zend_varnish_globals *varnish_globals)
{
}*/
/* }}} */

/* {{{ php_varnish_globals_dtor
 */
/*static void php_varnish_globals_dtor(zend_varnish_globals *varnish_globals)
{

}*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(varnish)
{
	zend_class_entry ce;

	/* REGISTER_INI_ENTRIES();*/
	/*ZEND_INIT_MODULE_GLOBALS(varnish, php_varnish_globals_ctor, php_varnish_globals_dtor);*/

#if PHP_MAJOR_VERSION >= 7
	memcpy(&default_varnish_adm_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_varnish_adm_handlers.clone_obj = NULL;
	default_varnish_adm_handlers.offset = XtOffsetOf(struct ze_varnish_adm_obj, zo);
	default_varnish_adm_handlers.free_obj = php_varnish_adm_obj_destroy;

	memcpy(&default_varnish_log_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_varnish_log_handlers.clone_obj = NULL;
	default_varnish_log_handlers.offset = XtOffsetOf(struct ze_varnish_log_obj, zo);
	default_varnish_log_handlers.free_obj = php_varnish_log_obj_destroy;

	memcpy(&default_varnish_stat_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_varnish_stat_handlers.clone_obj = NULL;
	default_varnish_stat_handlers.offset = XtOffsetOf(struct ze_varnish_stat_obj, zo);
	default_varnish_stat_handlers.free_obj = php_varnish_stat_obj_destroy;
#else
	memcpy(&default_varnish_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	default_varnish_handlers.clone_obj = NULL;
#endif

	/* Init internal classes */
	INIT_CLASS_ENTRY(ce, "VarnishAdmin", VarnishAdmin_methods);
	ce.create_object = php_varnish_adm_obj_init;
	VarnishAdmin_ce = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "VarnishStat", VarnishStat_methods);
	ce.create_object = php_varnish_stat_obj_init;
	VarnishStat_ce = zend_register_internal_class(&ce TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "VarnishLog", VarnishLog_methods);
	ce.create_object = php_varnish_log_obj_init;
	VarnishLog_ce = zend_register_internal_class(&ce TSRMLS_CC);

/* log is not working on windows at the time*/
#ifndef PHP_WIN32
#if HAVE_VARNISHAPILIB >= 40
#define SLTM(name, flags, shortdesc, longdesc) \
zend_declare_class_constant_long(VarnishLog_ce, "TAG_"#name, strlen("TAG_"#name), SLT_##name TSRMLS_CC);
#include "tbl/vsl_tags.h"
#else
#define SLTM(foo) \
zend_declare_class_constant_long(VarnishLog_ce, "TAG_"#foo, strlen("TAG_"#foo), SLT_##foo TSRMLS_CC);
#include "vsl_tags.h"
#endif
#undef SLTM
#endif
	/* Init exceptions */
	INIT_CLASS_ENTRY(ce, "VarnishException", NULL);
#if PHP_MAJOR_VERSION >= 7
	VarnishException_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default());
#else
	VarnishException_ce = zend_register_internal_class_ex(
		&ce, NULL, "exception" TSRMLS_CC
	);
#endif

	REGISTER_LONG_CONSTANT("VARNISH_STATUS_SYNTAX", PHP_VARNISH_STATUS_SYNTAX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_UNKNOWN", PHP_VARNISH_STATUS_UNKNOWN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_UNIMPL", PHP_VARNISH_STATUS_UNIMPL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_TOOFEW", PHP_VARNISH_STATUS_TOOFEW, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_TOOMANY", PHP_VARNISH_STATUS_TOOMANY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_PARAM", PHP_VARNISH_STATUS_PARAM, CONST_CS | CONST_PERSISTENT);  
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_AUTH", PHP_VARNISH_STATUS_AUTH, CONST_CS | CONST_PERSISTENT);   
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_OK", PHP_VARNISH_STATUS_OK, CONST_CS | CONST_PERSISTENT);	 
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_CANT", PHP_VARNISH_STATUS_CANT, CONST_CS | CONST_PERSISTENT);   
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_COMMS", PHP_VARNISH_STATUS_COMMS, CONST_CS | CONST_PERSISTENT);  
	REGISTER_LONG_CONSTANT("VARNISH_STATUS_CLOSE", PHP_VARNISH_STATUS_CLOSE, CONST_CS | CONST_PERSISTENT);  

	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_IDENT", "ident", CONST_CS | CONST_PERSISTENT);  
	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_HOST", "host", CONST_CS | CONST_PERSISTENT);  
	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_PORT", "port", CONST_CS | CONST_PERSISTENT);  
	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_TIMEOUT", "timeout", CONST_CS | CONST_PERSISTENT);  
	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_SECRET", "secret", CONST_CS | CONST_PERSISTENT);  
	REGISTER_STRING_CONSTANT("VARNISH_CONFIG_COMPAT", "compat", CONST_CS | CONST_PERSISTENT);  

	REGISTER_LONG_CONSTANT("VARNISH_COMPAT_2", PHP_VARNISH_COMPAT_2, CONST_CS | CONST_PERSISTENT);  
	REGISTER_LONG_CONSTANT("VARNISH_COMPAT_3", PHP_VARNISH_COMPAT_3, CONST_CS | CONST_PERSISTENT);  

	return SUCCESS;
}
/*  }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(varnish)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(varnish)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "varnish support", "enabled");
	php_info_print_table_row(2, "Extension version", PHP_VARNISH_VERSION);
	php_info_print_table_row(2, "Revision", "$Id$");
#ifdef VMOD_ABI_Version
	php_info_print_table_row(2, "Varnish version", VMOD_ABI_Version);
#endif
	php_info_print_table_end();

	/* 
	DISPLAY_INI_ENTRIES();
	*/
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
