#ifndef __PMX_MODEL_INFO_H__
#define __PMX_MODEL_INFO_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_text.h>

typedef struct pmx_minfo_s {
	pmx_text	localCharacterName;
	pmx_text	globalCharacterName;
	pmx_text	localComment;
	pmx_text	globalComment;
} pmx_minfo;

pmx_minfo *	pmx_minfo_create();
void		pmx_minfo_destroy( pmx_minfo * const _minfo );

#endif