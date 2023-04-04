OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
/*
STARTUP(kernel/startup.o)
*/
MEMORY
{
    BOOT_INFO	: 	o = 0x3fffffe0,	l = 0x00000020
    ROM		: 	o = 0x40000000,	l = 0x00001000
    
    RAM     	:	o = 0x40001000,	l = 0x00000b00
    
    STACK     	:	o = 0x40001ff0,	l = 0x00000010

}

SECTIONS
{ 
    . = ALIGN(4);
    .boot_info :
    {
    	. = ALIGN(4);
        __BOOT_INFO_START = .;
        *(.boot_info);
        __BOOT_INFO_END = .;	
        . = ALIGN(4);
    }>BOOT_INFO
    
        . = ALIGN(4);
    .text :    
    {
        . = ALIGN(4);
        __TEXT_START = .;
        *(startup.o);
        *(.text);
        __TEXT_END = .;
        . = ALIGN(4);
	__INT_ENTRY_START = .;
	*(.irq_entry);
	__INT_ENTRY_END = .;
        . = ALIGN(4);

        __CMD_START = .;
        *(.boot_cmd);
        __CMD_END = .;
        . = ALIGN(4);
        __DEV_START = .;
        *(.device);
        *(.device.0);
        *(.device.1);
        *(.device.*);
        __DEV_END = .;
        . = ALIGN(4);
        __BLK_DEV_START = .;
        *(.blk_dev);
        __BLK_DEV_END = .;
        . = ALIGN(4);
        __FS_START = .;
        *(.fs);
        __FS_END = .;
	. = ALIGN(4);
        __init_struct_start = .;
        *(.init_struct)
	__init_struct_end = .;
        . = ALIGN(4);
        __initcall_start = .;
        *(.initcall1.init)
        *(.initcall3.init)
        __initcall_end = .;

        . = ALIGN(4);
	__NAND_TABLE_START = .;
        *(.nand_table);
        __NAND_TABLE_END = .;
        *(.textlib);
        . = ALIGN(4);
        *(.source);      
        . = ALIGN(4);
        __RODATA_START = .;
        *(.rodata);
        . = ALIGN(4);
	*(.rodata*);
        __RODATA_END = .;
        . = ALIGN(4);
	__GOT_START = .;
	*(.got*);
        . = ALIGN(4);
        __INIT_START = .;
        *(.init);
        __INIT_END = .;
        . = ALIGN(4);
        __DATA_START = .;
        *(.data);
        __DATA_END = .;
        . = ALIGN(4);
	
    } > ROM
    
    . = ALIGN(4);    
    
    .bss :
    {
        . = ALIGN(4);
	__BSS_START = .;
	*(.bss);
	__BSS_END = .;
        . = ALIGN(4);
    } > RAM
    
        . = ALIGN(4);
    . = ALIGN(4);    
    .glue_7t :
    {
	*(.glue_7t);
    } > RAM

    . = ALIGN(4);    
    .glue_7 :
    {
	__GLUE_7_START = .;
	*(.glue_7);
    } > RAM

    . = ALIGN(4);
    

    .stack :
    {
	__STACK_START = .;
    } > STACK

}

