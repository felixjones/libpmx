#ifndef __PMX_MINFO_H__
#define __PMX_MINFO_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_text.h>

typedef struct pmx_minfo_s {
	struct {
		pmx_text	local;
		pmx_text	global;
	} name;

	struct {
		pmx_text	local;
		pmx_text	global;
	} comment;
} pmx_minfo;

pmx_int	pmx_minfo_size( const pmx_minfo * const _minfo );

#endif