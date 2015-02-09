#include "pmx_minfo.h"

pmx_int pmx_minfo_size( const pmx_minfo * const _minfo ) {
	pmx_int size = 16;

	size += _minfo->name.local.size;
	size += _minfo->name.global.size;
	size += _minfo->comment.local.size;
	size += _minfo->comment.global.size;

	return size;
}
