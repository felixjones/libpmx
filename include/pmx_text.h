#ifndef __PMX_TEXT_H__
#define __PMX_TEXT_H__

#include <libpmx/pmx.h>

typedef struct pmx_text_s {
	pmx_int	size;	/* bytes length of encoded text */

	union string_u {
		pmx_byte *	utf8;	/* UTF-8 encoded text */
		pmx_short *	utf16;	/* UTF-16 encoded text */
	} string;	/* text pointers */
} pmx_text;

pmx_text *	pmx_text_create();
void		pmx_text_destroy( pmx_text * const _text );

pmx_int		pmx_text_set_utf8( pmx_text * const _text, const char * const _cstr );
pmx_int		pmx_text_set_utf16( pmx_text * const _text, const wchar_t * const _wstr );

pmx_int		pmx_text_print( pmx_text * const _text, const pmx_byte _encoding );

#endif