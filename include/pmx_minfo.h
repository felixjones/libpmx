#ifndef __PMX_MINFO_H__
#define __PMX_MINFO_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_text.h>

enum pmx_minfo_mask_e {
	PMX_MINFO_NAME_LOCAL		= 0x1 << 0,
	PMX_MINFO_NAME_GLOBAL		= 0x1 << 1,
	PMX_MINFO_COMMENT_LOCAL		= 0x1 << 2,
	PMX_MINFO_COMMENT_GLOBAL	= 0x1 << 3
};

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