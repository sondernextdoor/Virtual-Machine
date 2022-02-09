using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Runtime.InteropServices;
using ExtensionMethods;

namespace ExtensionMethods
{
	public struct StringPosition
	{
		public int startOffset;
		public int endOffset;
		public string str;

		public StringPosition( int start, int end, string subStr )
		{
			startOffset = start;
			endOffset = end;
			str = subStr;
		}

		public int Length()
        {
			return endOffset - startOffset + 1;
        }

		public bool Empty()
        {
			return ( startOffset == -1 || endOffset == -1 || str == null );
        }

		public static StringPosition Null()
        {
			return new( -1, -1, null );
        }
	}


	public static class StringExtensions
	{
		public static IntPtr ToPointer( this string str )
        {
			return Marshal.StringToHGlobalAnsi( str );
		}

		public static string RemoveIf( this string str, params char[] characters )
        {
			foreach ( char c in characters )
            {
				if ( str.Contains( c ) == true )
                {
					str = str.Replace( c.ToString(), string.Empty );
                }
            }

			return str;
		}

		public static string ReplaceIf( this string str, char with, params char[] _if )
		{
			foreach ( char c in _if )
			{
				if ( str.Contains( c ) == true )
				{
					str = str.Replace( c.ToString(), with.ToString() );
				}
			}

			return str;
		}

		public static string ReplaceAt( this string str, int index, char with )
        {
			return ( str.ToCharArray()[index] = with ).ToString();
		}

		public static StringPosition Find( this string str, string toFind, int startIndex = 0, string cantBeAfter = null )
		{
			for ( int i = startIndex; i < str.Length; i++ )
			{
				if ( str[i] != toFind[0] )
                {
					continue;
                }

				if ( toFind.Length == 1 )
                {
					return new( i, i, str[i].ToString() );
                }

				for ( int j = i + 1, k = 1; j < str.Length && k < toFind.Length; j++, k++ )
                {
					if ( toFind[k] == '*' )
					{
						if ( ( k += 1 ) >= toFind.Length )
                        {
							return new( i, str.Length, str[i..] );
                        }

						while ( str[j] != toFind[k] )
                        {
							j += 1;

							if ( j >= str.Length )
                            {
								return StringPosition.Null();
                            }
                        }

						if ( ( k += 1 ) >= toFind.Length )
                        {
							return new( i, j, str[i..j] );
                        }

						if ( ( j += 1 ) >= str.Length )
                        {
							return StringPosition.Null();
                        }
					}

					if ( str[j] == toFind[k] || toFind[k] == '?' )
                    {
						if ( k + 1 >= toFind.Length )
                        {
							return new( i, j, str[i..j] );
                        }
                    }
                }
			}

			return StringPosition.Null();
		}
	}

	public static class ListExtensions
	{

		public static List<T> ReplaceAt<T>( this List<T> list, int index, T with )
		{
			list.RemoveAt( index );
			list.Insert( index, with );

			return list;
		}
	}
}

namespace VirtualMachine
{
    public partial class MainWindow : Window
	{

		private string asmTextBoxContents = string.Empty;
		private readonly long[] instructions = Array.Empty<long>();
		private string selectedFile = string.Empty;
		private CPU.Registers registers = new();
		private bool hexDisplay = false;

		public class FileMenuDisplay
		{
			public bool Display { get; set; }
		}

		public MainWindow()
		{
			InitializeComponent();

			this.Background = ( Brush )new BrushConverter().ConvertFrom( "#04181F" );
			this.registerList.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
			this.stackList.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
			this.asmTextBox.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
			this.fileListBox.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
			this.writeFileText.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );

			asmTextBox.AppendText
			(
				@"//" + " this is a comment\r" +
				"label \"myLabel\"\r" +
				"strout \"Bye World" + @"\n" + "\"\r" +
				"ret\r" +
				"entry\r" +
				"push 40\r" +
				"push 80\r" +
				"pop grc\r" +
				"mov gra 10\r" +
				"mov grb 15\r" +
				"add gra grb\r" +
				"mov rr acc\r" +
				"cmp rr acc\r" +
				"strout \"Hello World" + @"\n" + "\"\r" +
				"jmp \"myLabel\""
			);

			[DllImport( "kernel32.dll" )]
			[return: MarshalAs( UnmanagedType.Bool )]
			static extern bool AllocConsole();

			AllocConsole();
			FileSystem.fileList = fileListBox;
			FileSystem.UpdateFileTable();
			Assembler.Initialize();

			fileMenu.DataContext = new FileMenuDisplay() 
			{ 
				Display = FileSystem.fileTable.Count != 0 
			};
		}

		public void UpdateRegisterList()
		{
			registerList.Items[0] = "GRA: " + ( hexDisplay ? "0x" + registers.GRA.ToString( "X" ) : registers.GRA.ToString() );
			registerList.Items[1] = "GRB: " + ( hexDisplay ? "0x" + registers.GRB.ToString( "X" ) : registers.GRB.ToString() );
			registerList.Items[2] = "GRC: " + ( hexDisplay ? "0x" + registers.GRC.ToString( "X" ) : registers.GRC.ToString() );
			registerList.Items[3] = "GRD: " + ( hexDisplay ? "0x" + registers.GRD.ToString( "X" ) : registers.GRD.ToString() );
			registerList.Items[4] = "GRE: " + ( hexDisplay ? "0x" + registers.GRE.ToString( "X" ) : registers.GRE.ToString() );
			registerList.Items[5] = "GRF: " + ( hexDisplay ? "0x" + registers.GRF.ToString( "X" ) : registers.GRF.ToString() );
			registerList.Items[6] = "ACC: " + ( hexDisplay ? "0x" + registers.accumulator.ToString( "X" ) : registers.accumulator.ToString() );
			registerList.Items[7] = "RR: " + ( hexDisplay ? "0x" + registers.RR.ToString( "X" ) : registers.RR.ToString() );
			registerList.Items[8] = "SP: " + ( hexDisplay ? "0x" + registers.SP.ToString( "X" ) : registers.SP.ToString() );
			registerList.Items[9] = "BP: " + ( hexDisplay ? "0x" + registers.BP.ToString( "X" ) : registers.BP.ToString() );
			registerList.Items[10] = "IC: " + ( hexDisplay ? "0x" + registers.IC.ToString( "X" ) : registers.IC.ToString() );
			registerList.Items[11] = "Boolean Flag: " + registers.booleanFlag.ToString();
		}

		unsafe public void UpdateStackList()
        {
			ulong size = 0;
			long* stack = CPU.GetStack( &size );

			stackList.Items.Clear();

			if ( size == 0 )
            {
				return;
            }

			for ( int i = ( int )size - 1;; i-- )
			{
				stackList.Items.Add( stack[i] );
				
				if ( i == 0 )
                {
					break;
                }
			}
        }

		private void OnRegisterRightClick( object sender, RoutedEventArgs e )
        {
			hexDisplay = !hexDisplay;
			UpdateRegisterList();
        }

		private void OnAssemblerTextChanged( object sender, TextChangedEventArgs e )
        {
			TextRange textRange = new
			(
				asmTextBox.Document.ContentStart,
				asmTextBox.Document.ContentEnd
			);

			asmTextBoxContents = textRange.Text;

			if ( asmTextBoxContents.EndsWith( '\r' ) == true )
            {
				return;
            }

			FormattedText asmFormattedText = new
			( 
				asmTextBoxContents, System.Globalization.CultureInfo.CurrentCulture, FlowDirection.LeftToRight, 
				new Typeface( asmTextBox.FontFamily, asmTextBox.FontStyle, asmTextBox.FontWeight, asmTextBox.FontStretch ), 
				asmTextBox.FontSize, Brushes.Black 
			);

			asmTextBox.Document.PageWidth = asmFormattedText.Width + 12;
			asmTextBox.HorizontalScrollBarVisibility = ( asmTextBox.Width < asmFormattedText.Width + 12 ) ? ScrollBarVisibility.Visible : ScrollBarVisibility.Hidden;
		}

        private void OnAssemblerExecute( object sender, RoutedEventArgs e )
        {
			registers = Assembler.Assemble( asmTextBoxContents );
			UpdateRegisterList();
			UpdateStackList();
        }

		private void OnFileCreate( object sender, RoutedEventArgs e )
		{
			CreateFileWindow createFileWindow = new();
			createFileWindow.ResizeMode = ResizeMode.CanMinimize;
			createFileWindow.ShowDialog();

			if ( FileSystem.fileTable.Count != 0 )
            {
				fileMenu.DataContext = true;
            }
		}

		unsafe private void OnFileWrite( object sender, RoutedEventArgs e )
		{
			if ( fileListBox.SelectedItem != null )
            {
				if ( FileSystem.TryGetFileTableEntry( fileListBox.SelectedItem.ToString(), out FileSystem.File file ) == true )
                {
					string text = new TextRange( writeFileText.Document.ContentStart, writeFileText.Document.ContentEnd ).Text;
					text = text.RemoveIf( '\a', '\b', '\n', '\f', '\t', '\v', '\0' );

					if ( text.Length < ( int )file.size )
                    {
						if ( FileSystem.ClearFile( file.name ) == false )
                        {
							return;
                        }
                    }

					FileSystem.WriteFile( fileListBox.SelectedItem.ToString(), text, ( ulong )text.Length );
					FileSystem.UpdateFileTable();
				}
            }
		}

        private void OnFileSelect( object sender, RoutedEventArgs e )
        {
			if ( ( sender as ListBox ).SelectedItem == null )
            {
				return;
            }

			selectedFile = ( sender as ListBox ).SelectedItem.ToString();
			writeFileText.Document.Blocks.Clear();

			if ( FileSystem.TryGetFileTableEntry( selectedFile, out FileSystem.File file ) == true )
			{
				if ( file.size == 0 )
				{
					return;
				}

				string fileContent = FileSystem.ReadFile( file.name, file.size );
				writeFileText.AppendText( fileContent );
			}
		}

        unsafe private void OnFileDelete( object sender, RoutedEventArgs e )
        {
			if ( FileSystem.TryGetFileTableEntry( fileListBox.SelectedItem.ToString(), out FileSystem.File file ) == true )
			{
				if ( FileSystem.DeleteFile( file.name ) == true )
                {
					FileSystem.UpdateFileTable();
					fileListBox.Items.Remove( fileListBox.SelectedItem );
					writeFileText.Document.Blocks.Clear();

					if ( FileSystem.fileTable.Count == 0 )
                    {
						fileMenu.DataContext = true;
					}
				}
			}
        }
    }
}