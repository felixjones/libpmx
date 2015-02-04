#include "pmx_text.h"

#include <string.h>

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
