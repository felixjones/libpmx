#ifndef __PMX_READER_H__
#define __PMX_READER_H__

#include <libpmx/pmx.h>

#define PMX_READER_CUSTOM	( ( 'C' << 24 ) | ( 'U' << 16 ) | ( 'S' << 8 ) | 'T' )

typedef pmx_int	( * pmx_reader_read_t )( struct pmx_reader_s * const _reader, void * const _buffer, const pmx_int _eSize, const pmx_int _eCount );

typedef struct pmx_reader_s {
	pmx_reader_read_t	read_f;
	pmx_int				type;
} pmx_reader;

pmx_reader *	pmx_reader_create_file( void * const _file );
pmx_reader *	pmx_reader_create_buffer( void * const _buffer, const size_t _length );
void			pmx_reader_destroy( pmx_reader * const _reader );

pmx_int			pmx_reader_header( pmx_reader * const _reader, struct pmx_header_s * const _header );

#endif