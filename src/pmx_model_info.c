#include "pmx_model_info.h"

#include <string.h>

pmx_minfo *	pmx_minfo_create() {
	pmx_minfo * const info = ( pmx_minfo * )pmx_alloc( PMX_NULL, sizeof( *info ) );
	memset( info, 0, sizeof( *info ) );

	return info;
}

void pmx_minfo_destroy( pmx_minfo * const _minfo ) {
	pmx_alloc( _minfo->globalCharacterName.string.utf8, 0 );
	pmx_alloc( _minfo->globalComment.string.utf8, 0 );
	pmx_alloc( _minfo->localCharacterName.string.utf8, 0 );
	pmx_alloc( _minfo->localComment.string.utf8, 0 );
	pmx_alloc( _minfo, 0 );
}
