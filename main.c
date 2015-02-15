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
			const pmx_int vCount = pmx_read_count_vertex( read );
			pmx_read_data_struct vertexRead;
			float * const buffer = ( float * )pmx_alloc( PMX_NULL, sizeof( float ) * vCount * 8 );
			pmx_byte ii = 0;
			pmx_byte cc = 0;
			pmx_short offset = 0;

			const char * const commentLocal = pmx_read_gets( read, PMX_DATA_MINFO, PMX_MINFO_COMMENT_LOCAL, 0 );
			pmx_alloc( commentLocal, 0 );
			
			vertexRead.start = 48;
			vertexRead.count = 100;
			vertexRead.buffer = ( pmx_byte * )&buffer[0];
			vertexRead.bufferLen = sizeof( float ) * vCount * 8;
			vertexRead.type = PMX_DATA_VERTEX;
			vertexRead.mask = PMX_VERTEX_EDGE_SCALE | PMX_VERTEX_NORMAL;

			pmx_read_data( read, &vertexRead );

			cc = CountBits( vertexRead.mask );
			
			while ( ii < vertexRead.count ) {
				pmx_byte jj = 0;

				while ( jj < cc ) {
					printf( "%d> ", ii );

					switch ( vertexRead.maskOrder[jj] ) {
					case PMX_VERTEX_POSITION:
						printf( "Position : [%f %f %f]\n", buffer[offset], buffer[offset + 1], buffer[offset + 2] );
						offset += 3;
						break;
					case PMX_VERTEX_NORMAL:
						{
							float sum = 0.0f;
							sum += buffer[offset] * buffer[offset];
							sum += buffer[offset + 1] * buffer[offset + 1];
							sum += buffer[offset + 2] * buffer[offset + 2];

							printf( "Normal : %f\n", sqrtf( sum ) );

							offset += 3;
						}
						//printf( "%f %f %f\n", buffer[ii].number[3], buffer[ii].number[4], buffer[ii].number[5] );
						break;
					case PMX_VERTEX_UV:
						printf( "UV : " );
						printf( "%f %f\n", buffer[offset], buffer[offset + 1] );
						offset += 2;
						break;
					case PMX_VERTEX_EDGE_SCALE:
						printf( "Edge : " );
						printf( "%f\n", buffer[offset] );
						offset += 1;
						break;
					}

					jj++;
				}
				
				printf( "\n" );

				ii++;
			}

			pmx_alloc( buffer, 0 );
		}
		
		pmx_read_destroy( read );

		fclose( fp );
	}
	
	return 0;
}
