#include <libpmx/pmx.h>
#include <libpmx/pmx_read.h>

#include <stdio.h>

int main( int argc, char ** argv ) {
	FILE * const fp = fopen( "IA/IA.pmx", "rb" );

	if ( fp ) {
		pmx_read * const read = pmx_read_create_file( fp );
		pmx_header header;
		pmx_int err;
		
		err = pmx_read_header( read, &header );
		if ( pmx_header_check( &header ) == 0 ) {
			pmx_int vCount = pmx_read_count_vertex( read );
			pmx_int fCount = pmx_read_count_face( read );
			pmx_int tCount = pmx_read_count_texture( read );
			pmx_read_sizeof_texture( read, -1 );
		}
		
		pmx_read_destroy( read );

		fclose( fp );
	}
	
	return 0;
}
