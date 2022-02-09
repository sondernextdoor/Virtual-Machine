using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;


namespace VirtualMachine
{
    public partial class CreateFileWindow : Window
    {
        public CreateFileWindow()
        {
            InitializeComponent();

            this.Background = ( Brush )new BrushConverter().ConvertFrom( "#04181F" );
            this.textBlock.Foreground = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
            this.textBox.Background = ( Brush )new BrushConverter().ConvertFrom( "#B5C9D4" );
        }

        private void OnTextChanged( object sender, TextChangedEventArgs e )
        {
            FileSystem.createFileText = ( sender as TextBox ).Text;
        }

        unsafe private void OnCreate( object sender, RoutedEventArgs e )
        {
            if ( string.IsNullOrWhiteSpace( FileSystem.createFileText ) == true )
            {
                return;
            }

            while ( FileSystem.createFileText.StartsWith( ' ' ) )
            {
                FileSystem.createFileText = FileSystem.createFileText.Remove( 0, 1 );
            }

            FileSystem.CreateFile( FileSystem.createFileText );
            FileSystem.UpdateFileTable();
            this.Close();
        }
    }
}