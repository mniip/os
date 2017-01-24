SECTIONS {
	. = 0x7E00;
	.text :
	{
		*(.start);
		*(.text*);
	}
	.data :
	{
		_symtab = .;
		*(.psymtab);
		_symtab_end = .;
		_strtab = .;
		*(.pstrtab);
		_strtab_end = .;
		*(*);
		_end = .;
	}
}
