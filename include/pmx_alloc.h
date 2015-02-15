#ifndef __PMX_ALLOC_H__
#define __PMX_ALLOC_H__

#include <libpmx/pmx.h>

typedef void *	( * pmx_alloc_t )( void * const _ptr, const pmx_int _size );

#define PMX_ALLOC_STD	( ( pmx_alloc_t )0x0 )

void *	pmx_alloc( const void * const _ptr, const pmx_int _size );
void	pmx_alloc_set_f( pmx_alloc_t _pmx_alloc_f );

#endif