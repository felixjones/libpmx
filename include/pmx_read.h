#ifndef __PMX_READ_H__
#define __PMX_READ_H__

#include <libpmx/pmx.h>
#include <libpmx/pmx_header.h>
#include <libpmx/pmx_info.h>
#include <libpmx/pmx_isize.h>
#include <libpmx/pmx_minfo.h>
#include <libpmx/pmx_vertex.h>
#include <libpmx/pmx_weight.h>

#define PMX_READ_CUSTOM	( ( 'C' << 24 ) | ( 'U' << 16 ) | ( 'S' << 8 ) | 'T' )

enum pmx_read_seek_e {
	PMX_READ_SEEK_SET,
	PMX_READ_SEEK_CUR,
	PMX_READ_SEEK_END
};

enum pmx_data_e {
	PMX_DATA_VERTEX		= ( ( 'V' << 24 ) | ( 'R' << 16 ) | ( 'T' << 8 ) | 'X' ),
	PMX_DATA_FACE		= ( ( 'F' << 24 ) | ( 'A' << 16 ) | ( 'C' << 8 ) | 'E' ),
	PMX_DATA_TEXTURE	= ( ( 'T' << 24 ) | ( 'X' << 16 ) | ( 'U' << 8 ) | 'E' ),
	PMX_DATA_MATERIAL	= ( ( 'M' << 24 ) | ( 'T' << 16 ) | ( 'I' << 8 ) | 'L' ),
	PMX_DATA_BONE		= ( ( 'B' << 24 ) | ( 'O' << 16 ) | ( 'N' << 8 ) | 'E' ),
	PMX_DATA_MORPH		= ( ( 'M' << 24 ) | ( 'O' << 16 ) | ( 'P' << 8 ) | 'H' ),
	PMX_DATA_FRAME		= ( ( 'F' << 24 ) | ( 'R' << 16 ) | ( 'M' << 8 ) | 'E' ),
	PMX_DATA_RIGID		= ( ( 'R' << 24 ) | ( 'I' << 16 ) | ( 'I' << 8 ) | 'D' ),
	PMX_DATA_JOINT		= ( ( 'J' << 24 ) | ( 'O' << 16 ) | ( 'N' << 8 ) | 'T' )
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
	pmx_int				offsetMaterial;
	
	pmx_int				countVertex;
	pmx_int				countFace;
	pmx_int				countTexture;
	pmx_int				countMaterial;
} pmx_read;

typedef struct pmx_read_data_struct_s {
	pmx_int		type;
	pmx_int		start;
	pmx_int		count;
	pmx_int		mask;
	pmx_int		maskOrder[32];
	pmx_byte *	buffer;
	pmx_int		bufferLen;
} pmx_read_data_struct;

typedef struct pmx_read_text_struct_s {
	pmx_int		type;
	pmx_int		index;
	pmx_text *	text;
} pmx_read_text_struct;

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
pmx_int		pmx_read_count_material( pmx_read * const _read );

pmx_int		pmx_read_sizeof_vertex( pmx_read * const _read, const pmx_int _count );
pmx_int		pmx_read_sizeof_face( pmx_read * const _read, const pmx_int _count );
pmx_int		pmx_read_sizeof_texture( pmx_read * const _read, const pmx_int _count );
pmx_int		pmx_read_sizeof_material( pmx_read * const _read, const pmx_int _count );

pmx_int		pmx_read_data( pmx_read * const _read, pmx_read_data_struct * const _struct );
pmx_int		pmx_read_data_text( pmx_read * const _read, pmx_read_text_struct * const _struct );

#endif