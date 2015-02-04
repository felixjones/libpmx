#include "pmx_alloc.h"

#include <malloc.h>

static pmx_alloc_t	pmx_alloc_f = PMX_ALLOC_STD;

void * pmx_alloc( void * const _ptr, const pmx_int _size ) {
	if ( pmx_alloc_f == PMX_ALLOC_STD ) {
		if ( _size ) {
			return realloc( _ptr, ( size_t )_size );
		}
		free( _ptr );
		return PMX_NULL;
	}
	return pmx_alloc_f( _ptr, _size );
}

void pmx_alloc_set_f( pmx_alloc_t _pmx_alloc_f ) {
	pmx_alloc_f = _pmx_alloc_f;
}
