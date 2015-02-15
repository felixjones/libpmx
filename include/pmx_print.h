#ifndef __PMX_PRINT_H__
#define __PMX_PRINT_H__

#include <libpmx/pmx.h>
#include <stdarg.h>

typedef pmx_int	( * pmx_print_t )( const pmx_byte * const _format, va_list _arg );

#define PMX_PRINT_STD	( ( pmx_print_t )0x0 )

pmx_int	pmx_print( const pmx_byte * const _format, ... );
void	pmx_print_set_f( pmx_print_t _pmx_print_f );

#endif