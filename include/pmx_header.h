#ifndef __PMX_HEADER_H__
#define __PMX_HEADER_H__

#include <libpmx/pmx.h>

typedef struct pmx_header_s {
	pmx_byte	signature[4];	/* "PMX " string */
	pmx_float	version;
	pmx_byte	entriesCount;
} pmx_header;

pmx_header *	pmx_header_create();
void			pmx_header_destroy( pmx_header * const _header );
pmx_byte		pmx_header_check( const pmx_header * const _header );

#endif