#include "file_system.h"

bool __stdcall DllMain( void* module_handle, uint32_t reason, void* reserved )
{
    return true;
}

extern "C"
{
    static virtual_machine vm{ virtual_machine() };
    static std::vector<vm_filesystem::file*> file_array{};

    __declspec( dllexport ) void __stdcall initialize_vm()
    {
        freopen( "conin$","r", stdin );
        freopen( "conout$","w", stdout );
        freopen( "conout$","w", stderr );
    }

    __declspec( dllexport ) vm_registers __stdcall execute( int64_t* instructions )
    {
        return vm._cpu.execute( instructions );
    }

    __declspec( dllexport ) int64_t* __stdcall get_stack( uint64_t* out_size )
    {
        if ( out_size != nullptr )
        {
            *out_size = vm._cpu.stack.size();
        }

        return vm._cpu.stack.data();
    }

    __declspec( dllexport ) bool __stdcall create_file( const char* name )
    {
        return vm_filesystem::create_file( name );
    }
    
    __declspec( dllexport ) bool __stdcall write_file( const char* name, const char* buffer, uint64_t size, uint64_t offset )
    {
        if ( buffer == nullptr )
        {
            return false;
        }

        std::string string_buffer{};
        string_buffer.resize( size );

        for ( uint64_t i = 0; i < size; i++ )
        {
            string_buffer[i] = buffer[i];
        }

        return vm_filesystem::write_file( name, string_buffer.c_str(), size, offset );
    }

    __declspec( dllexport ) bool __stdcall read_file( const char* name, char* buffer, uint64_t size, uint64_t offset )
    {
        std::string string_buffer{};
        string_buffer.resize( size );
        
        vm_filesystem::read_file( name, string_buffer.data(), size, offset );

        for ( uint64_t i = 0; i < size; i++ )
        {
            buffer[i] = string_buffer[i];
        }

        return true;
    }

    __declspec( dllexport ) bool __stdcall delete_file( const char* name )
    {
        return vm_filesystem::delete_file( name );
    }

    __declspec( dllexport ) bool __stdcall clear_file( const char* name )
    {
        return vm_filesystem::clear_file( name );
    }

    __declspec( dllexport ) uint64_t __stdcall get_file_size( const char* name )
    {
        return vm_filesystem::get_file_size( name );
    }

    __declspec( dllexport ) void __stdcall free_file_array()
    {
        if ( file_array.empty() == false )
        {
            for ( vm_filesystem::file* _file : file_array )
            {
                free( _file );
            }

            file_array.clear();
        }
    }

    __declspec( dllexport ) void* __stdcall get_file_array( uint64_t* out_table_size )
    {
        free_file_array();

        std::vector<vm_filesystem::file> file_table = vm_filesystem::get_file_table();

        if ( out_table_size != nullptr )
        {
            *out_table_size = file_table.size();
        }

        for ( int i = 0; i < file_table.size(); i++ )
        {
            vm_filesystem::file* _file{ static_cast<vm_filesystem::file*>( malloc( sizeof( vm_filesystem::file ) ) ) };
            *_file = file_table.at( i );
            file_array.push_back( _file );
        }

        return file_array.data();
    }
}