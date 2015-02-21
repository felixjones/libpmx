#include <libpmx/pmx.h>
#include <libpmx/pmx_read.h>

#include <stdio.h>
#include <math.h>

static int CountBits( int x ) {
    int n = 0;
    if ( x ) {
		do {
			n++;
		} while( ( x = x & ( x - 1 ) ) );
	}
    return n;
}   

int main( int argc, char ** argv ) {
	FILE * const fp = fopen( "IA/IA.pmx", "rb" );

	if ( fp ) {
		pmx_read * const read = pmx_read_create_file( fp );
		pmx_header header;
		pmx_int err;
		
		err = pmx_read_header( read, &header );
		if ( pmx_header_check( &header ) == 0 ) {
			if ( pmx_read_info( read, PMX_NULL ) == 0 ) {
				const pmx_int vCount = pmx_read_count_vertex( read );
				pmx_read_data_struct vertexRead;
				float * const buffer = ( float * )pmx_alloc( PMX_NULL, sizeof( float ) * 8 * vCount );
				pmx_int ii;
				pmx_byte cc = 0;
				pmx_int offset = 0;

				const char * const commentLocal = pmx_read_gets( read, PMX_DATA_MINFO, PMX_MINFO_COMMENT_LOCAL, 0 );
			
				pmx_int tt;
				const pmx_int texCount = pmx_read_count_texture( read );
				for ( tt = 0; tt < texCount; tt++ ) {
					const char * const text = pmx_read_getls( read, PMX_DATA_TEXTURE, 0, tt );
					pmx_print( "%ls\n", text );
				}

				vertexRead.start = 0;
				vertexRead.count = vCount;
				vertexRead.buffer = ( pmx_byte * )&buffer[0];
				vertexRead.bufferLen = sizeof( float ) * 8 * vCount;
				vertexRead.type = PMX_DATA_VERTEX;
				vertexRead.mask = PMX_VERTEX_POSITION | PMX_VERTEX_NORMAL | PMX_VERTEX_UV;

				pmx_read_data( read, &vertexRead );

				cc = CountBits( vertexRead.mask );
			
				for ( ii = 0; ii < vertexRead.count; ii++ ) {
					pmx_byte jj;
					for ( jj = 0; jj < cc; jj++ ) {
						printf( "%d> ", ii );

						switch ( vertexRead.maskOrder[jj] ) {
						case PMX_VERTEX_POSITION:
							printf( "Position : [%f %f %f]\n", buffer[offset], buffer[offset + 1], buffer[offset + 2] );
							offset += 3;
							break;
						case PMX_VERTEX_NORMAL:
							printf( "Normal : [%f %f %f]\n", buffer[offset], buffer[offset + 1], buffer[offset + 2] );
							offset += 3;
							break;
						case PMX_VERTEX_UV:
							printf( "UV : [%f %f]\n", buffer[offset], buffer[offset + 1] );
							offset += 2;
							break;
						case PMX_VERTEX_EDGE_SCALE:
							printf( "Edge Scale : [%f]\n", buffer[offset] );
							offset += 1;
							break;
						}
					}
				
					printf( "\n" );

				}

				pmx_alloc( buffer, 0 );
			}
		}
		
		pmx_read_destroy( read );

		fclose( fp );
	}
	
	return 0;
}
