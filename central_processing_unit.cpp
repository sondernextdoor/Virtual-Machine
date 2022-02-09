#include "virtual_machine.h"

vm_registers virtual_machine::central_processing_unit::execute( int64_t* instructions, bool b_call, int64_t instruction_count )
{
	registers.IC = instruction_count;

	if ( instructions == nullptr )
	{ 
		return {}; 
	}

	if ( b_call == false )
	{
		for ( ;; )
		{
			if ( instructions[registers.IC++] == ENTRY )
			{
				break;
			}
		}
	}

	for ( ; instructions[registers.IC] != RET; registers.IC++ )
	{
		switch ( instructions[registers.IC] )
		{
			case PUSH:
			{
				push( instructions );
			} break;

			case POP:
			{
				pop( instructions );
			} break;

			case MOV:
			{
				mov( instructions );
			} break;

			case MOVB:
			{
				mov<int8_t>( instructions );
			} break;

			case MOVW:
			{
				mov<int16_t>( instructions );
			} break;

			case MOVD:
			{
				mov<int32_t>( instructions );
			} break;

			case MOVF:
			{
				movf( instructions );
			} break;

			case CMP:
			{
				cmp( instructions );
			} break;

			case ADD:
			{
				add( instructions );
			} break;

			case SUB:
			{
				sub( instructions );
			} break;

			case MUL:
			{
				mul( instructions );
			} break;

			case DIV:
			{
				div( instructions );
			} break;

			case MOD:
			{
				mod( instructions );
			} break;

			case INC:
			{
				inc( instructions );
			} break;

			case DEC:
			{
				dec( instructions );
			} break;

			case GREATER:
			{
				greater( instructions );
			} break;

			case LESS:
			{
				less( instructions );
			} break;

			case AND:
			{
				bitwise_and( instructions );
			} break;

			case OR:
			{
				bitwise_or( instructions );
			} break;

			case XOR:
			{
				bitwise_xor( instructions );
			} break;

			case NOT:
			{
				bitwise_not( instructions );
			} break;

			case LSHIFT:
			{
				left_shift( instructions );
			} break;

			case RSHIFT:
			{
				right_shift( instructions );
			} break;

			case JMP:
			{
				jmp( instructions );
			} break;

			case TJMP:
			{
				tjmp( instructions );
			} break;

			case FJMP:
			{
				fjmp( instructions );
			} break;

			case GETC:
			{
				get_char( instructions );
			} break;

			case STROUT:
			{
				str_out( instructions );
			} break;

			case NUMOUT:
			{
				num_out( instructions );
			} break;

			case HEXOUT:
			{
				num_out( instructions, true );
			} break;

			case STALL:
			{
				stall( instructions );
			} break;
		}
	}

	return registers;
}

void virtual_machine::central_processing_unit::push( int64_t* instructions )
{
	bool b_deref{ false };

	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}
	
	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				( 
					b_deref 
					? stack.push_back( *reinterpret_cast<int64_t*>( registers.get( i ) ) )
					: stack.push_back( registers.get( i ) )
				);

				registers.SP = reinterpret_cast<int64_t>( &stack.back() );
				registers.BP = reinterpret_cast<int64_t>( stack.data() );

				return;
			}
		}
	}

	( 
		b_deref 
		? stack.push_back( *reinterpret_cast<int64_t*>( instructions[registers.IC] ) )
		: stack.push_back( instructions[registers.IC] )
	);

	registers.SP = reinterpret_cast<int64_t>( &stack.back() );
	registers.BP = reinterpret_cast<int64_t>( stack.data() );
}

void virtual_machine::central_processing_unit::pop( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( stack.empty() )
	{
		registers.SP = 0;
		registers.BP = 0;
		return;
	}

	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}
	
	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				( 
					b_deref 
					? *reinterpret_cast<int64_t*>( registers.get( i ) ) = stack.back()
					: registers.get( i ) = stack.back()
				);
			
				stack.pop_back();

				if ( stack.empty() )
				{
					registers.SP = 0;
					registers.BP = 0;
				}
				else
				{
					registers.SP = reinterpret_cast<int64_t>( &stack.back() );
					registers.BP = reinterpret_cast<int64_t>( stack.data() );
				}

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	if ( b_deref != true )
	{
		return;
	}

	*reinterpret_cast<int64_t*>( instructions[registers.IC] ) = stack.back();
	stack.pop_back();

	if ( stack.empty() )
	{
		registers.SP = 0;
		registers.BP = 0;
	}
	else 
	{
		registers.SP = reinterpret_cast<int64_t>( &stack.back() );
		registers.BP = reinterpret_cast<int64_t>( stack.data() );
	}
}

template<typename T>
void virtual_machine::central_processing_unit::mov( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}
	
	T* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<T*>( registers.get( i ) )
				: reinterpret_cast<T*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		if ( b_deref == false )
		{
			return;
		}

		first_operand = reinterpret_cast<T*>( instructions[registers.IC] );
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				*first_operand = 
				( 
					b_deref 
					? *reinterpret_cast<T*>( registers.get( i ) )
					: registers.get( i )
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	*first_operand = 
	( 
		b_deref 
		? *reinterpret_cast<T*>( instructions[registers.IC] )
		: instructions[registers.IC]
	);
}

void virtual_machine::central_processing_unit::movf( int64_t* instructions )
{
	bool b_deref{};
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}
	
	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				( 
					b_deref 
					? *reinterpret_cast<int64_t*>( registers.get( i ) )
					: registers.get( i )
				) = registers.boolean_flag;

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	if ( b_deref == false )
	{
		return;
	}

	*reinterpret_cast<int64_t*>( instructions[registers.IC] ) = registers.boolean_flag;
}

void virtual_machine::central_processing_unit::cmp( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		if ( b_deref == false )
		{
			return;
		}

		first_operand = reinterpret_cast<int64_t*>( instructions[registers.IC] );
	}


	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.boolean_flag = 
				( 
					*first_operand == 
					( 
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					) 
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.boolean_flag = 
	( 
		*first_operand == 
		( 
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		) 
	);
}

void virtual_machine::central_processing_unit::add( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = 
				( 
					*first_operand +
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = 
	( 
		*first_operand +
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::sub( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = 
				( 
					*first_operand -
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = 
	( 
		*first_operand -
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::mul( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = 
				( 
					*first_operand *
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = 
	( 
		*first_operand *
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::div( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = 
				( 
					*first_operand /
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = 
	( 
		*first_operand /
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::mod( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = 
				( 
					*first_operand %
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = 
	( 
		*first_operand %
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::inc( int64_t* instructions )
{
	++registers.IC;

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			++registers.get( i );
			return;
		}
	}
}

void virtual_machine::central_processing_unit::dec( int64_t* instructions )
{
	++registers.IC;

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			--registers.get( i );
			return;
		}
	}
}

void virtual_machine::central_processing_unit::greater( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.boolean_flag = 
				( 
					*first_operand >
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.boolean_flag =
	( 
		*first_operand >
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::less( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = 
			( 
				b_deref 
				? reinterpret_cast<int64_t*>( registers.get( i ) )
				: reinterpret_cast<int64_t*>( &registers.get( i ) )
			);

			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = 
		( 
			b_deref 
			? reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: reinterpret_cast<int64_t*>( &instructions[registers.IC] )
		);
	}

	b_deref = ( instructions[++registers.IC] == DEREF );

	if ( b_deref == true )
	{
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.boolean_flag = 
				( 
					*first_operand <
					(
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) )
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.boolean_flag =
	( 
		*first_operand <
		(
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC]
		)
	);
}

void virtual_machine::central_processing_unit::bitwise_and( int64_t* instructions )
{
	++registers.IC;

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = reinterpret_cast<int64_t*>( &registers.get( i ) );
			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = reinterpret_cast<int64_t*>( &instructions[registers.IC] );
	}

	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( *first_operand & registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( *first_operand & instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::bitwise_or( int64_t* instructions )
{	
	++registers.IC;

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = reinterpret_cast<int64_t*>( &registers.get( i ) );
			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = reinterpret_cast<int64_t*>( &instructions[registers.IC] );
	}

	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( *first_operand | registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( *first_operand | instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::bitwise_xor( int64_t* instructions )
{	
	++registers.IC;

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = reinterpret_cast<int64_t*>( &registers.get( i ) );
			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = reinterpret_cast<int64_t*>( &instructions[registers.IC] );
	}

	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( *first_operand ^ registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( *first_operand ^ instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::bitwise_not( int64_t* instructions )
{	
	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( ~registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( ~instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::left_shift( int64_t* instructions )
{	
	++registers.IC;

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = reinterpret_cast<int64_t*>( &registers.get( i ) );
			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = reinterpret_cast<int64_t*>( &instructions[registers.IC] );
	}

	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( *first_operand << registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( *first_operand << instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::right_shift( int64_t* instructions )
{	
	++registers.IC;

	int64_t* first_operand{ nullptr };

	for ( uint64_t i = 0; i < register_count; i++ )
	{
		if ( instructions[registers.IC] == instruction_set.at( i ) )
		{
			first_operand = reinterpret_cast<int64_t*>( &registers.get( i ) );
			break;
		}
	}

	if ( first_operand == nullptr )
	{
		first_operand = reinterpret_cast<int64_t*>( &instructions[registers.IC] );
	}

	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				registers.accumulator = ( *first_operand >> registers.get( i ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	registers.accumulator = ( *first_operand >> instructions[registers.IC] );
}

void virtual_machine::central_processing_unit::jmp( int64_t* instructions )
{
	bool b_deref{ false };
	
	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				--( registers.IC = 
					( 
						b_deref 
						? *reinterpret_cast<int64_t*>( registers.get( i ) ) 
						: registers.get( i )
					)
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	--( registers.IC = 
		( 
			b_deref 
			? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
			: instructions[registers.IC] 
		) 
	);
}

void virtual_machine::central_processing_unit::tjmp( int64_t* instructions )
{
	if ( registers.boolean_flag == true )
	{
		jmp( instructions );
	}
}

void virtual_machine::central_processing_unit::fjmp( int64_t* instructions )
{
	if ( registers.boolean_flag == false )
	{
		jmp( instructions );
	}
}

void virtual_machine::central_processing_unit::stall( int64_t* instructions )
{
	if ( instructions[++registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				std::this_thread::sleep_for( std::chrono::milliseconds( registers.get( i ) ) );
				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	std::this_thread::sleep_for( std::chrono::milliseconds( instructions[registers.IC] ) );
}

void virtual_machine::central_processing_unit::get_char( int64_t* instructions )
{
	*reinterpret_cast<char*>( &registers.RR ) = getchar();
}

void virtual_machine::central_processing_unit::str_out( int64_t* instructions )
{
	bool b_deref{ false };

	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				std::cout << 
				( 
					b_deref 
					? *reinterpret_cast<char**>( registers.get( i ) )
					: reinterpret_cast<char*>( registers.get( i ) )
				);

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	std::cout << 
	( 
		b_deref 
		? *reinterpret_cast<char**>( instructions[registers.IC] )
		: reinterpret_cast<char*>( instructions[registers.IC] )
	);
}

void virtual_machine::central_processing_unit::num_out( int64_t* instructions, bool hex )
{
	bool b_deref{ false };

	if ( hex == true )
	{
		std::cout << std::hex;
	}

	if ( instructions[++registers.IC] == DEREF )
	{
		b_deref = true;
		++registers.IC;
	}

	if ( instructions[registers.IC] != NOREG )
	{
		for ( uint64_t i = 0; i < register_count; i++ )
		{
			if ( instructions[registers.IC] == instruction_set.at( i ) )
			{
				std::cout << 
				(	
					b_deref 
					? *reinterpret_cast<int64_t*>( registers.get( i ) )
					: registers.get( i )
				) << std::dec;

				return;
			}
		}
	}
	else
	{
		++registers.IC;
	}

	std::cout << 
	( 
		b_deref 
		? *reinterpret_cast<int64_t*>( instructions[registers.IC] )
		: instructions[registers.IC]
	) << std::dec;
}