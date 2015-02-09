#include "pmx_text.h"

#include <stdio.h>
#include <string.h>
#include "pmx_print.h"

pmx_int pmx_text_print( const pmx_text * const _text, const pmx_info * const _info ) {
	pmx_int retVal = 0;
	if ( _info->textTypeEncoding ) {
		char * const text = ( char * )pmx_alloc( PMX_NULL, sizeof( char ) * ( _text->size + 1 ) );
		memcpy( text, _text->string.utf8, _text->size );
		text[_text->size] = 0;

		retVal = pmx_print( "%hs", text );

		pmx_alloc( text, 0 );
	} else {
		const pmx_int sizeChar = _text->size / 2;
		wchar_t * const text = ( wchar_t * )pmx_alloc( PMX_NULL, sizeof( wchar_t ) * ( sizeChar + 1 ) );
		memcpy( text, _text->string.utf16, _text->size );
		text[sizeChar] = 0;

		retVal = pmx_print( "%ls", text );

		pmx_alloc( text, 0 );
	}
	return retVal;
}

pmx_int pmx_text_length( const pmx_text * const _text, const pmx_info * const _info ) {
	if ( _info->textTypeEncoding ) {
		return _text->size;
	}
	return _text->size / 2;
}

pmx_int pmx_text_char( const pmx_text * const _text, const pmx_info * const _info, char * const _dest ) {
	pmx_int retVal = 0;
	if ( _info->textTypeEncoding ) {
		char * const text = ( char * )pmx_alloc( PMX_NULL, sizeof( char ) * ( _text->size + 1 ) );
		memcpy( text, _text->string.utf8, _text->size );
		text[_text->size] = 0;

		retVal = sprintf( _dest, "%hs", text );

		pmx_alloc( text, 0 );
	} else {
		const pmx_int sizeChar = _text->size / 2;
		wchar_t * const text = ( wchar_t * )pmx_alloc( PMX_NULL, sizeof( wchar_t ) * ( sizeChar + 1 ) );
		memcpy( text, _text->string.utf16, _text->size );
		text[sizeChar] = 0;

		retVal = sprintf( _dest, "%ls", text );

		pmx_alloc( text, 0 );
	}
	return retVal;
}

pmx_int pmx_text_wchar( const pmx_text * const _text, const pmx_info * const _info, wchar_t * const _dest ) {
	pmx_int retVal = 0;
	if ( _info->textTypeEncoding ) {
		char * const text = ( char * )pmx_alloc( PMX_NULL, sizeof( char ) * ( _text->size + 1 ) );
		memcpy( text, _text->string.utf8, _text->size );
		text[_text->size] = 0;

		retVal = swprintf( _dest, L"%hs", text );

		pmx_alloc( text, 0 );
	} else {
		const pmx_int sizeChar = _text->size / 2;
		wchar_t * const text = ( wchar_t * )pmx_alloc( PMX_NULL, sizeof( wchar_t ) * ( sizeChar + 1 ) );
		memcpy( text, _text->string.utf16, _text->size );
		text[sizeChar] = 0;

		retVal = swprintf( _dest, L"%ls", text );

		pmx_alloc( text, 0 );
	}
	return retVal;
}
