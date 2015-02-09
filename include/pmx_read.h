#ifndef __PMX_READ_H__
#define __PMX_READ_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_header.h>
#include <libpmx/pmx_info.h>
#include <libpmx/pmx_isize.h>
#include <libpmx/pmx_minfo.h>

#define PMX_READ_CUSTOM	( ( 'C' << 24 ) | ( 'U' << 16 ) | ( 'S' << 8 ) | 'T' )

enum pmx_read_seek_e {
	PMX_READ_SEEK_SET,
	PMX_READ_SEEK_CUR,
	PMX_READ_SEEK_END
};

typedef pmx_int	( * pmx_read_fread_t )( struct pmx_read_s * const _read, void * const _dest, const pmx_int _length );
typedef pmx_int	( * pmx_read_fseek_t )( struct pmx_read_s * const _read, const pmx_int _offset, const pmx_int _origin );
typedef pmx_int	( * pmx_read_ftell_t )( struct pmx_read_s * const _read );

typedef struct pmx_read_s {
	pmx_int				type;

	pmx_read_fread_t	fread_f;
	pmx_read_fseek_t	fseek_f;
	pmx_read_ftell_t	ftell_f;
	
	pmx_header			header;
	pmx_info			info;
	pmx_isize			isize;
	pmx_minfo			minfo;

	pmx_int				offsetVertex;
	pmx_int				offsetFace;
	pmx_int				offsetTexture;
	
	pmx_int				countVertex;
	pmx_int				countFace;
	pmx_int				countTexture;
} pmx_read;

pmx_read *	pmx_read_create_file( void * const _file );
pmx_read *	pmx_read_create_buffer( void * const _buffer, const pmx_int _length );
void		pmx_read_destroy( pmx_read * const _read );

pmx_byte	pmx_read_header( pmx_read * const _read, pmx_header * const _header );
pmx_byte	pmx_read_info( pmx_read * const _read, pmx_info * const _info );
pmx_byte	pmx_read_isize( pmx_read * const _read, pmx_isize * const _isize );
pmx_byte	pmx_read_minfo( pmx_read * const _read, pmx_minfo * const _minfo );

pmx_int		pmx_read_count_vertex( pmx_read * const _read );
pmx_int		pmx_read_count_face( pmx_read * const _read );
pmx_int		pmx_read_count_texture( pmx_read * const _read );

pmx_int		pmx_read_sizeof_vertex( pmx_read * const _read, const pmx_int _count );
pmx_int		pmx_read_sizeof_face( pmx_read * const _read, const pmx_int _count );
pmx_int		pmx_read_sizeof_texture( pmx_read * const _read, const pmx_int _count );

#endif