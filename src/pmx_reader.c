#include "pmx_reader.h"

#include <stdio.h>

enum readerType_e {
	READER_FILE		= ( ( 'F' << 24 ) | ( 'I' << 16 ) | ( 'L' << 8 ) | 'E' ),
	READER_BUFFER	= ( ( 'B' << 24 ) | ( 'U' << 16 ) | ( 'F' << 8 ) | 'F' ),
	READER_CUSTOM	= PMX_READER_CUSTOM
};

__inline static pmx_byte pmx_reader_type_check( const pmx_int _type ) {
	if ( _type == READER_FILE || _type == READER_BUFFER || _type == READER_CUSTOM ) {
		return 0;
	}
	return -1;
}

typedef struct pmx_file_reader_s {
	pmx_reader	super;
	FILE *		file;
} pmx_file_reader;

static pmx_int pmx_file_reader_read( pmx_file_reader * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount ) {
	return ( pmx_int )fread( _buffer, _eSize, _eCount, _reader->file );
}

typedef struct pmx_buffer_reader_s {
	pmx_reader	super;
	void *		start;
	void *		end;
	pmx_byte *	cursor;
} pmx_buffer_reader;

static pmx_int pmx_buffer_reader_read( pmx_buffer_reader * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount ) {
	pmx_byte * out = ( pmx_byte * )_buffer;
	pmx_int length = _eSize * _eCount;

	while ( length-- && _reader->cursor != _reader->end ) {
		*out = *_reader->cursor;
		out++;
		_reader->cursor++;
	}

	return ( length + 1 ) / _eSize;
}

pmx_reader * pmx_reader_create_file( void * const _file ) {
	pmx_file_reader * const reader = ( pmx_file_reader * )pmx_alloc( PMX_NULL, sizeof( *reader ) );
	reader->super.type = READER_FILE;
	reader->super.read_f = ( pmx_reader_read_t )pmx_file_reader_read;
	
	reader->file = ( FILE * )_file;

	return &reader->super;
}

pmx_reader * pmx_reader_create_buffer( void * const _buffer, const size_t _length ) {
	pmx_buffer_reader * const reader = ( pmx_buffer_reader * )pmx_alloc( PMX_NULL, sizeof( *reader ) );
	reader->super.type = READER_BUFFER;
	reader->super.read_f = ( pmx_reader_read_t )pmx_buffer_reader_read;

	reader->start = _buffer;
	reader->end = pmx_ptr_add( _buffer, ( pmx_int )_length );
	reader->cursor = ( pmx_byte * )reader->start;

	return &reader->super;
}

void pmx_reader_destroy( pmx_reader * const _reader ) {
	pmx_alloc( _reader, 0 );
}

pmx_int pmx_reader_header( pmx_reader * const _reader, pmx_header * const _header ) {
	return _reader->read_f( _reader, _header, sizeof( *_header ), 1 );
}
