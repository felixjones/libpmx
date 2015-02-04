#include "pmx_reader.h"

#include <stdio.h>
#include "pmx_alloc.h"
#include "pmx_header.h"

#define OFFSET_HEADER		( 0 )
#define OFFSET_INDEX_SIZE	( 9 )
#define OFFSET_MODEL_INFO	( 17 )

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

typedef struct {
	pmx_reader	super;
	FILE *		file;
} pmx_file_reader;

static pmx_int pmx_file_reader_read( pmx_file_reader * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount ) {
	return ( pmx_int )fread( _buffer, _eSize, _eCount, _reader->file );
}

typedef struct {
	pmx_reader	super;
	void *		start;
	void *		end;
	pmx_byte *	cursor;
} pmx_buffer_reader;

static pmx_int pmx_buffer_reader_read( pmx_buffer_reader * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount ) {
	pmx_byte * out = ( pmx_byte * )_buffer;
	pmx_int length = _eSize * _eCount;

	while ( length-- && ( void * )_reader->cursor < _reader->end ) {
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

pmx_int pmx_reader_seek_abs( pmx_reader * const _reader, const pmx_int _cur ) {
	switch ( _reader->type ) {
	case READER_BUFFER: {
			pmx_buffer_reader * const reader = ( pmx_buffer_reader * )_reader;
			const pmx_int oldPos = reader->cursor - ( pmx_byte * )reader->start;
			reader->cursor = ( pmx_byte * )pmx_ptr_add( reader->start, _cur );
			return oldPos;
		}
	case READER_FILE: {
			pmx_file_reader * const reader = ( pmx_file_reader * )_reader;
			const pmx_int oldPos = ftell( reader->file );
			fseek( reader->file, _cur, SEEK_SET );
			return oldPos;
		}
	}
	return -1;
}

pmx_int pmx_read_text( pmx_reader * const _reader, pmx_text * const _text ) {
	pmx_int readLen = _reader->read_f( _reader, &_text->size, sizeof( _text->size ), 1 );
	_text->string.utf8 = ( pmx_byte * )pmx_alloc( PMX_NULL, _text->size );
	readLen += _reader->read_f( _reader, _text->string.utf8, sizeof( *_text->string.utf8 ), _text->size );

	return readLen;
}

pmx_int pmx_read_header( pmx_reader * const _reader, pmx_header * const _header ) {
	if ( pmx_reader_seek_abs( _reader, OFFSET_HEADER ) >= 0 ) {
		pmx_int readLen = _reader->read_f( _reader, _header->signature, sizeof( *_header->signature ), 4 );
		readLen += _reader->read_f( _reader, &_header->version, sizeof( _header->version ), 1 );
		readLen += _reader->read_f( _reader, &_header->entriesCount, sizeof( _header->entriesCount ), 1 );
	
		return readLen;
	}
	return 0;
}

pmx_int pmx_read_isize( pmx_reader * const _reader, pmx_isize * const _isize ) {
	if ( pmx_reader_seek_abs( _reader, OFFSET_INDEX_SIZE ) >= 0 ) {
		pmx_int readLen = _reader->read_f( _reader, &_isize->textTypeEncoding, sizeof( _isize->textTypeEncoding ), 1 );
		readLen += _reader->read_f( _reader, &_isize->additionalUVCount, sizeof( _isize->additionalUVCount ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.vertex, sizeof( _isize->indexSizes.vertex ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.texture, sizeof( _isize->indexSizes.texture ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.material, sizeof( _isize->indexSizes.material ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.bone, sizeof( _isize->indexSizes.bone ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.morph, sizeof( _isize->indexSizes.morph ), 1 );
		readLen += _reader->read_f( _reader, &_isize->indexSizes.rigid, sizeof( _isize->indexSizes.rigid ), 1 );

		return readLen;
	}
	return 0;
}

pmx_int pmx_read_model_info( pmx_reader * const _reader, pmx_minfo * const _minfo ) {
	if ( pmx_reader_seek_abs( _reader, OFFSET_MODEL_INFO ) >= 0 ) {
		pmx_int readLen = pmx_read_text( _reader, &_minfo->localCharacterName );
		readLen += pmx_read_text( _reader, &_minfo->globalCharacterName );
		readLen += pmx_read_text( _reader, &_minfo->localComment );
		readLen += pmx_read_text( _reader, &_minfo->globalComment );
	
		return readLen;
	}
	return 0;
}
