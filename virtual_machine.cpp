#include "virtual_machine.h"
#include "file_system.h"

virtual_machine::virtual_machine()
{
	_storage = storage();
	vm_filesystem::initialize_file_system( &_storage );
}