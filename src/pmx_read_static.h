#ifndef __PMX_READ_STATIC_H__
#define __PMX_READ_STATIC_H__

static pmx_byte pmx_read_text( pmx_read * const _read, pmx_text * const _text ) {
	_read->fread_f( _read, &_text->size, sizeof( _text->size ) );
	if ( _text->size ) {
		_text->string.utf8 = ( pmx_byte * )pmx_alloc( PMX_NULL, _text->size );
		_read->fread_f( _read, _text->string.utf8, _text->size );
	}
	
	return 0;
}

static pmx_byte * pmx_read_scratch_alloc( pmx_read * const _read, const pmx_int _size ) {
	pmx_scratch * scratch;
	pmx_int count;
	pmx_byte ** pointers;
	pmx_read_private * const read = ( pmx_read_private * )_read;
	
	if ( !read->scratch ) {
		read->scratch = ( pmx_byte * )pmx_alloc( PMX_NULL, sizeof( pmx_scratch ) );
		memset( read->scratch, 0, sizeof( pmx_scratch ) );
	}

	scratch = ( pmx_scratch * )read->scratch;
	count = scratch->count + 1;
	pointers = ( pmx_byte ** )pmx_alloc( scratch->pointers, count * sizeof( pmx_byte * ) );

	if ( pointers ) {
		pmx_byte * const page = ( pmx_byte * )pmx_alloc( PMX_NULL, _size );

		pointers[scratch->count] = page;

		scratch->pointers = pointers;
		scratch->count = count;

		return page;
	}

	return PMX_NULL;
}

static pmx_int pmx_read_scratch_dealloc( pmx_read * const _read ) {
	pmx_scratch * scratch;
	pmx_read_private * const read = ( pmx_read_private * )_read;

	if ( !read->scratch ) {
		return 0;
	}

	scratch = ( pmx_scratch * )read->scratch;

	while ( scratch->count-- ) {
		pmx_alloc( scratch->pointers[scratch->count], 0 );
	}

	pmx_alloc( scratch->pointers, 0 );
	pmx_alloc( read->scratch, 0 );
	read->scratch = PMX_NULL;

	return 0;
}

static pmx_int pmx_read_file_fread( pmx_read_file * const _read, void * const _dest, const pmx_int _length ) {
	return ( pmx_int )fread( _dest, 1, ( size_t )_length, _read->file );
}

static pmx_int pmx_read_file_fseek( pmx_read_file * const _read, const pmx_int _offset, const pmx_int _origin ) {
	switch ( _origin ) {
	case PMX_READ_SEEK_SET:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_SET );
	case PMX_READ_SEEK_CUR:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_CUR );
	case PMX_READ_SEEK_END:
		return ( pmx_int )fseek( _read->file, ( size_t )_offset, SEEK_END );
	}
	return -1;
}

static pmx_int pmx_read_file_ftell( pmx_read_file * const _read ) {
	return ( pmx_int )ftell( _read->file );
}

static pmx_int pmx_read_buffer_fread( pmx_read_buffer * const _read, void * const _dest, const pmx_int _length ) {
	const pmx_int length = ( pmx_int )_read->end - ( pmx_int )_read->cur;
	return ( pmx_int )memcpy( _dest, _read->cur, ( size_t )( _length < length ? _length : length ) );
}

static pmx_int pmx_read_buffer_fseek( pmx_read_buffer * const _read, const pmx_int _offset, const pmx_int _origin ) {
	void * newPos;

	switch ( _origin ) {
	case PMX_READ_SEEK_SET:
		newPos = pmx_ptr_add( _read->start, _offset );
        break;
	case PMX_READ_SEEK_CUR:
        newPos = pmx_ptr_add( _read->cur, _offset );
        break;
	case PMX_READ_SEEK_END:
        newPos = pmx_ptr_add( _read->end, _offset );
        break;
    default:
        newPos = PMX_NULL;
        break;
	}
	
	if ( newPos <= _read->end && newPos >= _read->start ) {
		_read->cur = newPos;
		return 0;
	}
	return -1;
}

static pmx_int pmx_read_buffer_ftell( pmx_read_buffer * const _read ) {
	return ( pmx_int )pmx_ptr_add( _read->cur, -( pmx_int )_read->start );
}

static pmx_int pmx_read_sizeof_text( pmx_read * const _read ) {
	pmx_text text;
	pmx_int size;

	text.size = 0;
	text.string.utf8 = PMX_NULL;
	pmx_read_text( _read, &text );

	size = text.size + 4;

	pmx_alloc( text.string.utf8, 0 );

	return size;
}

static pmx_int pmx_read_vertex( pmx_read * const _read, pmx_read_data_struct * const _struct ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_int ii = 0;
	const pmx_int startByte = pmx_read_sizeof_vertex( _read, _struct->start );
	pmx_int span = 0;
	pmx_byte order = 0;

	memset( _struct->maskOrder, 0, sizeof( _struct->maskOrder ) );

	if ( ( _struct->mask & PMX_VERTEX_POSITION ) == PMX_VERTEX_POSITION ) {
		_struct->maskOrder[order++] = PMX_VERTEX_POSITION;
		span += 12;
	}
	if ( ( _struct->mask & PMX_VERTEX_NORMAL ) == PMX_VERTEX_NORMAL ) {
		_struct->maskOrder[order++] = PMX_VERTEX_NORMAL;
		span += 12;
	}
	if ( ( _struct->mask & PMX_VERTEX_UV ) == PMX_VERTEX_UV ) {
		_struct->maskOrder[order++] = PMX_VERTEX_UV;
		span += 8;
	}
	if ( ( _struct->mask & PMX_VERTEX_ADDITIONAL_UV ) == PMX_VERTEX_ADDITIONAL_UV ) {
		_struct->maskOrder[order++] = PMX_VERTEX_ADDITIONAL_UV;
		span += 16 * read->info.additionalUVCount;
	}
	if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
		_struct->maskOrder[order++] = PMX_VERTEX_WEIGHT_TYPE;
		span += 1;
	}
	if ( ( _struct->mask & PMX_VERTEX_WEIGHT_DEF ) == PMX_VERTEX_WEIGHT_DEF ) {
		_struct->maskOrder[order++] = PMX_VERTEX_WEIGHT_DEF;
		span += 42;
	}
	if ( ( _struct->mask & PMX_VERTEX_EDGE_SCALE ) == PMX_VERTEX_EDGE_SCALE ) {
		_struct->maskOrder[order++] = PMX_VERTEX_EDGE_SCALE;
		span += 4;
	}

	_read->fseek_f( _read, read->offsetVertex + startByte, PMX_READ_SEEK_SET );	

	while ( ii < _struct->count ) {
		pmx_int index = ii * span;
		pmx_byte wType;

		if ( ( _struct->mask & PMX_VERTEX_POSITION ) == PMX_VERTEX_POSITION ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 12 );
		} else {
			_read->fseek_f( _read, 12, PMX_READ_SEEK_CUR );
		}

		if ( ( _struct->mask & PMX_VERTEX_NORMAL ) == PMX_VERTEX_NORMAL ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 12 );
		} else {
			_read->fseek_f( _read, 12, PMX_READ_SEEK_CUR );
		}
		
		if ( ( _struct->mask & PMX_VERTEX_UV ) == PMX_VERTEX_UV ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 8 );
		} else {
			_read->fseek_f( _read, 8, PMX_READ_SEEK_CUR );
		}

		if ( ( _struct->mask & PMX_VERTEX_ADDITIONAL_UV ) == PMX_VERTEX_ADDITIONAL_UV ) {
			pmx_byte uvCount = read->info.additionalUVCount;
			while ( uvCount-- ) {
				index += _read->fread_f( _read, &_struct->buffer[index], 16 );
			}
		} else {
			_read->fseek_f( _read, 16 * read->info.additionalUVCount, PMX_READ_SEEK_CUR );
		}
		
		_read->fread_f( _read, &wType, 1 );
		
		if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
			_struct->buffer[index] = wType;
			index++;
		}
		
		if ( ( _struct->mask & PMX_VERTEX_WEIGHT_TYPE ) == PMX_VERTEX_WEIGHT_TYPE ) {
			switch ( wType ) {
			case 0:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone );
				break;
			case 1:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 2 + 4 );
				break;
			case 2:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 4 + 16 );
				break;
			case 3:
				index += _read->fread_f( _read, &_struct->buffer[index], read->isize.bone * 2 + 40 );
				break;
			default:
				pmx_print_error( "read", "Unknown BDEF\n" );
				break;
			}
		} else {
			switch ( wType ) {
			case 0:
				_read->fseek_f( _read, read->isize.bone, PMX_READ_SEEK_CUR );
				break;
			case 1:
				_read->fseek_f( _read, read->isize.bone * 2 + 4, PMX_READ_SEEK_CUR );
				break;
			case 2:
				_read->fseek_f( _read, read->isize.bone * 4 + 16, PMX_READ_SEEK_CUR );
				break;
			case 3:
				_read->fseek_f( _read, read->isize.bone * 2 + 40, PMX_READ_SEEK_CUR );
				break;
			default:
				pmx_print_error( "read", "Unknown BDEF\n" );
				break;
			}
		}

		if ( ( _struct->mask & PMX_VERTEX_EDGE_SCALE ) == PMX_VERTEX_EDGE_SCALE ) {
			index += _read->fread_f( _read, &_struct->buffer[index], 4 );
		} else {
			_read->fseek_f( _read, 4, PMX_READ_SEEK_CUR );
		}

		ii++;
	}
	
	return _struct->count * span;
}

static const char * pmx_read_alloc_str( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_text text;
	char * outChars = PMX_NULL;

	pmx_read_text( _read, &text );

	if ( read->info.textTypeEncoding ) {
		outChars = ( char * )pmx_read_scratch_alloc( _read, text.size + 1 );

		memcpy( outChars, text.string.utf8, text.size );
		outChars[text.size] = 0;

		pmx_alloc( text.string.utf8, 0 );
	} else {
		pmx_int ii;
		const pmx_int hSize = text.size / 2;

		outChars = ( char * )pmx_read_scratch_alloc( _read, hSize + 1 );

		for ( ii = 0; ii < hSize; ii++ ) {
			outChars[ii] = text.string.utf8[ii * 2];
		}

		outChars[hSize] = 0;
	}

	return outChars;
}

static const wchar_t * pmx_read_alloc_wstr( pmx_read * const _read ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	pmx_text text;
	wchar_t * outChars = PMX_NULL;

	pmx_read_text( _read, &text );

	if ( read->info.textTypeEncoding ) {
		pmx_int ii;

		outChars = ( wchar_t * )pmx_read_scratch_alloc( _read, ( text.size * 2 ) + 2 );

		for ( ii = 0; ii < text.size; ii++ ) {
			outChars[ii] = ( wchar_t )text.string.utf8[ii];
		}
		outChars[text.size * 2] = 0;

		pmx_alloc( text.string.utf8, 0 );
	} else {
		outChars = ( wchar_t * )pmx_read_scratch_alloc( _read, text.size + 2 );

		memcpy( outChars, text.string.utf16, text.size );
		outChars[text.size / 2] = 0;
	}

	return outChars;
}

static const char * pmx_read_gets_texture( pmx_read * const _read, const pmx_int _index ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( _index < read->countTexture ) {
		const pmx_int offset = pmx_read_sizeof_texture( _read, _index );
		_read->fseek_f( _read, read->offsetTexture + offset, PMX_READ_SEEK_SET );

		return pmx_read_alloc_str( _read );
	}

	return PMX_NULL;
}

static const wchar_t * pmx_read_getls_texture( pmx_read * const _read, const pmx_int _index ) {
	pmx_read_private * const read = ( pmx_read_private * )_read;
	if ( _index < read->countTexture ) {
		const pmx_int offset = pmx_read_sizeof_texture( _read, _index );
		_read->fseek_f( _read, read->offsetTexture + offset, PMX_READ_SEEK_SET );

		return pmx_read_alloc_wstr( _read );
	}

	return PMX_NULL;
}

static const char * pmx_read_gets_minfo( pmx_read * const _read, const pmx_int _var ) {
	pmx_int nameLocal, nameGlobal, commentLocal;
	
	_read->fseek_f( _read, PMX_OFFSET_MINFO, PMX_READ_SEEK_SET );
	if ( _var == PMX_MINFO_NAME_LOCAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &nameLocal, 4 );
	_read->fseek_f( _read, nameLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_NAME_GLOBAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &nameGlobal, 4 );
	_read->fseek_f( _read, nameGlobal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_LOCAL ) {
		return pmx_read_alloc_str( _read );
	}

	_read->fread_f( _read, &commentLocal, 4 );
	_read->fseek_f( _read, commentLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_GLOBAL ) {
		return pmx_read_alloc_str( _read );
	}
	
	return PMX_NULL;
}

static const wchar_t * pmx_read_getls_minfo( pmx_read * const _read, const pmx_int _var ) {
	pmx_int nameLocal, nameGlobal, commentLocal;
	
	_read->fseek_f( _read, PMX_OFFSET_MINFO, PMX_READ_SEEK_SET );
	if ( _var == PMX_MINFO_NAME_LOCAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &nameLocal, 4 );
	_read->fseek_f( _read, nameLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_NAME_GLOBAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &nameGlobal, 4 );
	_read->fseek_f( _read, nameGlobal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_LOCAL ) {
		return pmx_read_alloc_wstr( _read );
	}

	_read->fread_f( _read, &commentLocal, 4 );
	_read->fseek_f( _read, commentLocal, PMX_READ_SEEK_CUR );
	if ( _var == PMX_MINFO_COMMENT_GLOBAL ) {
		return pmx_read_alloc_wstr( _read );
	}
	
	return PMX_NULL;
}

#endif