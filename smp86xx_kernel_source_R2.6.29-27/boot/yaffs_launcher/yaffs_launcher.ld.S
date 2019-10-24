ENTRY(__start)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  .text APPLET_ADDR :
  {
    _ftext = . ;
    start.o (.text)		/* __start entry point		    */
    *(.text)
    _ecode = . ;	/* End of code 			    */
    *(.rodata)
    *(.rodata.*)
    *(.rodata1)
    __mykey_start = .;
    *(.mykey)
    __mykey_end = .;
  } =0

  . = ALIGN(16);
  _etext = .;

  .data :
  AT( _etext )
  {
    _fdata = .;	/* Start of initialised data	    */
    *(.data)
   
    . = ALIGN(8);

    _gp = . + 0x7ff0; /* Base of small data		    */

    *(.lit8) 
    *(.lit4) 
    *(.sdata) 

    . = ALIGN(8);

    _edata  = .;	/* End of initialised data	    */
  }

  _fbss = .;
  .sbss : 
  { 
    *(.sbss) 
    *(.scommon)
  }
  .bss :
  {
    *(.bss)
    *(COMMON)
  }
  . = ALIGN(256);
  _end = . ;

  _end_stack = . + STACK_SIZE;

  /* Sections to be discarded */
  /DISCARD/ :
  {
    	*(.reginfo)
  }
}
