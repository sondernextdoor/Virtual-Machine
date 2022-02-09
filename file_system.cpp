#include "file_system.h"


using vm_filesystem::file;

static std::vector<file> file_table{};
virtual_machine::storage* storage{};

file& get_file_table_entry( const char* name )
{
	static file empty{};

	for ( uint64_t i = 0; i < file_table.size(); i++ )
	{
		if ( std::string( file_table.at( i ).name )._Equal( name ) == true )
		{
			return file_table.at( i );
		}
	}

	return empty;
}

uint64_t get_file_table_index( const char* name )
{
	for ( uint64_t i = 0; i < file_table.size(); i++ )
	{
		if ( std::string( file_table.at( i ).name )._Equal( name ) == true )
		{
			return i;
		}
	}

	return -1;
}

bool extend( uint64_t size )
{
	if ( size + storage->in_use > storage->max_size ) 
	{ 
		return false; 
	}

	std::filesystem::resize_file( storage->path, storage->in_use + size );
	storage->in_use += size;

	return true;
}

void shrink( uint64_t size )
{
	std::filesystem::resize_file( storage->path, storage->in_use - size );
	storage->in_use -= size;
}

bool shift( uint64_t start_offset, uint64_t places_to_shift )
{
	if ( storage->in_use - start_offset == 0 )
	{
		storage->in_use += places_to_shift;
		return true;
	}

	if ( storage->open_stream() == false )
	{
		return false;
	}

	std::string file_data{};

	file_data.resize( storage->in_use - start_offset );

	storage->storage_stream.seekg( start_offset, std::ios::beg );
	storage->storage_stream.read( file_data.data(), storage->in_use - start_offset );

	if ( file_data.empty() == true )
	{
		storage->close_stream();
		return false;
	}

	storage->storage_stream.seekg( start_offset + places_to_shift, std::ios::beg );
	storage->storage_stream.write( file_data.c_str(), file_data.size() );

	storage->in_use += places_to_shift;
	storage->storage_stream.clear();
	storage->storage_stream.seekg( 0, std::ios::beg );

	return true;
}

bool shift_left( uint64_t start_offset, uint64_t places_to_shift )
{
	if ( storage->in_use - start_offset == 0 )
	{
		return true;
	}

	if ( storage->open_stream() == false )
	{
		return false;
	}

	std::string file_data{};

	file_data.resize( storage->in_use - start_offset );

	storage->storage_stream.seekg( start_offset, std::ios::beg );
	storage->storage_stream.read( file_data.data(), storage->in_use - start_offset );

	if ( file_data.empty() == true )
	{
		storage->close_stream();
		return false;
	}

	storage->storage_stream.clear();
	storage->storage_stream.seekg( 0, std::ios::beg );

	storage->storage_stream.seekg( start_offset - places_to_shift, std::ios::beg );
	storage->storage_stream.write( file_data.data(), file_data.size() );

	storage->storage_stream.clear();
	storage->storage_stream.seekg( 0, std::ios::beg );

	shrink( places_to_shift );
	return true;
}

bool read_metadata()
{
	if ( storage->open_stream() == false )
	{
		return false;
	}

	std::string file_data{};
	file_data.resize( storage->in_use );

	storage->storage_stream.read( file_data.data(), storage->in_use );

	if ( file_data.empty() == true )
	{
		return false;
	}

	for ( uint64_t i = 0, j = 0; i < storage->in_use; i++ )
	{
		if ( file_data.at( i ) == '|' )
		{
			file _file{};
				
			for ( uint64_t k = ( j == 0 ? 0 : file_table.at( j - 1 ).start + file_table.at( j - 1 ).size ), x = 0; k < i; k++, x++ )
			{
				_file.name[x] = file_data.at( k );
				++_file.metadata_size;
			}

			++_file.metadata_size;
			++i;

			for ( uint64_t k = i; k < storage->in_use; k++ )
			{
				if ( file_data.at( k ) == '|' )
				{
					_file.metadata_size += k - i + 1;
					_file.size = std::stoul( std::string( &file_data[i], k - i ) );
					_file.start = k + 1;

					i = ( _file.start + _file.size ) - 1;
					break;
				}
			}

			file_table.push_back( _file );
			++j;
		}
	}

	storage->close_stream();
	return true;
}

bool write_metadata( file& _file, bool write_op = false )
{
	if ( storage->open_stream() == false )
	{
		return false;
	}

	std::string metadata{ _file.name };

	metadata.push_back('|');
	metadata.append( std::to_string( _file.size ) );
	metadata.push_back( '|' );

	if ( write_op == true )
	{
		uint64_t file_index{ get_file_table_index( _file.name ) };

		if ( file_index == -1 )
		{
			return false;
		}

		uint64_t digit_increase{ 0 };

		if ( metadata.size() > _file.metadata_size )
		{
			digit_increase = metadata.size() - _file.metadata_size;

			shift( _file.start, digit_increase );

			for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
			{
				file_table.at( i ).start += digit_increase;
			}

			storage->storage_stream.seekg( _file.start - _file.metadata_size );
			storage->storage_stream.write( metadata.c_str(), metadata.size() );
		
			_file.start += digit_increase;
			_file.metadata_size += digit_increase;
		}
		else if ( metadata.size() < _file.metadata_size )
		{
			uint64_t digit_decrease = _file.metadata_size - metadata.size();

			shift_left( _file.start, digit_decrease );

			for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
			{
				file_table.at( i ).start -= digit_decrease;
			}

			storage->storage_stream.seekg( _file.start - _file.metadata_size );
			storage->storage_stream.write( metadata.c_str(), metadata.size() );
		
			_file.start -= digit_decrease;
			_file.metadata_size -= digit_decrease;
		}
		else
		{
			storage->storage_stream.seekg( _file.start - _file.metadata_size );
			storage->storage_stream.write( metadata.c_str(), metadata.size() );
		}

		storage->storage_stream.clear();
		storage->storage_stream.seekg( 0, std::ios::beg );

		return true;
	}

	storage->storage_stream.seekg( _file.start, std::ios::beg );

	_file.start += metadata.size();
	_file.metadata_size = metadata.size();

	storage->storage_stream.write( metadata.c_str(), metadata.size() );
	storage->in_use += metadata.size();

	storage->storage_stream.clear();
	storage->storage_stream.seekg( 0, std::ios::beg );

	return true;
}

bool vm_filesystem::initialize_file_system( virtual_machine::storage* _storage )
{
	storage = _storage;
	return read_metadata();
}

bool vm_filesystem::create_file( const char* name )
{
	if ( name == nullptr )
	{
		return false;
	}

	if ( storage->open_stream() == false )
	{
		return false;
	}

	for ( uint64_t i = 0; i < file_table.size(); i++ )
	{
		if ( std::string( file_table.at( i ).name )._Equal( name ) == true ) 
		{
			storage->close_stream();
			return false;
		}
	}

	file _file{};

	_file.start = storage->in_use;
	_file.size = 0;

	for ( uint64_t i = 0; *( name + i ) != '\0'; i++ )
	{
		_file.name[i] = name[i];
	}

	if ( write_metadata( _file ) == false )
	{
		storage->close_stream();
		return false;
	}

	file_table.push_back( _file );
	storage->close_stream();

	return true;
}

bool vm_filesystem::write_file( const char* name, const char* buffer, uint64_t size, uint64_t offset )
{
	if ( name == nullptr || buffer == nullptr || size + offset + storage->in_use > storage->max_size )
	{ 
		return false; 
	}
	
	if ( storage->open_stream() == false )
	{
		return false;
	}

	uint64_t file_index{ get_file_table_index( name ) };

	if ( file_index == -1 )
	{
		storage->close_stream();
		return false;
	}

	file& _file{ get_file_table_entry( name ) };

	if ( size == 0 )
	{
		return clear_file( name );
	}

	if ( offset == 0 )
	{
		if ( size > _file.size )
		{
			if ( shift( _file.start + _file.size, size - _file.size ) == false )
			{
				storage->close_stream();
				return false;
			}

			for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
			{
				file_table.at( i ).start += size - _file.size;
			}

			storage->storage_stream.seekg( _file.start, std::ios::beg );
			storage->storage_stream.write( buffer, size );

			_file.size = size;
			return write_metadata( _file, true );
		}
		else
		{
			storage->storage_stream.seekg( _file.start, std::ios::beg );
			storage->storage_stream.write( buffer, size );

			if ( shift_left( _file.start + _file.size, _file.size - size ) == false )
			{
				storage->close_stream();
				return false;
			}

			for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
			{
				file_table.at( i ).start -= _file.size - size;
			}

			_file.size = size;
			return write_metadata( _file, true );
		}
	}

	if ( offset > _file.size )
	{
		storage->close_stream();
		return false;
	}

	if ( size > _file.size - offset )
	{
		if ( file_index != file_table.size() - 1 )
		{	
			if ( shift( _file.start + _file.size, size - ( _file.size - offset ) ) == false )
			{
				storage->close_stream();
				return false;
			}

			for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
			{
				file_table.at( i ).start += size - ( _file.size - offset );
			}
		}
		else
		{
			storage->in_use += size - ( _file.size - offset );
		}

		_file.size += size - ( _file.size - offset );
		write_metadata( _file, true );
	}
	
	storage->storage_stream.seekg( _file.start + offset, std::ios::beg );
	storage->storage_stream.write( buffer, size );
	storage->close_stream();
	return true;
}

bool vm_filesystem::read_file( const char* name, char* buffer, uint64_t size, uint64_t offset )
{
	if ( name == nullptr || buffer == nullptr || size == 0 ) 
	{ 
		return false; 
	}

	if ( storage->open_stream() == false )
	{
		return false;
	}

	file _file{ get_file_table_entry( name ) };

	if ( size + offset > _file.size )
	{
		if ( offset >= _file.size ) 
		{ 
			return false; 
		}

		size = _file.size - offset;
	}

	storage->storage_stream.seekg( _file.start + offset, std::ios::beg );
	storage->storage_stream.read( buffer, size );
	storage->close_stream();

	return true;
}

bool vm_filesystem::delete_file( const char* name )
{
	if ( name == nullptr ) 
	{ 
		return false; 
	}

	uint64_t file_index{ get_file_table_index( name ) };

	if ( file_index == -1 )
	{
		return false;
	}

	file _file{ get_file_table_entry( name ) };

	if ( file_index == file_table.size() - 1 && file_index != 0 )
	{
		shrink( _file.size + _file.metadata_size );
	}
	else if ( file_index == 0 && file_table.size() == 1 )
	{
		std::filesystem::resize_file( storage->path, 0 );
		storage->in_use = 0;
	}
	else
	{
		shift_left( _file.start + _file.size, _file.size + _file.metadata_size );

		for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
		{
			file_table.at( i ).start -= _file.size + _file.metadata_size;
		}
	}

	file_table.erase( file_table.begin() + file_index );
	return true;
}

bool vm_filesystem::clear_file( const char* name )
{
	if ( name == nullptr ) 
	{ 
		return false; 
	}

	uint64_t file_index{ get_file_table_index( name ) };

	if ( file_index == -1 )
	{
		storage->close_stream();
		return false;
	}

	file& _file{ get_file_table_entry( name ) };

	if ( file_index == file_table.size() - 1 )
	{
		uint64_t size{ _file.size };
		_file.size = 0;

		if ( write_metadata( _file, true ) == false )
		{
			_file.size = size;
			return false;
		}

		shrink( size );
		return true;
	}

	if ( shift_left( _file.start + _file.size, _file.size ) == false )
	{
		return false;
	}

	for ( uint64_t i = file_index + 1; i < file_table.size(); i++ )
	{
		file_table.at( i ).start -= _file.size;
	}

	_file.size = 0;

	return write_metadata( _file, true );
}

uint64_t vm_filesystem::get_file_size( const char* name )
{
	if ( name == nullptr ) 
	{ 
		return false; 
	}

	return get_file_table_entry( name ).size;
}

std::vector<file> vm_filesystem::get_file_table()
{
	return file_table;
}