typedef void(*init_func)(void);
#define UFCON0 ((volatile unsigned int *)(0x50000020))

static init_func init[]={
	arm920t_init_mmu,
	s3c2410_init_clock,
	s3c2410_init_memory,
	s3c2410_init_irq,
	s3c2410_init_io,
	NULL
};


void load_init_boot(init_func *init)
{
	int i;
	for(i=0;init[i];i++){
		init[i]();
	}
	boot_start();
}

void plat_boot(void)
{
	extern void test_vparameter(int,...);
	int i;
	//load_init_boot(init);
	for(i=0;init[i];i++){
		init[i]();
	}
	init_sys_mmu();
	start_mmu();
	test_mmu();
	test_vparameter(3,6,8,6);
	while(1);
}


