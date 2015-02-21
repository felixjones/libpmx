#include "pmx_print.h"

#include <stdio.h>

static pmx_print_t	pmx_print_f = PMX_PRINT_STD;

pmx_int pmx_print( const char * const _format, ... ) {
	pmx_int retVal;
	va_list argList;
	va_start( argList, _format );

	retVal = pmx_print_varg( _format, argList );

	va_end( argList );
	return retVal;
}

pmx_int pmx_print_varg( const char * const _format, va_list _args ) {
	pmx_int retVal;

	if ( pmx_print_f == PMX_PRINT_STD ) {
		retVal = vprintf( ( const char * )_format, _args );
	} else {
		retVal = pmx_print_f( _format, _args );
	}

	return retVal;
}

void pmx_print_set_f( pmx_print_t _pmx_print_f ) {
	pmx_print_f = _pmx_print_f;
}

pmx_int pmx_print_error( const pmx_int _code, const char * const _class, const char * const _format, ... ) {
	pmx_int retVal;
	va_list argList;
	va_start( argList, _format );

	retVal = pmx_print( "libpmx %s error : ", _class );

	if ( retVal > -1 ) {
		const pmx_int printVal = pmx_print_varg( _format, argList );

		if ( printVal > -1 ) {
			retVal += printVal;
		} else {
			retVal = -1;
		}
	}
	
	va_end( argList );
	return retVal;
}
