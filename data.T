SECTIONS {
	. = 0x7E00;
	.text :
	{
		*(.start);
		*(.text*);
	}
	.data :
	{
		*(*);
	}
}
