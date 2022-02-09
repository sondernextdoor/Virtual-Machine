#pragma once
#include "file_system.h"

template <typename T>
void test_expected_value( T value, T expected_value, const char* caller = __builtin_FUNCTION(), int32_t line = __builtin_LINE() )
{
	if ( value != expected_value )
	{
		std::cerr << caller << " failed\nexpected value: " << expected_value 
			<< "\nactual value: " << value << "\nline: " << line << std::endl; 
	}
}

class vm_tests
{
	virtual_machine vm = virtual_machine();
	vm_registers& registers{ vm._cpu.registers.get_reference() };

public:

	static void run_tests()
	{
		vm_tests tests{ vm_tests() };

		tests.push_pop();
		tests.mov();
		tests.cmp();
		tests.add();	
		tests.sub();	
		tests.mul();	
		tests.div();	
		tests.mod();
		tests.inc_dec();
		tests.greater_less();
		tests.and_or_xor_not_lshift_rshift();
		tests.no_reg();
		tests.jmp();
	}

private:

	void push_pop()
	{
		int64_t stub[]
		{
			ENTRY,
			PUSH, 15,
			POP, GRA,
			RET
		};

		vm._cpu.execute( stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 15 ) );
		registers.clear();
	}

	void mov()
	{
		int val{ 15 };
		std::vector<int8_t> bytes{ 15, 30, 45, 60, 75, 90, 105, 120 };
		std::vector<int16_t> words{ 15, 30, 45, 60, 75, 90, 105, 120 };
		std::vector<int32_t> double_words{ 15, 30, 45, 60, 75, 90, 105, 120 };

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			RET
		};

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, GRA,
			RET
		};

		int64_t address_literal_stub[]
		{
			ENTRY,
			MOV, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t address_reg_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, DEREF, GRA,
			RET
		};

		int64_t movb_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &bytes[2] ),
			MOVB, DEREF, GRA, 15,
			RET
		};

		int64_t movw_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &words[2] ),
			MOVW, DEREF, GRA, 15,
			RET
		};

		int64_t movd_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &double_words[2] ),
			MOVD, DEREF, GRA, 15,
			RET
		};

		int64_t movf_stub[]
		{
			ENTRY,
			MOVF, GRA,
			RET
		};

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 15 ) );
		registers.clear();

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 15 ) );
		registers.clear();

		vm._cpu.execute( address_literal_stub );
		test_expected_value( registers.lower_32( registers.GRA ), static_cast<int32_t>( 15 ) );
		registers.clear();

		vm._cpu.execute( address_reg_stub );

		test_expected_value( registers.GRA, reinterpret_cast<int64_t>( &val ) );
		test_expected_value( registers.lower_32( registers.GRB ), static_cast<int32_t>( 15 ) );
		registers.clear();

		vm._cpu.execute( movb_stub );

		for ( uint64_t i = 0, j = 15; i < bytes.size(); i++, j += 15 )
		{
			test_expected_value( static_cast<int32_t>( bytes[i] ),  static_cast<int32_t>( ( i == 2 ? 15 : j ) ) );
		}

		test_expected_value( registers.GRA, reinterpret_cast<int64_t>( &bytes[2] ) );
		registers.clear();

		vm._cpu.execute( movw_stub );

		for ( uint64_t i = 0, j = 15; i < words.size(); i++, j += 15 )
		{
			test_expected_value( words[i],  static_cast<int16_t>( ( i == 2 ? 15 : j ) ) );
		}

		test_expected_value( registers.GRA,  reinterpret_cast<int64_t>( &words[2] ) );
		registers.clear();

		vm._cpu.execute( movd_stub );

		for ( uint64_t i = 0, j = 15; i < bytes.size(); i++, j += 15 )
		{
			test_expected_value( double_words[i], static_cast<int32_t>( ( i == 2 ? 15 : j ) ) );
		}

		test_expected_value( registers.GRA,  reinterpret_cast<int64_t>( &double_words[2] ) );
		registers.clear();

		vm._cpu.execute( movf_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( registers.boolean_flag ) );
		registers.clear();
	}

	void cmp()
	{
		int64_t val{ 15 };

		int64_t reg_stub_true[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 15,
			CMP, GRA, GRB,
			RET
		};

		int64_t reg_literal_stub_true[]
		{
			ENTRY,
			MOV, GRA, 15,
			CMP, GRA, 15,
			RET
		};

		int64_t reg_address_stub_true[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			CMP, GRA, DEREF, GRB,
			RET
		};

		int64_t reg_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 20,
			CMP, GRA, GRB,
			RET
		};

		int64_t reg_literal_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			CMP, GRA, 20,
			RET
		};

		int64_t reg_address_stub_false[]
		{
			ENTRY,
			MOV, GRA, 20,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			CMP, GRA, DEREF, GRB,
			RET
		};

		vm._cpu.execute( reg_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( reg_literal_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( reg_address_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( reg_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( reg_literal_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( reg_address_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();
	}

	void add()
	{
		int64_t val{ 15 };

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 15,
			ADD, GRA, GRB,
			RET
		};

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			ADD, GRA, 15,
			RET
		};

		int64_t reg_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			ADD, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			ADD, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t reg_double_address_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			ADD, DEREF, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_double_address_stub[]
		{
			ENTRY,
			ADD, DEREF, reinterpret_cast<int64_t>( &val ), DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( reg_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( literal_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( reg_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );		
		registers.clear();

		vm._cpu.execute( literal_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();
	}

	void sub()
	{
		int64_t val{ 5 };

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 5,
			SUB, GRA, GRB,
			RET
		};

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			SUB, GRA, 5,
			RET
		};

		int64_t reg_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			SUB, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			SUB, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t reg_double_address_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			SUB, DEREF, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_double_address_stub[]
		{
			ENTRY,
			SUB, DEREF, reinterpret_cast<int64_t>( &val ), DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 10 ) );
		registers.clear();

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 10 ) );
		registers.clear();

		vm._cpu.execute( reg_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 10 ) );
		registers.clear();

		vm._cpu.execute( literal_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 10 ) );
		registers.clear();

		vm._cpu.execute( reg_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 0 ) );
		registers.clear();

		vm._cpu.execute( literal_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 0 ) );
		registers.clear();
	}

	void mul()
	{
		int64_t val{ 2 };

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 2,
			MUL, GRA, GRB,
			RET
		};

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MUL, GRA, 2,
			RET
		};

		int64_t reg_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			MUL, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MUL, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t reg_double_address_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			MUL, DEREF, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_double_address_stub[]
		{
			ENTRY,
			MUL, DEREF, reinterpret_cast<int64_t>( &val ), DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( reg_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( literal_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 30 ) );
		registers.clear();

		vm._cpu.execute( reg_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 4 ) );
		registers.clear();

		vm._cpu.execute( literal_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 4 ) );
		registers.clear();
	}

	void div()
	{
		int64_t val{ 2 };

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 2,
			DIV, GRA, GRB,
			RET
		};

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			DIV, GRA, 2,
			RET
		};

		int64_t reg_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			DIV, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			DIV, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t reg_double_address_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			DIV, DEREF, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_double_address_stub[]
		{
			ENTRY,
			DIV, DEREF, reinterpret_cast<int64_t>( &val ), DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 7 ) );
		registers.clear();

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 7 ) );
		registers.clear();

		vm._cpu.execute( reg_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 7 ) );
		registers.clear();

		vm._cpu.execute( literal_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 7 ) );
		registers.clear();

		vm._cpu.execute( reg_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( literal_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();
	}

	void mod()
	{
		int64_t val{ 2 };

		int64_t reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, 2,
			MOD, GRA, GRB,
			RET
		};

		int64_t literal_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOD, GRA, 2,
			RET
		};

		int64_t reg_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			MOD, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_address_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOD, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t reg_double_address_stub[]
		{
			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			MOD, DEREF, GRA, DEREF, GRB,
			RET
		};

		int64_t literal_double_address_stub[]
		{
			ENTRY,
			MOD, DEREF, reinterpret_cast<int64_t>( &val ), DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( reg_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( literal_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( reg_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( literal_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( reg_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 0 ) );
		registers.clear();

		vm._cpu.execute( literal_double_address_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 0 ) );
		registers.clear();
	}

	void inc_dec()
	{
		int64_t val{ 15 };

		int64_t inc_reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			INC, GRA,
			RET
		};

		int64_t dec_reg_stub[]
		{
			ENTRY,
			MOV, GRA, 15,
			DEC, GRA,
			RET
		};

		vm._cpu.execute( inc_reg_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 16 ) );
		registers.clear();

		vm._cpu.execute( dec_reg_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 14 ) );
		registers.clear();
	}

	void greater_less()
	{
		int64_t val{ 15 };

		int64_t greater_reg_stub_true[]
		{
			ENTRY,
			MOV, GRA, 16,
			GREATER, GRA, 15,
			RET
		};
		
		int64_t greater_literal_address_stub_true[]
		{
			ENTRY,
			MOV, GRA, 16,
			GREATER, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t greater_reg_address_stub_true[]
		{
			ENTRY,
			MOV, GRA, 16,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			GREATER, GRA, DEREF, GRB,
			RET
		};

		int64_t less_reg_stub_true[]
		{
			ENTRY,
			MOV, GRA, 14,
			LESS, GRA, 15,
			RET
		};
		
		int64_t less_literal_address_stub_true[]
		{
			ENTRY,
			MOV, GRA, 14,
			LESS, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t less_reg_address_stub_true[]
		{
			ENTRY,
			MOV, GRA, 14,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			LESS, GRA, DEREF, GRB,
			RET
		};

		int64_t greater_reg_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			GREATER, GRA, 15,
			RET
		};
		
		int64_t greater_literal_address_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			GREATER, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t greater_reg_address_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			GREATER, GRA, DEREF, GRB,
			RET
		};

		int64_t less_reg_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			LESS, GRA, 15,
			RET
		};
		
		int64_t less_literal_address_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			LESS, GRA, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		int64_t less_reg_address_stub_false[]
		{
			ENTRY,
			MOV, GRA, 15,
			MOV, GRB, reinterpret_cast<int64_t>( &val ),
			LESS, GRA, DEREF, GRB,
			RET
		};

		vm._cpu.execute( greater_reg_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( greater_reg_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( greater_reg_address_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( less_reg_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( less_literal_address_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( less_reg_address_stub_true );
		test_expected_value( registers.boolean_flag, true );
		registers.clear();

		vm._cpu.execute( greater_reg_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( greater_reg_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( greater_reg_address_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( less_reg_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( less_literal_address_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();

		vm._cpu.execute( less_reg_address_stub_false );
		test_expected_value( registers.boolean_flag, false );
		registers.clear();
	}

	void and_or_xor_not_lshift_rshift()
	{
		int64_t and_stub[]
		{
			ENTRY,
			MOV, GRA, 5,
			AND, GRA, 9,
			RET
		};

		int64_t or_stub[]
		{
			ENTRY,
			MOV, GRA, 5,
			OR, GRA, 9,
			RET
		};

		int64_t xor_stub[]
		{
			ENTRY,
			MOV, GRA, 5,
			XOR, GRA, 9,
			RET
		};

		int64_t not_stub[]
		{
			ENTRY,
			MOV, GRA, 5,
			NOT, GRA,
			RET
		};

		int64_t lshift_stub[]
		{
			ENTRY,
			MOV, GRA, 9,
			LSHIFT, GRA, 1,
			RET
		};

		int64_t rshift_stub[]
		{
			ENTRY,
			MOV, GRA, 9,
			RSHIFT, GRA, 1,
			RET
		};

		vm._cpu.execute( and_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( or_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 13 ) );
		registers.clear();

		vm._cpu.execute( xor_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 12 ) );
		registers.clear();

		vm._cpu.execute( not_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( -6 ) );
		registers.clear();

		vm._cpu.execute( lshift_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 18 ) );
		registers.clear();

		vm._cpu.execute( rshift_stub );
		test_expected_value( registers.accumulator, static_cast<int64_t>( 4 ) );
		registers.clear();
	}

	void no_reg()
	{
		int64_t stub[]
		{
			ENTRY,
			MOV, GRA, NOREG, -0x7E5120,
			RET
		};

		vm._cpu.execute( stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( -8278304 ) );
		registers.clear();
	}

	void jmp()
	{
		int64_t val{ 4 };

		int64_t jmp_reg_stub[]
		{
			MOV, GRA, -1,
			RET,

			MOV, GRA, 1,
			RET,

			ENTRY,
			MOV, GRA, 4,
			JMP, GRA,
			RET
		};

		int64_t jmp_literal_stub[]
		{
			MOV, GRA, 1,
			RET,

			ENTRY,
			JMP, 0,
			RET
		};

		int64_t jmp_address_reg_stub[]
		{
			MOV, GRA, -1,
			RET,

			MOV, GRA, 1,
			RET,

			ENTRY,
			MOV, GRA, reinterpret_cast<int64_t>( &val ),
			JMP, DEREF, GRA,
			RET
		};

		int64_t jmp_address_literal_stub[]
		{
			MOV, GRA, -1,
			RET,

			MOV, GRA, 1,
			RET,

			ENTRY,
			JMP, DEREF, reinterpret_cast<int64_t>( &val ),
			RET
		};

		vm._cpu.execute( jmp_reg_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 1 ) );
		registers.clear();

		
		vm._cpu.execute( jmp_literal_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( jmp_address_reg_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 1 ) );
		registers.clear();

		vm._cpu.execute( jmp_address_literal_stub );
		test_expected_value( registers.GRA, static_cast<int64_t>( 1 ) );
		registers.clear();
	}
};