#ifndef __PMX_TEXT_H__
#define __PMX_TEXT_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_info.h>

typedef struct pmx_text_s {
	pmx_int	size;	/* bytes length of encoded text */

	union string_u {
		pmx_byte *	utf8;	/* UTF-8 encoded text */
		pmx_short *	utf16;	/* UTF-16 encoded text */
	} string;	/* text pointers */
} pmx_text;

pmx_int		pmx_text_print( const pmx_text * const _text, const pmx_info * const _info );
pmx_int		pmx_text_length( const pmx_text * const _text, const pmx_info * const _info );
pmx_int		pmx_text_char( const pmx_text * const _text, const pmx_info * const _info, char * const _dest );
pmx_int		pmx_text_wchar( const pmx_text * const _text, const pmx_info * const _info, wchar_t * const _dest );

#endif