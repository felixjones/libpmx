#include "pmx_read.h"

#include <stdio.h>
#include <string.h>

#define PMX_READ_FILE	( ( 'F' << 24 ) | ( 'I' << 16 ) | ( 'L' << 8 ) | 'E' )
#define PMX_READ_BUFFER	( ( 'B' << 24 ) | ( 'U' << 16 ) | ( 'F' << 8 ) | 'F' )

#define PMX_OFFSET_HEADER	( 0 )
#define PMX_OFFSET_INFO		( 8 )
#define PMX_OFFSET_ISIZE	( 11 )
#define PMX_OFFSET_MINFO	( 17 )

enum pmx_read_bits_e {
	PMX_BIT_HEADER			= ( 0x1 << 0 ),
	PMX_BIT_INFO			= ( 0x1 << 1 ),
	PMX_BIT_ISIZE			= ( 0x1 << 2 ),
	PMX_BIT_MINFO			= ( 0x1 << 3 ),

	PMX_BIT_INFO_ALL		= ( PMX_BIT_HEADER | PMX_BIT_INFO | PMX_BIT_ISIZE | PMX_BIT_MINFO ),

	PMX_BIT_OFFSET_VERTEX	= ( 0x1 << 8 ),
	PMX_BIT_OFFSET_FACE		= ( 0x1 << 9 ),
	PMX_BIT_OFFSET_TEXTURE	= ( 0x1 << 10 ),

	PMX_BIT_OFFSET_ALL		= ( PMX_BIT_OFFSET_VERTEX | PMX_BIT_OFFSET_FACE | PMX_BIT_OFFSET_TEXTURE )
};

typedef struct pmx_read_private_s {
	pmx_read	super;

	pmx_header	header;
	pmx_info	info;
	pmx_isize	isize;
	pmx_minfo	minfo;

	pmx_int		offsetVertex;
	pmx_int		offsetFace;
	pmx_int		offsetTexture;
	pmx_int		offsetMaterial;
	
	pmx_int		countVertex;
	pmx_int		countFace;
	pmx_int		countTexture;
	pmx_int		countMaterial;

	pmx_byte *	scratch;
} pmx_read_private;

typedef struct pmx_read_file_s {
	pmx_read_private	super;
	FILE *				file;
} pmx_read_file;

typedef struct pmx_read_buffer_s {
	pmx_read_private	super;
	void *				start;
	void *				end;
	void *				cur;
} pmx_read_buffer;

typedef struct pmx_scratch_s {
	pmx_byte **	pointers;
	pmx_int		count;
} pmx_scratch;

static pmx_byte * pmx_read_scratch_alloc( pmx_read * const _read, const pmx_int _size ) {
	pmx_scratch * scratch;
	pmx_int count;
	pmx_byte ** pointers;
	pmx_read_private * const read = ( pmx_read_private * )_read;
	
	if ( !read->scratch ) {
		read->scratch = ( pmx_byte * )pmx_alloc( PMX_NULL, sizeof( pmx_scratch ) );
		memset( read->scratch, 0, sizeof( pmx_scratch ) );
	}

	scratch = ( pmx_scratch * )read->scratch;
	count = scratch->count + 1;
	pointers = ( pmx_byte ** )pmx_alloc( scratch->pointers, count * sizeof( pmx_byte * ) );

	if ( pointers ) {
		pmx_byte * const page = ( pmx_byte * )pmx_alloc( PMX_NULL, _size );

		pointers[scratch->count] = page;

		scratch->pointers = pointers;
		scratch->count = count;

		return page;
	}

	return PMX_NULL;
}

static pmx_int pmx_read_scratch_dealloc( pmx_read * const _read ) {
	pmx_scratch * scratch;
	pmx_read_private * const read = ( pmx_read_private * )_read;

	if ( !read->scratch ) {
		return;
	}

	scratch = ( pmx_scratch * )read->scratch;

	while ( scratch->count-- ) {
		pmx_alloc( scratch->pointers[scratch->count], 0 );
	}

	pmx_alloc( scratch->pointers, 0 );
	pmx_alloc( read->scratch, 0 );
	read->scratch = PMX_NULL;

	return 0;
}

static pmx_int pmx_read_file_fread( pmx_read_file * const _read, void * const _dest, const pmx_int _length ) {
	return ( pmx_int )fread( _dest, 1, ( size_t )_length, _read->file );
}

static pmx_int pmx_read_file_fseek( pmx_read_file * const _read, const pmx_int _offset, const pmx_int _origin ) {
	switch ( _origin ) {
	case PMX_READ_SEEK_SET:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_SET );
	case PMX_READ_SEEK_CUR:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_CUR );
	case PMX_READ_SEEK_END:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_END );
	}
	return -1;
}

static pmx_int pmx_read_file_ftell( pmx_read_file * const _read ) {
	return ( pmx_int )ftell( _read->file );
}

static pmx_int pmx_read_buffer_fread( pmx_read_buffer * const _read, void * const _dest, const pmx_int _length ) {
	const pmx_int length = ( pmx_int )_read->end - ( pmx_int )_read->cur;
	return ( pmx_int )memcpy( _dest, _read->cur, ( size_t )( _length < length ? _length : length ) );
}

static pmx_int pmx_read_buffer_fseek( pmx_read_buffer * const _read, const pmx_int _offset, const pmx_int _origin ) {
	void * newPos;

	switch ( _origin ) {
	case PMX_READ_SEEK_SET:
		newPos = pmx_ptr_add( _read->start, _offset );
        break;
	case PMX_READ_SEEK_CUR:
        newPos = pmx_ptr_add( _read->cur, _offset );
        break;
	case PMX_READ_SEEK_END:
        newPos = pmx_ptr_add( _read->end, _offset );
        break;
    default:
        newPos = PMX_NULL;
        break;
	}
	
	if ( newPos <= _read->end && newPos >= _read->start ) {
		_read->cur = newPos;
		return 0;
	}
	return -1;
}

static pmx_int pmx_read_buffer_ftell( pmx_read_buffer * const _read ) {
	return ( pmx_int )pmx_ptr_add( _read->cur, -( pmx_int )_read->start );
}

pmx_byte pmx_read_text( pmx_read * const _read, pmx_text * const _text ) {
	_read->fread_f( _read, &_text->size, sizeof( _text->size ) );
	if ( _text->size ) {
		_text->string.utf8 = ( pmx_byte * )pmx_alloc( PMX_NULL, _text->size );
		_read->fread_f( _read, _text->string.utf8, _text->size );
	}
	
	return 0;
}

pmx_read * pmx_read_create_file( void * const _file ) {
	pmx_read_file * const read = ( pmx_read_file * )pmx_alloc( PMX_NULL, sizeof( pmx_read_file ) );
	memset( read, 0, sizeof( *read ) );
	
	read->super.super.type = PMX_READ_FILE;
	read->super.super.fread_f = ( pmx_read_fread_t )pmx_read_file_fread;
	read->super.super.fseek_f = ( pmx_read_fseek_t )pmx_read_file_fseek;
	read->super.super.ftell_f = ( pmx_read_ftell_t )pmx_read_file_ftell;

	read->file = ( FILE * )_file;

	return &read->super.super;
}

pmx_read * pmx_read_create_buffer( void * const _buffer, const pmx_int _length ) {
	pmx_read_buffer * const read = ( pmx_read_buffer * )pmx_alloc( PMX_NULL, sizeof( pmx_read_buffer ) );
	memset( read, 0, sizeof( *read ) );
	
	read->super.super.type = PMX_READ_BUFFER;
	read->super.super.fread_f = ( pmx_read_fread_t )pmx_read_buffer_fread;
	read->super.super.fseek_f = ( pmx_read_fseek_t )pmx_read_buffer_fseek;
	read->super.super.ftell_f = ( pmx_read_ftell_t )pmx_read_buffer_ftell;

	read->start = _buffer;
	read->cur = read->start;
	read->end = pmx_ptr_add( read->start, _length );

	return &read->super.super;
}

void pmx_read_destroy( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_read_scratch_dealloc( _read );

	pmx_alloc( read->minfo.name.local.string.utf8, 0 );
	pmx_alloc( read->minfo.name.global.string.utf8, 0 );
	
	pmx_alloc( read->minfo.comment.local.string.utf8, 0 );
	pmx_alloc( read->minfo.comment.global.string.utf8, 0 );

	pmx_alloc( read, 0 );
}

pmx_byte pmx_read_header( pmx_read * const _read, pmx_header * const _header ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	_read->fseek_f( _read, PMX_OFFSET_HEADER, PMX_READ_SEEK_SET );

	_read->fread_f( _read, read->header.signature, sizeof( read->header.signature ) );
	_read->fread_f( _read, &read->header.version, sizeof( read->header.version ) );
	
	if ( _header ) {
		memcpy( _header, &read->header, sizeof( *_header ) );
	}

	return 0;
}

pmx_byte pmx_read_info( pmx_read * const _read, pmx_info * const _info ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	_read->fseek_f( _read, PMX_OFFSET_INFO, PMX_READ_SEEK_SET );

	_read->fread_f( _read, &read->info.dataCount, sizeof( read->info.dataCount ) );
	_read->fread_f( _read, &read->info.textTypeEncoding, sizeof( read->info.textTypeEncoding ) );
	_read->fread_f( _read, &read->info.additionalUVCount, sizeof( read->info.additionalUVCount ) );
	
	if ( _info ) {
		memcpy( _info, &read->info, sizeof( *_info ) );
	}

	pmx_read_isize( _read, PMX_NULL );
	pmx_read_minfo( _read, PMX_NULL );
		
	read->offsetVertex = PMX_OFFSET_MINFO + pmx_minfo_size( &read->minfo );
	read->offsetFace = read->offsetVertex + pmx_read_sizeof_vertex( _read, pmx_read_count_vertex( _read ) );
	read->offsetTexture = read->offsetFace + pmx_read_sizeof_face( _read, pmx_read_count_face( _read ) );
	read->offsetMaterial = read->offsetTexture + pmx_read_sizeof_texture( _read, pmx_read_count_texture( _read ) );

	return 0;
}

pmx_byte pmx_read_isize( pmx_read * const _read, pmx_isize * const _isize ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	_read->fseek_f( _read, PMX_OFFSET_ISIZE, PMX_READ_SEEK_SET );

	_read->fread_f( _read, &read->isize.vertex, sizeof( read->isize.vertex ) );
	_read->fread_f( _read, &read->isize.texture, sizeof( read->isize.texture ) );
	_read->fread_f( _read, &read->isize.material, sizeof( read->isize.material ) );
	_read->fread_f( _read, &read->isize.bone, sizeof( read->isize.bone ) );
	_read->fread_f( _read, &read->isize.morph, sizeof( read->isize.morph ) );
	_read->fread_f( _read, &read->isize.rigidBody, sizeof( read->isize.rigidBody ) );
	
	if ( _isize ) {
		memcpy( _isize, &read->isize, sizeof( *_isize ) );
	}

	return 0;
}

pmx_byte pmx_read_minfo( pmx_read * const _read, pmx_minfo * const _minfo ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	_read->fseek_f( _read, PMX_OFFSET_MINFO, PMX_READ_SEEK_SET );
	
	pmx_read_text( _read, &read->minfo.name.local );
	pmx_read_text( _read, &read->minfo.name.global );
	
	pmx_read_text( _read, &read->minfo.comment.local );
	pmx_read_text( _read, &read->minfo.comment.global );
	
	if ( _minfo ) {
		memcpy( _minfo, &read->minfo, sizeof( *_minfo ) );
	}

	return 0;
}

pmx_int pmx_read_count_vertex( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( read->countVertex < 1 ) {
		pmx_int count = 0;

		if ( read->offsetVertex ) {
			_read->fseek_f( _read, read->offsetVertex, PMX_READ_SEEK_SET );
			_read->fread_f( _read, &count, sizeof( count ) );
		}
	
		read->countVertex = count;
	}
	return read->countVertex;
}

pmx_int pmx_read_count_face( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( read->countFace < 1 ) {
		pmx_int count = 0;
	
		if ( read->offsetFace ) {
			_read->fseek_f( _read, read->offsetFace, PMX_READ_SEEK_SET );
			_read->fread_f( _read, &count, sizeof( count ) );
		}
	
		read->countFace = count / 3;
	}
	return read->countFace;
}

pmx_int pmx_read_count_texture( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( read->countTexture < 1 ) {
		pmx_int count = 0;
	
		if ( read->offsetTexture ) {
			_read->fseek_f( _read, read->offsetTexture, PMX_READ_SEEK_SET );
			_read->fread_f( _read, &count, sizeof( count ) );
		}

		read->countTexture = count;
	}

	return read->countTexture;
}

pmx_int pmx_read_count_material( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( read->countMaterial < 1 ) {
		pmx_int count = 0;
	
		if ( read->offsetMaterial ) {
			_read->fseek_f( _read, read->offsetMaterial, PMX_READ_SEEK_SET );
			_read->fread_f( _read, &count, sizeof( count ) );
		}

		read->countMaterial = count;
	}
	return read->countMaterial;
}

pmx_int pmx_read_sizeof_vertex( pmx_read * const _read, const pmx_int _count ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	const pmx_int offset = read->offsetVertex;
	pmx_int size = 4;
	pmx_int count = _count; // count
	
	if ( count < 0 ) {
		count = pmx_read_count_vertex( _read );
	}

	while ( count-- ) {
		pmx_byte wType;

		size += 32; // pos, norm, uv
		size += 16 * read->info.additionalUVCount; // additional uv

		_read->fseek_f( _read, offset + size, PMX_READ_SEEK_SET );
		size += _read->fread_f( _read, &wType, 1 );

		switch ( wType ) {
		case 0:
			size += read->isize.bone;
			break;
		case 1:
			size += read->isize.bone * 2;
			size += 4;
			break;
		case 2:
			size += read->isize.bone * 4;
			size += 16;
			break;
		case 3:
			size += read->isize.bone * 2;
			size += 40;
			break;
		default:
			pmx_print_error( "read", "Unknown BDEF\n" );
			break;
		}

		size += 4; // edge scale
	}

	return size;
}

pmx_int pmx_read_sizeof_face( pmx_read * const _read, const pmx_int _count ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_int size = 4;
	pmx_int count = _count; // count
	
	if ( count < 0 ) {
		count = pmx_read_count_face( _read );
	}

	size += ( count * read->isize.vertex * 3 );

	return size;
}

static pmx_int pmx_read_sizeof_text( pmx_read * const _read ) {
	pmx_text text;
	pmx_int size;

	text.size = 0;
	text.string.utf8 = PMX_NULL;
	pmx_read_text( _read, &text );

	size = text.size + 4;

	pmx_alloc( text.string.utf8, 0 );

	return size;
}

pmx_int pmx_read_sizeof_texture( pmx_read * const _read, const pmx_int _count ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	const pmx_int offset = read->offsetTexture;
	pmx_int size = 4;
	pmx_int count = _count; // count

	if ( count < 0 ) {
		count = pmx_read_count_texture( _read );
	}
	
	_read->fseek_f( _read, offset + size, PMX_READ_SEEK_SET );

	while ( count-- ) {
		size += pmx_read_sizeof_text( _read ); // Texture file
	}

	return size;
}

pmx_int pmx_read_sizeof_material( pmx_read * const _read, const pmx_int _count ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	const pmx_int offset = read->offsetMaterial;
	pmx_int size = 4;
	pmx_int count = _count; // count

	if ( count < 0 ) {
		count = pmx_read_count_material( _read );
	}
	
	while ( count-- ) {
		pmx_byte toonMode;
		
		_read->fseek_f( _read, offset + size, PMX_READ_SEEK_SET );
		size += pmx_read_sizeof_text( _read ); // Local name
		size += pmx_read_sizeof_text( _read ); // Global name

		size += 65; // diffuse, specular colour, specularity, ambient colour, drawing mode, edge colour, edge size

		size += read->isize.texture * 2; // Texture index, environment index

		size += 1; // Environment mode
			
		_read->fseek_f( _read, offset + size, PMX_READ_SEEK_SET );
		_read->fread_f( _read, &toonMode, 1 );

		size += 1; // Toon mode

		if ( toonMode ) {
			size += 1; // Inbuilt
		} else {
			size += read->isize.texture; // Texture index
		}
		
		_read->fseek_f( _read, offset + size, PMX_READ_SEEK_SET );
		size += pmx_read_sizeof_text( _read ); // Memo

		size += 4; // Face count
	}

	return size;
}

static pmx_int pmx_read_vertex( pmx_read * const _read, pmx_read_data_struct * const _struct ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_int ii = 0;
	const pmx_int startByte = pmx_read_sizeof_vertex( _read, _struct->start );
	pmx_int span = 0;
	pmx_byte order = 0;

	memset( _struct->maskOrder, 0, sizeof( _struct->maskOrder ) );

	if ( ( _struct->mask & PMX_VERTEX_POSITION ) == PMX_VERTEX_POSITION ) {
		_struct->maskOrder[order++] = PMX_VERTEX_POSITION;
		span += 12;
	}
	if ( ( _struct->mask & PMX_VERTEX_NORMAL ) == PMX_VERTEX_NORMAL ) {
		_struct->maskOrder[order++] = PMX_VERTEX_NORMAL;
		span += 12;
	}
	if ( ( _struct->mask & PMX_VERTEX_UV ) == PMX_VERTEX_UV ) {
		_struct->maskOrder[order++] = PMX_VERTEX_UV;
		span += 8;
	}
	if ( ( _struct->mask & PMX_VERTEX_ADDITIONAL_UV ) == PMX_VERTEX_ADDITIONAL_UV ) {
		_struct->maskOrder[order++] = PMX_VERTEX_ADDITIONAL_UV;
		span += 16 * read->info.additionalUVCount;
	}
	if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
		_struct->maskOrder[order++] = PMX_VERTEX_WEIGHT_TYPE;
		span += 1;
	}
	if ( ( _struct->mask & PMX_VERTEX_WEIGHT_DEF ) == PMX_VERTEX_WEIGHT_DEF ) {
		_struct->maskOrder[order++] = PMX_VERTEX_WEIGHT_DEF;
		span += 42;
	}
	if ( ( _struct->mask & PMX_VERTEX_EDGE_SCALE ) == PMX_VERTEX_EDGE_SCALE ) {
		_struct->maskOrder[order++] = PMX_VERTEX_EDGE_SCALE;
		span += 4;
	}

	_read->fseek_f( _read, read->offsetVertex + startByte, PMX_READ_SEEK_SET );	

	while ( ii < _struct->count ) {
		pmx_int index = ii * span;
		pmx_byte wType;

		if ( ( _struct->mask & PMX_VERTEX_POSITION ) == PMX_VERTEX_POSITION ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 12 );
		} else {
			_read->fseek_f( _read, 12, PMX_READ_SEEK_CUR );
		}

		if ( ( _struct->mask & PMX_VERTEX_NORMAL ) == PMX_VERTEX_NORMAL ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 12 );
		} else {
			_read->fseek_f( _read, 12, PMX_READ_SEEK_CUR );
		}
		
		if ( ( _struct->mask & PMX_VERTEX_UV ) == PMX_VERTEX_UV ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 8 );
		} else {
			_read->fseek_f( _read, 8, PMX_READ_SEEK_CUR );
		}

		if ( ( _struct->mask & PMX_VERTEX_ADDITIONAL_UV ) == PMX_VERTEX_ADDITIONAL_UV ) {
			pmx_byte uvCount = read->info.additionalUVCount;
			while ( uvCount-- ) {
				index += _read->fread_f( _read, &_struct->buffer[index], 16 );
			}
		} else {
			_read->fseek_f( _read, 16 * read->info.additionalUVCount, PMX_READ_SEEK_CUR );
		}
		
		_read->fread_f( _read, &wType, 1 );
		
		if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
			_struct->buffer[index] = wType;
			index++;
		}
		
		if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
			switch ( wType ) {
			case 0:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone );
				break;
			case 1:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 2 + 4 );
				break;
			case 2:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 4 + 16 );
				break;
			case 3:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 2 + 40 );
				break;
			default:
				pmx_print_error( "read", "Unknown BDEF\n" );
				break;
			}
		} else {
			switch ( wType ) {
			case 0:
				_read->fseek_f( _read, read->isize.bone, PMX_READ_SEEK_CUR );
				break;
			case 1:
				_read->fseek_f( _read, read->isize.bone * 2 + 4, PMX_READ_SEEK_CUR );
				break;
			case 2:
				_read->fseek_f( _read, read->isize.bone * 4 + 16, PMX_READ_SEEK_CUR );
				break;
			case 3:
				_read->fseek_f( _read, read->isize.bone * 2 + 40, PMX_READ_SEEK_CUR );
				break;
			default:
				pmx_print_error( "read", "Unknown BDEF\n" );
				break;
			}
		}

		if ( ( _struct->mask & PMX_VERTEX_EDGE_SCALE ) == PMX_VERTEX_EDGE_SCALE ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 4 );
		} else {
			_read->fseek_f( _read, 4, PMX_READ_SEEK_CUR );
		}

		ii++;
	}
	
	return _struct->count * span;
}

pmx_int pmx_read_data( pmx_read * const _read, pmx_read_data_struct * const _struct ) {
	switch ( _struct->type ) {
	case PMX_DATA_VERTEX:
		return pmx_read_vertex( _read, _struct );
	}
	return 0;
}

static const char * pmx_read_alloc_str( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_text text;
	char * outChars = PMX_NULL;

	pmx_read_text( _read, &text );

	if ( read->info.textTypeEncoding ) {
		outChars = ( char * )pmx_read_scratch_alloc( _read, text.size + 1 );

		memcpy( outChars, text.string.utf8, text.size );
		outChars[text.size] = 0;

		pmx_alloc( text.string.utf8, 0 );
	} else {
		pmx_int ii;
		const pmx_int hSize = text.size / 2;

		outChars = ( char * )pmx_read_scratch_alloc( _read, hSize + 1 );

		for ( ii = 0; ii < hSize; ii++ ) {
			outChars[ii] = text.string.utf8[ii * 2];
		}

		outChars[hSize] = 0;
	}

	return outChars;
}

static const wchar_t * pmx_read_alloc_wstr( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_text text;
	wchar_t * outChars = PMX_NULL;

	pmx_read_text( _read, &text );

	if ( read->info.textTypeEncoding ) {
		pmx_int ii;

		outChars = ( wchar_t * )pmx_read_scratch_alloc( _read, ( text.size * 2 ) + 2 );

		for ( ii = 0; ii < text.size; ii++ ) {
			outChars[ii] = ( wchar_t )text.string.utf8[ii];
		}
		outChars[text.size * 2] = 0;

		pmx_alloc( text.string.utf8, 0 );
	} else {
		outChars = ( wchar_t * )pmx_read_scratch_alloc( _read, text.size + 2 );

		memcpy( outChars, text.string.utf16, text.size );
		outChars[text.size / 2] = 0;
	}

	return outChars;
}

static const char * pmx_read_gets_texture( pmx_read * const _read, const pmx_int _index ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( _index < read->countTexture ) {
		const pmx_int offset = pmx_read_sizeof_texture( _read, _index );
		_read->fseek_f( _read, read->offsetTexture + offset, PMX_READ_SEEK_SET );

		return pmx_read_alloc_str( _read );
	}

	return PMX_NULL;
}

static const wchar_t * pmx_read_getls_texture( pmx_read * const _read, const pmx_int _index ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( _index < read->countTexture ) {
		const pmx_int offset = pmx_read_sizeof_texture( _read, _index );
		_read->fseek_f( _read, read->offsetTexture + offset, PMX_READ_SEEK_SET );

		return pmx_read_alloc_wstr( _read );
	}

	return PMX_NULL;
}

static const char * pmx_read_gets_minfo( pmx_read * const _read, const pmx_int _var ) {
	pmx_int nameLocal, nameGlobal, commentLocal;
	
	_read->fseek_f( _read, PMX_OFFSET_MINFO, PMX_READ_SEEK_SET );
	if ( _var == PMX_MINFO_NAME_LOCAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &nameLocal, 4 );
	_read->fseek_f( _read, nameLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_NAME_GLOBAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &nameGlobal, 4 );
	_read->fseek_f( _read, nameGlobal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_LOCAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &commentLocal, 4 );
	_read->fseek_f( _read, commentLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_GLOBAL ) {
		return pmx_read_alloc_str( _read );
	}
	
	return PMX_NULL;
}

static const wchar_t * pmx_read_getls_minfo( pmx_read * const _read, const pmx_int _var ) {
	pmx_int nameLocal, nameGlobal, commentLocal;
	
	_read->fseek_f( _read, PMX_OFFSET_MINFO, PMX_READ_SEEK_SET );
	if ( _var == PMX_MINFO_NAME_LOCAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &nameLocal, 4 );
	_read->fseek_f( _read, nameLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_NAME_GLOBAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &nameGlobal, 4 );
	_read->fseek_f( _read, nameGlobal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_LOCAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &commentLocal, 4 );
	_read->fseek_f( _read, commentLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_GLOBAL ) {
		return pmx_read_alloc_wstr( _read );
	}
	
	return PMX_NULL;
}

const char * pmx_read_gets( pmx_read * const _read, const pmx_int _type, const pmx_int _var, const pmx_int _index ) {
	switch ( _type ) {
	case PMX_DATA_MINFO:
		return pmx_read_gets_minfo( _read, _var );
	case PMX_DATA_TEXTURE:
		return pmx_read_gets_texture( _read, _index );
	}
	return PMX_NULL;
}

const wchar_t * pmx_read_getls( pmx_read * const _read, const pmx_int _type, const pmx_int _var, const pmx_int _index ) {
	switch ( _type ) {
	case PMX_DATA_MINFO:
		return pmx_read_getls_minfo( _read, _var );
	case PMX_DATA_TEXTURE:
		return pmx_read_getls_texture( _read, _index );
	}
	return PMX_NULL;
}
