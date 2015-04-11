#include <pmx.h>
#include <stdio.h>
#include <stdlib.h>

void log_error( pmx_struct _pmx, const char * const _message, void * const _user_ptr ) {
	printf( "[E] %s\n", _message );
}

void log_warning( pmx_struct _pmx, const char * const _message, void * const _user_ptr ) {
	printf( "[W] %s\n", _message );
}

int main( int argc, char ** argv ) {
	
	FILE * const iaxFile = fopen( "IAx/IAx.pmx", "rb" );
	if ( iaxFile ) {
		char header[PMX_HEAD_LEN];
		float version;

		fread( header, PMX_HEAD_LEN, 1, iaxFile );
		version = pmx_header_version( header, PMX_HEAD_LEN );

		if ( version ) {
			// PMX FILE IS VALID			
			pmx_struct pmx = pmx_create_read_struct( version, log_error, log_warning, NULL );
			pmx_info info = pmx_create_info_struct( pmx );
			wchar_t * buffer = NULL;
			int buffer_length;

			pmx_init_io( pmx, iaxFile );
			pmx_set_head_bytes( pmx, PMX_HEAD_LEN );
			pmx_read_info( pmx, info );

			pmx_destroy_info_struct( pmx, info );
			pmx_destroy_read_struct( pmx );
		}

		fclose( iaxFile );
	}

	return 0;
}
