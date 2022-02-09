# Virtual-Machine

A Turing-complete 64-bit virtual machine developed in C++ and C#

# Structure & Design

The GUI and assembler are implemented in C# (WPF). The VM is meant to be built as a .dll and loaded into the GUI/client program; from there, necessary functionality is exposed to the GUI.

The virtual machine is implemented as a class that contains two nested classes; central_processing_unit and storage. The virtual machine class has one public method (the constructor) and two public member variables; a central_processing_unit object and a storage object. 
(virtual_machine.h/virtual_machine.cpp)

The central_processing_unit class represents the processor of the virtual machine. It contains the logic that receives, interprets, and executes any valid given assembly instructions. The class contains one public method and two public member variables; one of type vm_registers representing the CPU registers and the other a vector of type int64_t representing the stack. In addition, it contains twenty-seven private methods which provide the functionality required to carry out the various computational operations. (virtual_machine.h/central_processing_unit.cpp)

The storage class represents the storage device of the virtual machine. It contains the logic that creates the native file used as the virtual storage device. It contains three public methods and four public member variables; one of type string that contains the path to the native file, one of type fstream that serves as the data stream for the native file, and two of type uint64_t, one that contains the maximum size of the native file and one that contains the number of bytes currently in use by the native file.
(virtual_machine.h/storage.cpp)

As mentioned previously, the virtual CPU contains a member variable of type vm_registers. vm_registers is a simple data structure consisting of six 64-bit general purpose registers (GRA-GRF), five 64-bit specialized registers (ACC/accumulator, RR/return register, SP/stack pointer, BP/base pointer and IC/instruction counter) and one boolean flag. In addition, it contains six public methods; three for accessing the lower 32/16/8 bits of a given register, one for returning the proper register when passed an index, one for obtaining a reference to the structure, and one for clearing all registers.
(registers.h/registers.cpp)

A virtual file system is implemented separate from the virtual_machine class. It contains the logic necessary to create, read, write and delete files within the native file (i.e. the VM storage device). Persistent storage is achieved through use of metadata specifying the file name and the file start offset. From these two data-points, the size of each file can be ascertained and then subsequently read. Metadata is stored using the following syntactic prepend: "[file name] | [size]". The logic for the virtual file system is located within its own namespace, vm_filesystem.
(file_system.h/file_system.cpp)

# Registers & Flags

In total, there are 11 registers and one boolean flag. 


  GRA
  
    General register A. 
    64-bit register used for any given operations
    
  GRB
  
    General register B. 
    64-bit register used for any given operations
    
  GRC
  
    General register C. 
    64-bit register used for any given operations
    
  GRD
  
    General register D. 
    64-bit register used for any given operations
    
  GRE
  
    General register E. 
    64-bit register used for any given operations
    
  GRF
  
    General register F. 
    64-bit register used for any given operations
    
  ACC
  
  
    Accumulator. 
    64-bit register used to store the result of any mathematical calculation 
    e.g. ADD, GRA, 10 would store the result in the accumulator.
    
  RR
  
    Return register. 
    64-bit register that can be used for any given operations.
    Ideally, it should be reserved for storing the return value from any external calls.
    
  SP
  
    Stack pointer. 
    64-bit register that holds a pointer that will always point to the top of the stack. 
    If the stack is empty, it’ll be 0.
    
  BP
  
    Base pointer. 
    64-bit register that holds a pointer that will always point to the bottom of the stack. 
    If the stack is empty, it’ll be 0.
    
  IC
  
    Instruction counter. 
    64-bit register that holds the current instruction count.
    e.g. mov (0) gra (1) grb (2).
    
  Boolean Flag
  
    Boolean flag used in comparison operations.
    

# Instructions
    push
    
      Writes a value to the stack in LIFO order.
      
    pop 
    
      Removes a value from the stack in LIFO order.
      
    mov
    
      Moves a 64-bit value from operand two to operand one. 
      Operand one must be a register or a memory address, and operand two can be anything.
      
    movb
    
      Moves a 8-bit value from operand two to operand one. 
      Operand one must be a register or a memory address, and operand two can be anything.
      
    movw
    
      Moves a 16-bit value from operand two to operand one. 
      Operand one must be a register or a memory address, while operand two can be anything.
      
    movd
    
      Moves a 32-bit value from operand two to operand one. 
      Operand one must be a register or a memory address, while operand two can be anything.
      
    movf
      
      Moves the state of the boolean flag into the desired register or memory location.
      
    cmp
    
      Compares operand one against operand two for equality and sets the boolean flag accordingly.
      
    ret
    
      Ends execution.
      
    add
    
      Adds the first and second operands and stores the value in the accumulator.
      
    sub
    
      Subtracts the first and second operands and stores the value in the accumulator.
      
    mul
    
      Multiplies the first and second operands and stores the value in the accumulator.
      
    div
    
      Divides the first and second operands and stores the value in the accumulator.
      
    mod
    
      Obtains the remainder from the division of operand one and two and stores the value in the accumulator.
      
    inc 
    
      Increments the specified register.
      
    dec
    
      Decrements the specified register.
      
    greater
    
      Compares operand one against operand two and sets the boolean flag according to whether operand one is greater than operand two.
      
    less
    
      Compares operand one against operand two and sets the boolean flag according to whether operand one is less than operand two.
     
    and
      
      Performs a bitwise-AND between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    or
    
      Performs a bitwise-OR between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    xor
    
      Performs a bitwise-XOR between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    not
    
      Performs a bitwise-NOT between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    lshift
    
      Performs a bitwise-shift (left) between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    rshift
    
      Performs a bitwise-shift (right) between the first and second operands.
      Operand one must be a register or a memory address, and operand two can be anything.
      
    jmp
    
      Sets the instruction counter to the specified value.
      
    tjmp
    
      Sets the instruction counter to the speicifed value if the boolean flag is true.
      
    fjmp
    
      Sets the instruction counter to the speicifed value if the boolean flag is false.
      
    getc
    
      Prompts for character input and sets the return register to the value recieved.
      
    strout
    
      Outputs the specified string/char pointer to the console.
      
    numout
    
      Outputs the specified value in decimal to the console.
      
    hexout
    
      Outputs the specified value in hexadecimal to the console.
      
    deref
    
      Instructs the CPU to dereference the next value, be it a register or a literal address.
      
    entry
    
      The starting point of execution. This must be present in any valid asm code.
    
    noreg
    
      Instructs the CPU to treat the next value as a literal. 
      This is only required if the literal matches one of the register representations.
      e.g. MOV, GRA, -0x7E5120 will evaluate to MOV, GRA, GRA
      MOV, GRA, NOREG, -0x7E5120 will move -0x7E5120 into GRA.
      
    stall
    
      Stops execution until the desired time has passed. 
      Operand two must be a register or a literal specifying the desired time to stall in milliseconds.
