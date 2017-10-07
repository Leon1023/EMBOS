typedef void(*init_func)(void);
#define UFCON0		((volatile unsigned int *)(0x50000020))
#define TIMER_BASE	(0xd1000000)//定时器寄存器初址，虚拟地址
#define TCFG0		((volatile unsigned int *)(TIMER_BASE+0x0))
#define TCFG1		((volatile unsigned int *)(TIMER_BASE+0x4))
#define TCON		((volatile unsigned int *)(TIMER_BASE+0x8))
#define TCONB4		((volatile unsigned int *)(TIMER_BASE+0x3c))

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

//定时器4初始化
void timer_init(void)
{
	*TCFG0|=0x800;//一级8分频，二级默认值(二分频，中断方式)
	*TCON|&=(~(7<<20));//定时器4控制位清零
	*TCON|=(1<<22);//自动装载初值
	*TCONB4=1000;//初值设为1000
	*TCON|=(1<<21);//手动装初值开始
	*TCON|=~(1<<21);//手动装初值结束
	*TCON|=(0<<20);//启动减数器

	umask_init(14);//打开定时器中断掩码
	enable_irq();//全局中断使能
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
	test_printk();
	//test_vparameter(3,6,8,6);
	//timer_init();
	init_page_map();
	kmalloc_init();
	char *p1,*p2,*p3,*p4;
//	p1=(char *)get_free_pages(0,6);
	p1=kmalloc(127);
	printk("the return address of p1:%x\n",p1);
//	p2=(char *)get_free_pages(0,6);
	p2=kmalloc(124);
	printk("the return address of p1:%x\n",p2);
	//put_free_pages(p2);
	//put_free_pages(p1);
	kfree(p1);
	kfree(p2);
	//p3=(char *)get_free_pages(0,7);
	p3=kmalloc(119);
	printk("the return address of p1:%x\n",p3);
//	p4=(char *)get_free_pages(0,7);
	p4=kmalloc(512);
	printk("the return address of p1:%x\n",p4);
	while(1);
}


