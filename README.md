# libpmx
Polygon Model eXtended library.

Implemented based on [PMX 2.1 description](https://gist.github.com/felixjones/f8a06bd48f9da9a4539f).

## Basic usage

	#include <pmx.h>
	#include <stdio.h>

	FILE * const pmx_file = fopen( "example_model.pmx", "rb" ); // Open example model in binary read mode

	if ( pmx_file ) {
		char header[PMX_HEAD_LEN];
		float version;

		fread( header, PMX_HEAD_LEN, 1, pmx_file ); // Read the header

		version = pmx_header_version( header, PMX_HEAD_LEN ); // Get the pmx version
		// An invalid header will report version 0.0f

		if ( version ) {
			pmx_struct pmx = pmx_create_read_struct( version, NULL, NULL, NULL );

			if ( pmx ) {
				pmx_info info = pmx_create_info_struct( pmx );

				if ( info ) {
					pmx_init_io( pmx, pmx_file ); // Set file handle
					pmx_set_head_bytes( pmx, PMX_HEAD_LEN ); // We already read the header

					pmx_read_info( pmx, info ); // Read everything up to model data

					pmx_destroy_info_struct( pmx, info ); // Dealloc info
				}
				pmx_destroy_read_struct( pmx ); // Dealloc reader
			}
		}

		fclose( pmx_file ); // Close file
	}