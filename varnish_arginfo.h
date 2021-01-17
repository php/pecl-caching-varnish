/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8f85af0f0ca3c73efcc24ed046662a4df80bf74f */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_VarnishAdmin___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, opt, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_connect, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_auth arginfo_class_VarnishAdmin_connect

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_getParams, 0, 0, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setParam, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_stop, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_start arginfo_class_VarnishAdmin_stop

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_banUrl, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, regex, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_ban arginfo_class_VarnishAdmin_banUrl

#define arginfo_class_VarnishAdmin_isRunning arginfo_class_VarnishAdmin_connect

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_getPanic, 0, 0, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_clearPanic arginfo_class_VarnishAdmin_stop

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setHost, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setIdent, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, ident, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setSecret, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, secret, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setTimeout, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setPort, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_setCompat, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, compat, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_getVclList arginfo_class_VarnishAdmin_getParams

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishAdmin_vclUse, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_VarnishAdmin_disconnect arginfo_class_VarnishAdmin_connect

#define arginfo_class_VarnishStat___construct arginfo_class_VarnishAdmin___construct

#define arginfo_class_VarnishStat_getSnapshot arginfo_class_VarnishAdmin_getParams

#define arginfo_class_VarnishLog___construct arginfo_class_VarnishAdmin___construct

#define arginfo_class_VarnishLog_getLine arginfo_class_VarnishAdmin_getParams

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_VarnishLog_getTagName, 0, 1, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ind, IS_LONG, 0)
ZEND_END_ARG_INFO()
