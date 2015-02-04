#ifndef __PMX_READER_H__
#define __PMX_READER_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_text.h>
#include <libpmx/pmx_header.h>
#include <libpmx/pmx_index_size.h>
#include <libpmx/pmx_model_info.h>

#define PMX_READER_CUSTOM	( ( 'C' << 24 ) | ( 'U' << 16 ) | ( 'S' << 8 ) | 'T' )

typedef pmx_int	( * pmx_reader_read_t )( struct pmx_reader_s * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount );

typedef struct pmx_reader_s {
	pmx_reader_read_t	read_f;
	pmx_int				type;
} pmx_reader;

pmx_reader *	pmx_reader_create_file( void * const _file );
pmx_reader *	pmx_reader_create_buffer( void * const _buffer, const size_t _length );
void			pmx_reader_destroy( pmx_reader * const _reader );

pmx_int			pmx_reader_seek_abs( pmx_reader * const _reader, const pmx_int _cur );

pmx_int			pmx_read_text( pmx_reader * const _reader, pmx_text * const _text );

pmx_int			pmx_read_header( pmx_reader * const _reader, pmx_header * const _header );
pmx_int			pmx_read_isize( pmx_reader * const _reader, pmx_isize * const _isize );
pmx_int			pmx_read_model_info( pmx_reader * const _reader, pmx_minfo * const _minfo );

#endif