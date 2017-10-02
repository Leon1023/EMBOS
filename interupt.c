#define INT_BASE	(0Xca000000)//因为使用了mmu，使用虚拟地址代替实际虚拟地址0x4a000000
#define INT_MASK	(INT_BASE+0X8)
#define INT_INTOFFSET	(INT_BASE+0X14)
#define INTPND		(INT_BASE+0X10)
#define SRCPND		(INT_BASE+0X0)

//开全局中断irq
void enable_irq(void)
{
	asm volatile(
		"mrs r4,cpsr\n\t"
		"bic r4,r4,#0x80\n\t"//将cpsr的I位(第7位)清零。
		"mrs cpsr,r4\n\t"
		://因为C无变量和汇编寄存器交流，故列表为空
		:
		:"r4"//需要保存r4的上下文环境
		);
}

//打开相应中断号的掩码，使之可发生中断
void umask_int(unsigned int offset)
{
	*(volatile unsigned int *)INTMASK&=~(1<<offset);//默认全1，清零打开
}

//中断处理函数
void common_irq_handler(void)
{
	unsigned int temp=(1<<(*(volatile unsigned int *)INTOFFSET));//根据中断号确定相应位
	printk("%d\t",*(volatile unsigned int *)INTOFFSET);
	*(volatile unsigned int *)SRCPND|=1<<temp;//通过向相应位写1，清除相应位的中断请求信号
	*(volatile unsigned int *)INTPND|=1<<temp;//同上
	printk("timer interupt occured!\n");//真正的中断处理程序位于此后
}


