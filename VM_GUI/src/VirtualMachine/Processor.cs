using System.Runtime.InteropServices;

namespace VirtualMachine
{
	unsafe public static class CPU
	{
		[DllImport( "vm-lib.dll", EntryPoint = "initialize_vm" )] private static extern void InitializeVM();
		[DllImport( "vm-lib.dll", EntryPoint = "execute" )] private static extern Registers ExecuteVM( long[] instructions );
		[DllImport( "vm-lib.dll", EntryPoint = "get_stack" )] private static extern long* GetStackVM( ulong* outSize );

		public struct Registers
		{
			public long GRA;
			public long GRB;
			public long GRC;
			public long GRD;
			public long GRE;
			public long GRF;
			public long accumulator;
			public long RR;
			public long SP;
			public long BP;
			public long IC;
			public bool booleanFlag;
		}

		static public void Initialize()
		{
			InitializeVM();
		}

		static public Registers Execute( long[] instructions )
		{
			return ExecuteVM( instructions );
		}

		static public long* GetStack( ulong* outSize )
		{
			return GetStackVM( outSize );
		}
	}
}