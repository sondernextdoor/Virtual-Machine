using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Controls;
using System.Runtime.InteropServices;
using ExtensionMethods;

namespace VirtualMachine
{
    	unsafe public class FileSystem
	{
		public const int maxPath = 260;

		[StructLayout( LayoutKind.Sequential, CharSet = CharSet.Ansi )]
		public struct File
		{
			[MarshalAs( UnmanagedType.ByValTStr, SizeConst = maxPath )]
			public string name;
			public ulong start;
			public ulong size;
			public ulong metadataSize;
		}

		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "create_file" )] static extern bool CreateFileVM( IntPtr name );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "read_file" )] static extern bool ReadFileVM( IntPtr name, IntPtr buffer, ulong size, ulong offset );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "write_file" )] static extern bool WriteFileVM( IntPtr name, IntPtr buffer, ulong size, ulong offset );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "delete_file" )] static extern bool DeleteFileVM( IntPtr name );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "clear_file" )] static extern bool ClearFileVM( IntPtr name );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "get_file_array" )] static extern IntPtr GetFileArrayVM( ulong* outTableSize );
		[DllImport( "vm-lib.dll", CharSet = CharSet.Ansi, EntryPoint = "free_file_array" )] static extern void FreeFileArrayVM();

		static public string createFileText = string.Empty;
		static public string writeFileText = string.Empty;
		static public ListBox fileList = new();
		static public List<File> fileTable = new();

		static public bool TryGetFileTableEntry( string fileName, out File fileTableEntry )
		{
			File fileEntry = new();
			bool found = false;

			fileTable.ForEach( delegate ( File file ) 
			{ 
				if ( file.name.Equals( fileName ) == true ) 
				{
					fileEntry = file;
					found = true; 
				} 
			});

			fileTableEntry = fileEntry;
			return found;
		}

		static public bool CreateFile( string name )
		{
			return CreateFileVM( Marshal.StringToHGlobalAnsi( name ) );
		}

		static public bool WriteFile( string name, string buffer, ulong size, ulong offset = 0 )
		{
			if ( buffer.Last().Equals( '\r' ) == true )
			{
				buffer = buffer.Remove( buffer.LastIndexOf( '\r' ) );
				size -= 1;
			}

			return WriteFileVM( name.ToPointer(), buffer.ToPointer(), size, offset );
		}

		static public bool DeleteFile( string name )
		{
			return DeleteFileVM( name.ToPointer() );
		}

		static public bool ClearFile( string name )
		{
			return ClearFileVM( name.ToPointer() );
		}

		static public string ReadFile( string name, ulong size, ulong offset = 0 )
		{
			IntPtr buffer = Marshal.AllocHGlobal( ( int )size );
			ReadFileVM( name.ToPointer(), buffer, size, offset );
			string fileData = Marshal.PtrToStringAnsi( buffer, ( int )size );
			Marshal.FreeHGlobal( buffer );

			return fileData;
		}

		static public List<File> UpdateFileTable()
		{
			ulong size = 0;
			IntPtr fileArray = GetFileArrayVM( &size );

			for ( int i = 0; ( ulong )i < size; i++ )
			{
				File file = Marshal.PtrToStructure<File>( *( IntPtr* )( fileArray + ( sizeof( void* ) * i ) ) );

				if ( TryGetFileTableEntry( file.name, out _ ) == false )
				{
					fileTable.Add( file );
					fileList.Items.Add( file.name );
				}
				else
				{
					fileTable.ReplaceAt( i, file );
				}
			}

			FreeFileArrayVM();
			return fileTable;
		}
	}
}
