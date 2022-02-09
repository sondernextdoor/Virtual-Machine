#include "registers.h"

int32_t& vm_registers::lower_32( int64_t& _register )
{
	return *reinterpret_cast<int32_t*>( &_register );
}

int16_t& vm_registers::lower_16( int64_t& _register )
{
	return *reinterpret_cast<int16_t*>( &_register );
}

int8_t& vm_registers::lower_8( int64_t& _register )
{
	return *reinterpret_cast<int8_t*>( &_register );
}

int64_t& vm_registers::get( uint64_t index )
{
	switch ( index )
	{
		case 0:
		{
			return GRA;
		} break;

		case 1:
		{
			return GRB;
		} break;

		case 2:
		{
			return GRC;
		} break;

		case 3:
		{
			return GRD;
		} break;

		case 4:
		{
			return GRE;
		} break;

		case 5:
		{
			return GRF;
		} break;

		case 6:
		{
			return accumulator;
		} break;

		case 7:
		{
			return RR;
		} break;

		case 8:
		{
			return SP;
		} break;

		case 9:
		{
			return BP;
		} break;

		case 10:
		{
			return IC;
		} break;
	}

	return GRA;
}

vm_registers& vm_registers::get_reference() &
{
	return *this;
}

void vm_registers::clear()
{
	for ( uint64_t i = 0; i < register_count; i++ )
	{
		get( i ) = 0;
	}

	boolean_flag = 0;
}