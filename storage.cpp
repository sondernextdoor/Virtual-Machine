#include "virtual_machine.h"

virtual_machine::storage::storage()
{
	std::filesystem::path current_path{ std::filesystem::current_path() };
	std::filesystem::space_info space{ std::filesystem::space( current_path ) };

    if ( open_stream( std::ios::in | std::ios::out | std::ios::app ) == false ) 
	{
        return;
    } 
	
	max_size = space.available / 100;

	storage_stream.ignore( max_size );
	in_use = storage_stream.gcount();

	close_stream();
 	path = ( current_path /= "storage.vm" ).generic_string();
}

bool virtual_machine::storage::open_stream( std::ios::openmode open_mode )
{
	storage_stream.open( "storage.vm", open_mode );
	storage_stream.clear();

	if ( storage_stream.is_open() == false )
	{
		return false;	
	}

	storage_stream.seekg( 0, std::ios::beg );
	return true;
}

void virtual_machine::storage::close_stream()
{
	if ( storage_stream.is_open() )
	{
		storage_stream.close();
	}
}