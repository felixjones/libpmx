#include <libpmx/pmx.h>
#include <stdio.h>

int main( int argc, char ** argv ) {
	FILE * const fp = fopen( "IA/IA.pmx", "r" );

	if ( fp ) {
		pmx_reader * const reader = pmx_reader_create_file( fp );
		pmx_header header;
		pmx_int err;

		err = pmx_reader_header( reader, &header );

		err = pmx_header_check( &header );
		
		pmx_reader_destroy( reader );

		fclose( fp );
	}
	
	return 0;
}
