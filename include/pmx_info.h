#ifndef __PMX_INFO_H__
#define __PMX_INFO_H__

#include <libpmx/pmx.h>

typedef struct pmx_info_s {
	pmx_byte	dataCount;
	pmx_byte	textTypeEncoding;
	pmx_byte	additionalUVCount;
} pmx_info;

#endif