#pragma once
#include "virtual_machine.h"

namespace vm_filesystem
{
	static constexpr const uint32_t max_path{ 260 };

	struct file
	{
		char name[max_path]{};
		uint64_t start{};
		uint64_t size{};
		uint64_t metadata_size{};
	};

	bool initialize_file_system( virtual_machine::storage* storage );
	bool create_file( const char* name );
	bool write_file( const char* name, const char* buffer, uint64_t size, uint64_t offset = 0 );
	bool read_file( const char* name, char* buffer, uint64_t size, uint64_t offset = 0 );
	bool delete_file( const char* name );
	bool clear_file( const char* name );
	uint64_t get_file_size( const char* name );
	std::vector<file> get_file_table();
}