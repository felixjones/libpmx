#ifndef __PMX_HEADER_H__
#define __PMX_HEADER_H__

#include <libpmx/pmx.h>

typedef struct pmx_header_s {
	pmx_byte	signature[4];	/* "PMX " string */
	pmx_float	version;		/* 2.0 */
} pmx_header;

pmx_byte	pmx_header_check( const pmx_header * const _header );

#endif