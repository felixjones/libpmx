#ifndef __PMX_H__
#define __PMX_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
	
typedef uint8_t	pmx_byte;	/* Unsigned binary integer */
typedef int16_t	pmx_short;	/* Signed, 2's complement binary integer */
typedef int32_t	pmx_int;	/* Signed, 2's complement binary integer */
typedef float	pmx_float;	/* An IEEE-754 floating-point value */

typedef float	pmx_vec2[2];
typedef float	pmx_vec3[3];
typedef float	pmx_vec4[4];

#include <libpmx/pmx_alloc.h>
#include <libpmx/pmx_print.h>

#define PMX_NULL	( 0x0 )

static __inline void * pmx_ptr_add( void * const _ptr, const pmx_int _val ) {
	return ( ( ( pmx_byte * )_ptr ) + _val );
}

#if defined(__cplusplus)
}
#endif

#endif