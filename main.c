#include <libpmx/pmx.h>
#include <libpmx/pmx_reader.h>

#include <stdio.h>

int main( int argc, char ** argv ) {
	FILE * const fp = fopen( "IA/IA.pmx", "r" );

	if ( fp ) {
		pmx_reader * const reader = pmx_reader_create_file( fp );
		pmx_header header;
		pmx_int err;
		
		err = pmx_read_header( reader, &header );
		if ( pmx_header_check( &header ) == 0 ) {
			pmx_isize sizes;
			pmx_minfo * const minfo = pmx_minfo_create();
			
			err = pmx_read_model_info( reader, minfo );

			err = pmx_read_isize( reader, &sizes );
			
			pmx_text_print( &minfo->localCharacterName, sizes.textTypeEncoding );
			pmx_text_print( &minfo->globalCharacterName, sizes.textTypeEncoding );
			pmx_text_print( &minfo->localComment, sizes.textTypeEncoding );
			pmx_text_print( &minfo->globalComment, sizes.textTypeEncoding );

			pmx_minfo_destroy( minfo );
		}
		
		pmx_reader_destroy( reader );

		fclose( fp );
	}
	
	return 0;
}
