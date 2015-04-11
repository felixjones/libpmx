#ifndef __PMX_H__
#define __PMX_H__

#include <stddef.h>

#define PMX_HEAD_LEN	( 8 )

typedef struct pmx_struct_s *	pmx_struct;
typedef struct pmx_info_s *		pmx_info;
typedef void					( * pmx_log_f )( pmx_struct _pmx, const char * const _message, void * const _user_ptr );
typedef int						( * pmx_io_seek_f )( void * const _io_ptr, const long int _offset, const int _origin );
typedef size_t					( * pmx_io_read_f )( void * const _dest, const size_t _size, void * const _io_ptr );
typedef void *					( * pmx_alloc_f )( void * const _ptr, const size_t _size );

float		pmx_header_version( char * const _bytes, const int _length );

pmx_struct	pmx_create_read_struct( const float _version, pmx_log_f _log_error, pmx_log_f _log_warning, void * const _log_uptr );
pmx_struct	pmx_create_read_struct_alloc( const float _version, pmx_log_f _log_error, pmx_log_f _log_warning, void * const _log_uptr, pmx_alloc_f _alloc );
void		pmx_destroy_read_struct( pmx_struct _pmx );

pmx_info	pmx_create_info_struct( pmx_struct _pmx );
void		pmx_destroy_info_struct( pmx_struct _pmx, pmx_info _info );

void		pmx_init_io( pmx_struct _pmx, void * const _io_ptr );
void		pmx_set_head_bytes( pmx_struct _pmx, const char _bytes );
void		pmx_read_info( pmx_struct _pmx, pmx_info _info );

char		pmx_get_global( pmx_struct _pmx, pmx_info _info, const char _index );
int			pmx_read_text( const char _enc, char * const _dest, const int _dest_len, const char * const _src, const int _src_len );
int			pmx_read_text_16le( const char _enc, short * const _dest, const int _dest_len, const short * const _src, const int _src_len );

int			pmx_read_local_name( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length );
int			pmx_read_universal_name( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length );
int			pmx_read_local_comment( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length );
int			pmx_read_universal_comment( pmx_struct _pmx, pmx_info _info, char * const _buffer, const int _length );

int			pmx_read_local_name_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length );
int			pmx_read_universal_name_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length );
int			pmx_read_local_comment_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length );
int			pmx_read_universal_comment_16le( pmx_struct _pmx, pmx_info _info, short * const _buffer, const int _length );

__inline static char pmx_get_text_encoding( pmx_struct _pmx, pmx_info _info ) {
	return pmx_get_global( _pmx, _info, 0 );
}

__inline static char pmx_get_additional_uv_count( pmx_struct _pmx, pmx_info _info ) {
	return pmx_get_global( _pmx, _info, 1 );
}

__inline static char pmx_get_vertex_index_size( pmx_struct _pmx, pmx_info _info ) {
	return pmx_get_global( _pmx, _info, 2 );
}

__inline static char pmx_get_texture_index_size( pmx_struct _pmx, pmx_info _info ) {
	return pmx_get_global( _pmx, _info, 3 );
}

__inline static char pmx_get_material_index_size( pmx_struct _pmx, pmx_info _info ) {
	return pmx_get_global( _pmx, _info, 4 );
}

#endif