#include "pmx_print.h"

#include <stdio.h>

static pmx_print_t	pmx_print_f = PMX_PRINT_STD;

pmx_int pmx_print( const char * const _format, ... ) {
	pmx_int retVal;
	va_list argList;
	va_start( argList, _format );

	if ( pmx_print_f == PMX_PRINT_STD ) {
		retVal = vprintf( ( const char * )_format, argList );
	} else {
		retVal = pmx_print_f( _format, argList );
	}

	va_end( argList );
	return retVal;
}

void pmx_print_set_f( pmx_print_t _pmx_print_f ) {
	pmx_print_f = _pmx_print_f;
}
