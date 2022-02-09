#pragma once
#include "instruction_set.h"

static constexpr const uint32_t register_count{ 11 };

struct vm_registers
{
	int64_t GRA{};
	int64_t GRB{};
	int64_t GRC{};
	int64_t GRD{};
	int64_t GRE{};
	int64_t GRF{};
	int64_t accumulator{};
	int64_t RR{};
	int64_t SP{}; 
	int64_t BP{};
	int64_t IC{};
	bool boolean_flag{};


	int32_t& lower_32( int64_t& _register );
	int16_t& lower_16( int64_t& _register );
	int8_t& lower_8( int64_t& _register );
	int64_t& get( uint64_t index );
	vm_registers& get_reference() &;
	void clear();
};