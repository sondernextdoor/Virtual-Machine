#pragma once
#include "registers.h"

class virtual_machine
{
public:

	class central_processing_unit
	{
	public:

		vm_registers registers{};
		std::vector<int64_t> stack{};

		vm_registers execute( int64_t* instructions, bool b_call = false, int64_t instruction_count = 0 );

	private:

		void push( int64_t* instructions );
		void pop( int64_t* instructions );
		template <typename T = int64_t> 
		void mov( int64_t* instructions );
		void movf( int64_t* instructions );
		void cmp( int64_t* instructions );
		void add( int64_t* instructions );
		void sub( int64_t* instructions );
		void mul( int64_t* instructions );
		void div( int64_t* instructions );
		void mod( int64_t* instructions );
		void inc( int64_t* instructions );
		void dec( int64_t* instructions );
		void greater( int64_t* instructions );
		void less( int64_t* instructions );
		void bitwise_and( int64_t* instructions );
		void bitwise_or( int64_t* instructions );
		void bitwise_xor( int64_t* instructions );
		void bitwise_not( int64_t* instructions );
		void left_shift( int64_t* instructions );
		void right_shift( int64_t* instructions );
		void jmp( int64_t* instructions );
		void tjmp( int64_t* instructions );
		void fjmp( int64_t* instructions );
		void stall( int64_t* instructions );
		void get_char( int64_t* instructions );
		void str_out( int64_t* instructions );
		void num_out( int64_t* instructions, bool hex = false );
	};

	class storage
	{
	public:

		std::string path{};
		std::fstream storage_stream{};
		uint64_t max_size{};
		uint64_t in_use{};

		storage();
		bool open_stream( std::ios::openmode open_mode = ( std::ios::binary | std::ios::in | std::ios::out ) );
		void close_stream();
	};

	central_processing_unit _cpu;
	storage _storage;
	virtual_machine();
};