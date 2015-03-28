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

#include "pmx_read_static.h"

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

pmx_int pmx_read_data( pmx_read * const _read, pmx_read_data_struct * const _struct ) {
	switch ( _struct->type ) {
	case PMX_DATA_VERTEX:
		return pmx_read_vertex( _read, _struct );
	}
	return 0;
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
