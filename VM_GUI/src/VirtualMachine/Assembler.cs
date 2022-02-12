using System.Collections.Generic;
using System.Linq;
using ExtensionMethods;
using CPU = VirtualMachine.CPU;

unsafe public class Assembler
{
	static private readonly Dictionary<string, long> instructionMap = new();
	static private readonly Dictionary<string, int> labelTable = new();

	public struct Mnemonic
	{
		static public readonly long GRA = -0x7E5120;
		static public readonly long GRB = -0x7E5121;
		static public readonly long GRC = -0x7E5122;
		static public readonly long GRD = -0x7E5123;
		static public readonly long GRE = -0x7E5124;
		static public readonly long GRF = -0x7E5125;
		static public readonly long ACC = -0x7E5126;
		static public readonly long RR = -0x7E5127;
		static public readonly long SP = -0x7E5128;
		static public readonly long BP = -0x7E5129;
		static public readonly long IC = -0x7E5130;

		static public readonly long PUSH = -0x7E5131;
		static public readonly long POP = -0x7E5132;
		static public readonly long MOV = -0x7E5133;
		static public readonly long MOVB = -0x7E5134;
		static public readonly long MOVW = -0x7E5135;
		static public readonly long MOVD = -0x7E5136;
		static public readonly long MOVF = -0x7E5137;
		static public readonly long CMP = -0x7E5138;
		static public readonly long RET = -0x7E5139;

		static public readonly long ADD = -0x7E5140;
		static public readonly long SUB = -0x7E5141;
		static public readonly long MUL = -0x7E5142;
		static public readonly long DIV = -0x7E5143;
		static public readonly long MOD = -0x7E5144;
		static public readonly long INC = -0x7E5145;
		static public readonly long DEC = -0x7E5146;
		static public readonly long GREATER = -0x7E5147;
		static public readonly long LESS = -0x7E5148;

		static public readonly long AND = -0x7E5149;
		static public readonly long OR = -0x7E5150;
		static public readonly long XOR = -0x7E5151;
		static public readonly long NOT = -0x7E5152;
		static public readonly long LSHIFT = -0x7E5153;
		static public readonly long RSHIFT = -0x7E5154;

		static public readonly long JMP = -0x7E5155;
		static public readonly long TJMP = -0x7E5156;
		static public readonly long FJMP = -0x7E5157;

		static public readonly long GETC = -0x7E5160;
		static public readonly long STROUT = -0x7E5161;
		static public readonly long NUMOUT = -0x7E5162;
		static public readonly long HEXOUT = -0x7E5163;

		static public readonly long DEREF = -0x7E5164;
		static public readonly long ENTRY = -0x7E5165;
		static public readonly long NOREG = -0x7E5166;
		static public readonly long STALL = -0x7E5167;
	}

	static protected void InitializeMap()
	{
		instructionMap.Add( "gra", -0x7E5120 );
		instructionMap.Add( "grb", -0x7E5121 );
		instructionMap.Add( "grc", -0x7E5122 );
		instructionMap.Add( "grd", -0x7E5123 );
		instructionMap.Add( "gre", -0x7E5124 );
		instructionMap.Add( "grf", -0x7E5125 );
		instructionMap.Add( "acc", -0x7E5126 );
		instructionMap.Add( "rr", -0x7E5127 );
		instructionMap.Add( "sp", -0x7E5128 );
		instructionMap.Add( "bp", -0x7E5129 );
		instructionMap.Add( "ic", -0x7E5130 );
		instructionMap.Add( "push", -0x7E5131 );
		instructionMap.Add( "pop", -0x7E5132 );
		instructionMap.Add( "mov", -0x7E5133 );
		instructionMap.Add( "movb", -0x7E5134 );
		instructionMap.Add( "movw", -0x7E5135 );
		instructionMap.Add( "movd", -0x7E5136 );
		instructionMap.Add( "movf", -0x7E5137 );
		instructionMap.Add( "cmp", -0x7E5138 );
		instructionMap.Add( "ret", -0x7E5139 );
		instructionMap.Add( "add", -0x7E5140 );
		instructionMap.Add( "sub", -0x7E5141 );
		instructionMap.Add( "mul", -0x7E5142 );
		instructionMap.Add( "div", -0x7E5143 );
		instructionMap.Add( "mod", -0x7E5144 );
		instructionMap.Add( "inc", -0x7E5145 );
		instructionMap.Add( "dec", -0x7E5146 );
		instructionMap.Add( "greater", -0x7E5147 );
		instructionMap.Add( "less", -0x7E5148 );
		instructionMap.Add( "and", -0x7E5149 );
		instructionMap.Add( "or", -0x7E5150 );
		instructionMap.Add( "xor", -0x7E5151 );
		instructionMap.Add( "not", -0x7E5152 );
		instructionMap.Add( "lshift", -0x7E5153 );
		instructionMap.Add( "rshift", -0x7E5154 );
		instructionMap.Add( "jmp", -0x7E5155 );
		instructionMap.Add( "tjmp", -0x7E5156 );
		instructionMap.Add( "fjmp", -0x7E5157 );
		instructionMap.Add( "getc", -0x7E5160 );
		instructionMap.Add( "strout", -0x7E5161 );
		instructionMap.Add( "numout", -0x7E5162 );
		instructionMap.Add( "hexout", -0x7E5163 );
		instructionMap.Add( "deref", -0x7E5164 );
		instructionMap.Add( "entry", -0x7E5165 );
		instructionMap.Add( "noreg", -0x7E5166 );
		instructionMap.Add( "stall", -0x7E5167 );
		instructionMap.Add( "*", -0x7E5164 );
	}

	static private long FindOpcode( string instruction )
	{
        	if ( instructionMap.TryGetValue( instruction, out long value ) == false )
        	{
			return 0;
        	}

        	return value;
	}

	static private string[] FormatInstructions( string rawInstructions )
	{
		List<string> strings = new();
		List<string> labels = new();

		while ( true )
		{
			StringPosition commentPosition = rawInstructions.Find( "//*\r" );

			if ( commentPosition.Empty() == true )
			{
				break;
			}

			rawInstructions = rawInstructions.Remove( commentPosition.startOffset, commentPosition.Length() );
		}

		for ( int i = 0; i < rawInstructions.Length; i++ )
		{
			if ( rawInstructions[i].CompareTo( '\"' ) == 0 )
			{
				for ( int j = i + 1; j < rawInstructions.Length; j++ )
				{
					if ( rawInstructions[j].CompareTo( '\"' ) == 0 )
					{
						strings.Add( rawInstructions.Substring( i, j - i + 1 ) );

						for ( ; i < j + 1; i++ )
						{
							if ( rawInstructions[i].CompareTo( ' ' ) == 0 )
							{
								rawInstructions = rawInstructions.Remove( i, 1 );
								i -= 1;
								j -= 1;
							}
						}

						break;
					}
				}
			}

			if ( rawInstructions[i..].StartsWith( "label" ) == true )
			{
				while ( rawInstructions[i].CompareTo( '\"' ) != 0 )
                		{
					i += 1;
                		}

				for ( int j = i + 1; j < rawInstructions.Length; j++ )
				{
					if ( rawInstructions[j].CompareTo( '\"' ) == 0 )
					{
						labels.Add( rawInstructions.Substring( i, j - i + 1 ) );
						i = j;
						break;
					}
				}

				continue;
			}
		}

		List<string> instructions = new( rawInstructions.Split( ' ', '\r', '\n' ) );
		Dictionary<string, string> escapes = new();

		escapes.Add( @"\a", "\a" );
		escapes.Add( @"\b", "\b" );
		escapes.Add( @"\f", "\f" );
		escapes.Add( @"\n", "\n" );
		escapes.Add( @"\r", "\r" );
		escapes.Add( @"\t", "\t" );
		escapes.Add( @"\v", "\v" );
		escapes.Add( @"\\", "\\" );
		escapes.Add( @"\0", "\0" );
		escapes.Add( "\\\"", "\"" );

		for ( int i = 0, labelCounter = 0; i < instructions.Count; i++ )
        	{
			if ( instructions[i].StartsWith( new string( @"//" ) )
				 || instructions[i].Equals( "" ) || instructions[i].Equals( "\n" ) 
				 || instructions[i].Equals( "\r" ) || instructions[i].Equals( "\t" ) 
				 || string.IsNullOrWhiteSpace( instructions[i] ) )
			{
				instructions.RemoveAt( i );
			}

			if ( i < instructions.Count && ( instructions[i].Equals( "label" ) == true 
				|| instructions[i].Equals( "\nlabel" ) == true || instructions[i].Equals( "\rlabel" ) == true ) )
			{
				if ( labelTable.TryGetValue( labels[labelCounter], out int offset ) != true )
                		{
					labelTable.Add( labels[labelCounter], i );
				}

				labelCounter += 1;
				instructions.RemoveRange( i, 2 );
				i -= 1;
				continue;
			}
		}

		for ( int i = 0, stringCounter = 0; i < instructions.Count; i++ )
		{
			if ( instructions[i].Contains( "jmp" ) == true )
			{
				if ( instructions[i + 1].StartsWith( "\"" ) == true )
				{
					if ( labelTable.TryGetValue( instructions[i += 1], out int offset ) == true )
					{
						instructions[i] = offset.ToString();
					}

					continue;
				}
			}

			if ( stringCounter < strings.Count && instructions[i].StartsWith( '\"' ) == true )
			{
				instructions[i] = strings[stringCounter];
				stringCounter += 1;

				for ( int escapeCounter = 0; escapeCounter < escapes.Count; escapeCounter++ )
                		{
					instructions[i] = instructions[i].Replace
					( 
						escapes.ElementAt( escapeCounter ).Key, escapes.ElementAt( escapeCounter ).Value 
					);
                		}

				continue;
			}

			instructions[i] = instructions[i].RemoveIf( '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\0' );

			for ( int j = 0; j < instructions.Count; j++ )
			{
				if ( string.IsNullOrWhiteSpace( instructions[j] ) == true )
                		{
					instructions.RemoveAt( j );
					j -= 1;
                		}
			}		
		}

		return instructions.ToArray();
	}

	static public CPU.Registers Assemble( string rawInstructions )
	{
		List<long> opcodes = new();
        	string[] instructions = FormatInstructions( rawInstructions );

		for ( int i = 0; i < instructions.Length; i++ )
		{
			if ( instructions[i][0].CompareTo( '\"' ) == 0 )
			{
				opcodes.Add( instructions[i].RemoveIf( '\"' ).ToPointer().ToInt64() );
				continue;
			}

            		long opcode = FindOpcode( instructions[i] );

            		if ( opcode == 0 )
			{
				opcodes.Add( long.Parse( instructions[i] ) );
				continue;
			}

			opcodes.Add( opcode );

			if ( opcodes.Last() == Mnemonic.NOREG )
			{
				opcodes.Add( long.Parse( instructions[i += 1] ) );
			}
		}

		return CPU.Execute( opcodes.ToArray() );
	}

	static public void Initialize()
	{
		CPU.Initialize();
		InitializeMap();
	}
}
