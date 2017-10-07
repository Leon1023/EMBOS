//系统设计时，虚拟开发板物理内存地址范围是0x30000000-0x30800000，共8M
//因为内存开始要用于存放内核运行时的代码，内存最后1M用于各模式的堆栈和页表数据

#define _MEM_END		0x30700000	//0x30700001-0x30800000堆栈和页表空间
#define _MEM_START		0x300f0000	//0x30000000-0x300effff操作系统自身代码

#define PAGE_SHIFT		(12)
#define PAGE_SIZE		(1<<PAGE_SHIFT)	//页的大小为4K
#define PAGE_MASK		(~(PAGE_SIZE-1))//地址页对齐时用的掩码

#define KERNEL_MEM_END		(_MEM_END)	//useble ram结束地址
#define KERNEL_PAGING_START	((_MEM_START+(~PAGE_MASK))&(PAGE_MASK)) //页空间的开始地址
#define KERNEL_PAGING_END	(KERNEL_PAGING_START+PAGE_SIZE*\
				((KERNEL_MEM_END-KERNEL_PAGING_START)/\
				(PAGE_SIZE+sizeof(struct page))))	//页空间的结束地址
#define KERNEL_PAGE_NUM		((KERNEL_PAGING_END-KERNEL_PAGING_START)/PAGE_SIZE)//页的个数
#define KERNEL_PAGE_END		_MEM_END //页结构体空间结束地址
#define KERNEL_PAGE_START	(KERNEL_PAGE_END-KERNEL_PAGE_NUM*sizeof(struct page))//页结构体空间开始地址


