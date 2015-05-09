#include "pmx.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

/*
===============================================================================

	Vec2

===============================================================================
*/
typedef struct pmx_vec2_s {
	float	x;
	float	y;
} pmx_vec2_t;

/*
===============================================================================

	Vec3

===============================================================================
*/
typedef struct pmx_vec3_s {
	float	x;
	float	y;
	float	z;
} pmx_vec3_t;

/*
===============================================================================

	Vec4

===============================================================================
*/
typedef struct pmx_vec4_s {
	float	x;
	float	y;
	float	z;
	float	w;
} pmx_vec4_t;

typedef struct pmx_weight_deform_s *	pmx_weight_deform;

/*
===============================================================================

	Vertex data

===============================================================================
*/
struct pmx_vertex_data_s {
	pmx_vec3_t			position;
	pmx_vec3_t			normal;
	pmx_vec2_t			uv;
	pmx_vec4_t *		additional_uv;
	char				weight_deform_type;
	pmx_weight_deform *	weight_deform;
	float				edge_scale;
};

typedef struct pmx_vertex_data_s *	pmx_vertex_data;

/*
===============================================================================

	libpmx

===============================================================================
*/
struct pmx_struct_s {
	pmx_log_f		log_error;
	pmx_log_f		log_warning;
	void *			log_user_ptr;
	void *			io_ptr;
	pmx_io_read_f	io_read;
	pmx_io_seek_f	io_seek;
	size_t			io_cur;
	pmx_alloc_f		alloc;
};

/*
===============================================================================

	PMX model info

===============================================================================
*/
struct pmx_info_s {
	char			global_count;
	char *			globals;
	int				local_name_len;
	char *			local_name;
	int				universal_name_len;
	char *			universal_name;
	int				local_comment_len;
	char *			local_comment;
	int				universal_comment_len;
	char *			universal_comment;
	int				vertex_data_count;
	int				vertex_data_len;
	pmx_vertex_data	vertex_data;
};

static const char PMX_SIGNATURE[] = { 0x50, 0x4D, 0x58, 0x20 };
static const float PMX_VALID_VER[] = { 2.0f, 2.1f };

/*
====================
pmx_float_cmp

	Compares floats within range of FLT_EPSILON
====================
*/
__inline static int pmx_float_cmp( const float _a, const float _b ) {
	return ( fabsf( _a - _b ) > FLT_EPSILON );
}

/*
====================
pmx_ver_index

	Returns an index from 0 to index count of PMX_VALID_VER
====================
*/
__inline static int pmx_ver_index( const float _ver ) {
	int ii;
	for ( ii = 0; ii < 2; ii++ ) {
		if ( pmx_float_cmp( _ver, PMX_VALID_VER[ii] ) == 0 ) {
			return ii;
		}
	}
	return -1;
}

/*
====================
pmx_std_fread

	std::fread forward op
====================
*/
static size_t pmx_std_fread( void * const _dest, const size_t _size, void * const _io_ptr ) {
	return fread( _dest, 1, _size, _io_ptr );
}

/*
====================
pmx_std_fseek

	std::fseek forward op
====================
*/
static int pmx_std_fseek( void * const _io_ptr, const long int _offset, const int _origin ) {
	return fseek( _io_ptr, _offset, _origin );
}

/*
====================
pmx_std_alloc

	Either reallocs or frees memory
====================
*/
static void * pmx_std_alloc( void * const _ptr, const size_t _size ) {
	if ( _size ) {
		return realloc( _ptr, _size );
	}
	free( _ptr );
	return NULL;
}

/*
====================
pmx_header_version

	Returns PMX version from a header
====================
*/
float pmx_header_version( char * const _bytes, const int _length ) {
	if ( _length < PMX_HEAD_LEN ) {
		goto HEADER_FAIL;
	}

	if ( memcmp( _bytes, PMX_SIGNATURE, 4 ) == 0 ) {
		const float version = *( float * )( _bytes + 4 );
		const int version_index = pmx_ver_cmp( version );

		if ( version_index < 0 ) {
			goto HEADER_FAIL;
		}

		return PMX_VALID_VER[version_index];
	}
HEADER_FAIL:
	return 0.0f;
}

/*
====================
pmx_create_read_struct

	Returns a structure made for READING a PMX file
====================
*/
pmx_struct pmx_create_read_struct( const float _version, pmx_log_f _log_error, pmx_log_f _log_warning, void * const _log_uptr ) {
	return pmx_create_read_struct_alloc( _version, _log_error, _log_warning, _log_uptr, pmx_std_alloc );
}

/*
====================
pmx_create_read_struct_alloc

	pmx_create_read_struct with a custom allocator
====================
*/
pmx_struct pmx_create_read_struct_alloc( const float _version, pmx_log_f _log_error, pmx_log_f _log_warning, void * const _log_uptr, pmx_alloc_f _alloc ) {
	pmx_struct const pmx = ( pmx_struct )_alloc( NULL, sizeof( struct pmx_struct_s ) );

	pmx->log_error = _log_error;
	pmx->log_warning = _log_warning;
	pmx->log_user_ptr = _log_uptr;

	pmx->io_ptr = NULL;
	pmx->io_read = pmx_std_fread;
	pmx->io_seek = pmx_std_fseek;
	pmx->io_cur = 0;

	pmx->alloc = _alloc;

	return pmx;
}

/*
====================
pmx_destroy_read_struct

	Deallocates a given read struct
====================
*/
void pmx_destroy_read_struct( pmx_struct _pmx ) {
	_pmx->alloc( _pmx, 0 );
}

/*
====================
pmx_create_info_struct

	Allocates a global info structure
====================
*/
pmx_info pmx_create_info_struct( pmx_struct _pmx ) {
	pmx_info const info = ( pmx_info )_pmx->alloc( NULL, sizeof( struct pmx_info_s ) );
	
	info->global_count = 0;
	info->globals = NULL;
	
	info->local_name_len = 0;
	info->local_name = NULL;
	info->universal_name_len = 0;
	info->universal_name = NULL;
	
	info->local_comment_len = 0;
	info->local_comment = NULL;
	info->universal_comment_len = 0;
	info->universal_comment = NULL;

	info->vertex_data_count = 0;
	info->vertex_data_len = 0;
	info->vertex_data = NULL;

	return info;
}

/*
====================
pmx_destroy_info_struct

	Deallocates a global info structure
====================
*/
void pmx_destroy_info_struct( pmx_struct _pmx, pmx_info _info ) {
	_pmx->alloc( _info->globals, 0 );
	
	_pmx->alloc( _info->local_name, 0 );
	_pmx->alloc( _info->universal_name, 0 );
	
	_pmx->alloc( _info->local_comment, 0 );
	_pmx->alloc( _info->universal_comment, 0 );

	_pmx->alloc( _info, 0 );
}

/*
====================
pmx_init_io

	Sets the internal io "file" pointer
====================
*/
void pmx_init_io( pmx_struct _pmx, void * const _io_ptr ) {
	_pmx->io_ptr = _io_ptr;
}

/*
====================
pmx_set_head_bytes

	Moves the io cursor to describe how many header "bytes" have been read
====================
*/
void pmx_set_head_bytes( pmx_struct _pmx, const char _bytes ) {
	_pmx->io_cur = _bytes;
}

/*
====================
pmx_read_info

	Reads an entire info struct from current PMX file
====================
*/
void pmx_read_info( pmx_struct _pmx, pmx_info _info ) {
	_pmx->io_seek( _pmx->io_ptr, PMX_HEAD_LEN - ( long int )_pmx->io_cur, SEEK_CUR );
	_pmx->io_cur = PMX_HEAD_LEN;

	_pmx->io_cur += _pmx->io_read( &_info->global_count, 1, _pmx->io_ptr );
	_info->globals = _pmx->alloc( _info->globals, _info->global_count );
	if ( _info->globals ) {
		_pmx->io_cur += _pmx->io_read( _info->globals, _info->global_count, _pmx->io_ptr );
	}
	
	_pmx->io_cur += _pmx->io_read( &_info->local_name_len, 4, _pmx->io_ptr );
	_info->local_name = _pmx->alloc( _info->local_name, _info->local_name_len );
	if ( _info->local_name ) {
		_pmx->io_cur += _pmx->io_read( _info->local_name, _info->local_name_len, _pmx->io_ptr );
	}
	
	_pmx->io_cur += _pmx->io_read( &_info->universal_name_len, 4, _pmx->io_ptr );
	_info->universal_name = _pmx->alloc( _info->universal_name, _info->universal_name_len );
	if ( _info->universal_name ) {
		_pmx->io_cur += _pmx->io_read( _info->universal_name, _info->universal_name_len, _pmx->io_ptr );
	}
	
	_pmx->io_cur += _pmx->io_read( &_info->local_comment_len, 4, _pmx->io_ptr );
	_info->local_comment = _pmx->alloc( _info->local_comment, _info->local_comment_len );
	if ( _info->local_comment ) {
		_pmx->io_cur += _pmx->io_read( _info->local_comment, _info->local_comment_len, _pmx->io_ptr );
	}
	
	_pmx->io_cur += _pmx->io_read( &_info->universal_comment_len, 4, _pmx->io_ptr );
	_info->universal_comment = _pmx->alloc( _info->universal_comment, _info->universal_comment_len );
	if ( _info->universal_comment ) {
		_pmx->io_cur += _pmx->io_read( _info->universal_comment, _info->universal_comment_len, _pmx->io_ptr );
	}

}

/*
====================
pmx_get_global

	Returns a global byte variable
====================
*/
char pmx_get_global( pmx_struct _pmx, pmx_info _info, const char _index ) {
	if ( !_info->globals ) {
		pmx_read_info( _pmx, _info );
	}
	return _info->globals[_index];
}

/*
====================
pmx_read_text

	Reads text into a UTF8 string
====================
*/
int pmx_read_text( const char _enc, char * const _dest, const int _dest_len, const char * const _src, const int _src_len ) {
	if ( _enc ) {
		// utf8
		const int read_len = ( _dest_len - 1 < _src_len ? _dest_len - 1 : _src_len );
		int ii;
		for ( ii = 0; ii < read_len; ii++ ) {
			_dest[ii] = _src[ii];
		}
		_dest[read_len] = 0;

		return read_len + 1;
	} else {
		// utf16le
		const int src_len16 = _src_len >> 1;
		const short * const src16 = ( short * )_src;

		const int read_len = ( _dest_len - 1 < src_len16 ? _dest_len - 1 : src_len16 );
		int ii;
		for ( ii = 0; ii < read_len; ii++ ) {
			_dest[ii] = ( char )src16[ii];
		}
		_dest[read_len] = 0;

		return read_len + 1;
	}
}

/*
====================
pmx_read_text

	Reads text into a UTF16LE string
====================
*/
int pmx_read_text_16le( const char _enc, short * const _dest, const int _dest_len, const short * const _src, const int _src_len ) {
	if ( _enc ) {
		// utf8
		const int dest_len8 = _dest_len >> 1;
		const char * const src8 = ( char * )_src;

		const int read_len = ( dest_len8 - 1 < _src_len ? dest_len8 - 1 : _src_len );
		int ii;
		for ( ii = 0; ii < read_len; ii++ ) {
			_dest[ii] = src8[ii];
		}
		_dest[read_len] = 0;

		return read_len + 1;
	} else {
		// utf16le
		const int read_len = ( _dest_len - 2 < _src_len ? _dest_len - 2 : _src_len ) >> 1;
		int ii;
		for ( ii = 0; ii < read_len; ii++ ) {
			_dest[ii] = _src[ii];
		}
		_dest[read_len] = 0;

		return read_len + 1;
	}
}

/*
====================
pmx_read_local_name

	Reads the local (usually Japanese) PMX name into a given UTF8 buffer
====================
*/
int pmx_read_local_name( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length ) {
	if ( !_info->local_name ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->local_name_len : _info->local_name_len >> 1 );
	}
	return pmx_read_text( _info->globals[0], _buffer, _length, _info->local_name, _info->local_name_len );
}

/*
====================
pmx_read_universal_name

	Reads the universal (usually English) PMX name into a given UTF8 buffer
====================
*/
int pmx_read_universal_name( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length ) {
	if ( !_info->universal_name ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->universal_name_len : _info->universal_name_len >> 1 );
	}
	return pmx_read_text( _info->globals[0], _buffer, _length, _info->universal_name, _info->universal_name_len );
}

/*
====================
pmx_read_local_comment

	Reads the local (usually Japanese) PMX comment into a given UTF8 buffer
====================
*/
int pmx_read_local_comment( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length ) {
	if ( !_info->local_comment ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->local_comment_len : _info->local_comment_len >> 1 );
	}
	return pmx_read_text( _info->globals[0], _buffer, _length, _info->local_comment, _info->local_comment_len );
}

/*
====================
pmx_read_universal_comment

	Reads the universal (usually English) PMX comment into a given UTF8 buffer
====================
*/
int pmx_read_universal_comment( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length ) {
	if ( !_info->universal_comment ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->universal_comment_len : _info->universal_comment_len >> 1 );
	}
	return pmx_read_text( _info->globals[0], _buffer, _length, _info->universal_comment, _info->universal_comment_len );
}

/*
====================
pmx_read_local_name_16le

	Reads the local (usually Japanese) PMX name into a given UTF16LE buffer
====================
*/
int pmx_read_local_name_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length ) {
	if ( !_info->local_name ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->local_name_len : _info->local_name_len >> 1 );
	}
	return pmx_read_text_16le( _info->globals[0], _buffer, _length, ( short * )_info->local_name, _info->local_name_len );
}

/*
====================
pmx_read_universal_name_16le

	Reads the universal (usually English) PMX name into a given UTF16LE buffer
====================
*/
int pmx_read_universal_name_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length ) {
	if ( !_info->universal_name ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->universal_name_len : _info->universal_name_len >> 1 );
	}
	return pmx_read_text_16le( _info->globals[0], _buffer, _length, ( short * )_info->universal_name, _info->universal_name_len );
}

/*
====================
pmx_read_local_comment_16le

	Reads the local (usually Japanese) PMX name into a given UTF8 buffer
====================
*/
int pmx_read_local_comment_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length ) {
	if ( !_info->local_comment ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->local_comment_len : _info->local_comment_len >> 1 );
	}
	return pmx_read_text_16le( _info->globals[0], _buffer, _length, ( short * )_info->local_comment, _info->local_comment_len );
}

/*
====================
pmx_read_universal_comment_16le

	Reads the universal (usually English) PMX name into a given UTF8 buffer
====================
*/
int pmx_read_universal_comment_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length ) {
	if ( !_info->universal_comment ) {
		pmx_read_info( _pmx, _info );
	}
	if ( !_buffer ) {
		return ( _info->globals[0] ? _info->universal_comment_len : _info->universal_comment_len >> 1 );
	}
	return pmx_read_text_16le( _info->globals[0], _buffer, _length, ( short * )_info->universal_comment, _info->universal_comment_len );
}
