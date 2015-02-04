#include "pmx_text.h"

#include <string.h>
#include <stdio.h>
#include "pmx_alloc.h"
#include "pmx_print.h"

pmx_text * pmx_text_create() {
	pmx_text * const text = ( pmx_text * )pmx_alloc( PMX_NULL, sizeof( pmx_text ) );
	memset( text, 0, sizeof( *text ) );
	return text;
}

void pmx_text_destroy( pmx_text * const _text ) {
	pmx_alloc( _text->string.utf8, 0 );
	pmx_alloc( _text, 0 );
}

pmx_int pmx_text_set_utf8( pmx_text * const _text, const char * const _cstr ) {
	if ( _cstr ) {
		_text->size = ( pmx_int )strlen( _cstr );
		_text->string.utf8 = ( pmx_byte * )pmx_alloc( _text->string.utf8, _text->size );
	
		memcpy( _text->string.utf8, _cstr, _text->size );

		return _text->size;
	}
	pmx_alloc( _text->string.utf8, 0 );
	return 0;
}

pmx_int pmx_text_set_utf16( pmx_text * const _text, const wchar_t * const _wstr ) {
	if ( _wstr ) {
		_text->size = ( pmx_int )( wcslen( _wstr ) * sizeof( int16_t ) );
		_text->string.utf16 = ( int16_t * )pmx_alloc( _text->string.utf16, _text->size );

		memcpy( _text->string.utf16, _wstr, _text->size );
	
		return _text->size;
	}
	pmx_alloc( _text->string.utf16, 0 );
	return 0;
}

pmx_int pmx_text_print( pmx_text * const _text, const pmx_byte _encoding ) {
	pmx_int retVal = -1;
	if ( _encoding ) {
		char * const buffer = ( char * )pmx_alloc( PMX_NULL, _text->size + 1 );
		memcpy( buffer, _text->string.utf8, _text->size );
		buffer[_text->size] = 0;

		retVal = pmx_print( "%hs", buffer );
		
		pmx_alloc( buffer, 0 );
	} else {
		wchar_t * const buffer = ( wchar_t * )pmx_alloc( PMX_NULL, _text->size + 2 );
		memcpy( buffer, _text->string.utf16, _text->size );
		buffer[_text->size >> 1] = 0;

		retVal = pmx_print( "%ls", buffer );

		pmx_alloc( buffer, 0 );
	}
	return retVal;
}
