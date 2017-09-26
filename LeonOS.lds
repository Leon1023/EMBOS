OUTPUT_ARCH(arm)
ENTRY(_startup)
SECTIONS
{
	. = 0x00000000;
	.text:
	{
		*(.startup)
		*(.text)
	}

	.align 32;
	.data:
	{
		*(.data)
	}

	.align 32;
	__bss_start__ = .;
	.bss:
	{
		*(.bss)
	}
	__bss_end__ = .;
}
