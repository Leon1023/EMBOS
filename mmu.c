#define PTE_L1_SECTION_PADDR_BASE_MASK  (0xfff00000)//截取物理地址高12位，也即页表项偏移量
#define PTE_BITS_L1_SECTION		(0x2)//段页表项标志位，位于页表项bit(1~0)

#define PAGE_TABLE_L1_BASE_ADDR_MASK	(0xfffc0000)//截取段页表基地址高18位，并将低14位清零
#define VIRT_TO_PTE_L1_INDEX(addr)	(((addr)&0xfff00000)>>18)//截取虚拟地址高12位，地址对齐后作为段页表项地址的低14位。

#define MEM_MAP_SIZE 			0x800000
#define PHYSICAL_MEM_ADDR		0x30000000
#define	VIRTUAL_MEM_ADDR		0x30000000

#define PTE_ALL_AP_L1_SECTION_DEFAULT	(0x1<<10)
#define PTE_L1_SECTION_NO_CACHE_AND_WB	(0x0<<2)
#define	PTE_L1_SECTION_DOMAIN_DEFAULT	(0x0<<5)

#define	L1_PTE_BASE_ADDR		0x30700000

#define IO_MAP_SIZE 			0x18000000
#define PHYSICAL_IO_ADDR		0x48000000
#define	VIRTUAL_IO_ADDR			0xc8000000

#define VECTOR_MAP_SIZE 		0x100000
#define PHYSICAL_VECTOR_ADDR		0x30000000
#define	VIRTUAL_VECTOR_ADDR		0x0

//根据物理地址求出段页表项的基本内容
unsigned int gen_l1_pte(unsigned int paddr)
{
	return(paddr&PTE_L1_SECTION_PADDR_BASE_MASK|PTE_BITS_L1_SECTION);
}

//根据段页表项基址和虚拟地址前12位生成段页表项地址
unsigned int gen_l1_pte_addr(unsigned int baddr,unsigned int vaddr)
{
	return(baddr&PAGE_TABLE_L1_BASE_ADDR_MASK|VIRT_TO_PTE_L1_INDEX(vaddr));
}
//初始化MMU
void init_sys_mmu(void)
{
	unsigned int pte,pte_addr;//分别定义段页表项和段页表项地址
	int j;
	//内存地址映射0x30000000-->0x30000000
	for(j=0;j<MEM_MAP_SIZE>>20;j++){//一个页表项对应1M的空间，共需MEM_SIZE/1M次映射
		pte=gen_l1_pte(PHYSICAL_MEM_ADDR+(j<<20));
		pte|=PTE_ALL_AP_L1_SECTION_DEFAULT;
		pte|=PTE_L1_SECTION_NO_CACHE_AND_WB;
		pte|=PTE_L1_SECTION_DOMAIN_DEFAULT;
		pte_addr=gen_l1_pte_addr(L1_PTE_BASE_ADDR,VIRTUAL_MEM_ADDR+(j<<20));
		*(volatile unsigned int *)pte_addr=pte;
	}
	//IO地址映射0xc8000000-->0x48000000
	for(j=0;j<IO_MAP_SIZE>>20;j++){
		pte=gen_l1_pte(PHYSICAL_IO_ADDR+(j<<20));
		pte|=PTE_ALL_AP_L1_SECTION_DEFAULT;
		pte|=PTE_L1_SECTION_NO_CACHE_AND_WB;
		pte|=PTE_L1_SECTION_DOMAIN_DEFAULT;
		pte_addr=gen_l1_pte_addr(L1_PTE_BASE_ADDR,VIRTUAL_IO_ADDR+(j<<20));
		*(volatile unsigned int *)pte_addr=pte;
	}
	//地址映射0x30000000-->0x0
	for(j=0;j<MEM_MAP_SIZE>>20;j++){//一个页表项对应1M的空间，共需MEM_SIZE/1M次映射
		pte=gen_l1_pte(PHYSICAL_VECTOR_ADDR+(j<<20));
		pte|=PTE_ALL_AP_L1_SECTION_DEFAULT;
		pte|=PTE_L1_SECTION_NO_CACHE_AND_WB;
		pte|=PTE_L1_SECTION_DOMAIN_DEFAULT;
		pte_addr=gen_l1_pte_addr(L1_PTR_BASE_ADDR,VIRTUAL_VECTOR_ADDR+(j<<20));
		*(volatile unsigned int *)pte_addr=pte;
	}
}

//启动MMU
void start_mmu(void)
{
	unsigned int ttb=L1_PTR_BASE_ADDR;//将段页表基址赋给ttb变量
	asm{
		"mcr p15,0,%0,c2,c0,0\n"//%0x代表输出输入列表中的第一个参数，此处即指ttb。语句功能是把ttb赋给c2寄存器，它专门用来存放页表基址的
		"mvn r0,#0\n"//立即数0取反后赋给r0
		"mcr p15,0,r0,c3,c0,0\n"//c3是域权限控制寄存器，此处将所有16个域都赋11，表示所有用户可读写
		"mov r0,#0x1\n"//
		"mcr p15,0,r0,c1,c0,0\n"//c1寄存器最低位赋1，表示启动MMU
		"mov r0,r0\n"//无用语句，清除指令流水线缓存的指令
		"mov r0,r0\n"
		"mov r0,r0\n"
		://汇编向c语言输出的参数列表
		:"r"(ttb)//由c语言向汇编输入的参数列表,修饰符r:通用寄存器，m:内存地址，l:立即数，X或&:只作为输出，缺省:只读，=:只写，+:读写
		:"r0"//需要保存原值的寄存器列表
	};
}

void test_mmu(void)
{
	const char *p="test mmu\n";
	while(*p)
		*(volatile unsigned int *)0xd0000020=*p++;
	//vaddr:0xd0000020==paddr:0x50000020 ，也即串口FIFO的寄存器地址
}
