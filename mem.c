#define PAGE_AVAILABLE		0x00	//page free
#define PAGE_DIRTY		0x01	//page busy
#define PAGE_PROTECT		0x02	//access deny
#define PAGE_BUDDY_BUSY		0x04
#define PAGE_IN_CACHE		0x08

#define MAX_BUDDY_PAGE_NUM	(9)	//from 2^0 to 2^8,we set 9 different types of buddy

#define AVERAGE_PAGE_NUM_PER_BUDDY	(KERNEL_PAGE_NUM/MAX_BUDDY_PAGE_NUM)
#define PAGE_NUM_FOR_MAX_BUDDY		((1<<MAX_BUDDY_PAGE_NUM)-1)	

#define BUDDY_END(x,order)		((x)+(1<<(order))-1)
#define NEXT_BUDDY_START(x,order)	((x)+(1<<(order)))
#define PREV_BUDDY_START(x,order)	((x)-(1<<(order)))

//slab
#define MEM_CACHE_DEFAULT_ORDER		(0)
#define MEM_CACHE_MAX_ORDER		(5)
#define MEM_CACHE_SAVE_RATE		(0x5a)
#define MEM_CACHE_PERCENT		(0x64)
#define MEM_CACHE_MAX_WAST (PAGE_SIZE-MEM_CACHE_SAVE_RATE*PAGE_SIZE/MEM_CACHE_PERCENT)

//kmalloc
#define KMALLOC_BIAS_SHIFT		(5)	//step is 32byte
#define KMALLOC_MAX_SIZE		(4096)	//
#define KMALLOC_MINIMAL_SIZE_BIAS	(1<<KMALLOC_BIAS_SHIFT)
#define KMALLOC_CACHE_SIZE		(KMALLOC_MAX_SIZE/KMALLOC_MINIMAL_SIZE_BIAS)
struct kmem_cache kmalloc_cache[KMALLOC_CACHE_SIZE]={{0,0,0,0,NULL,NULL,NULL},};
#define kmalloc_cache_size_to_index(size)	((((size))>>(KMALLOC_BIAS_SHIFT)))


struct list_head page_buddy[MAX_BUDDY_PAGE_NUM];//list_head for each buddy stored in page_buddy[]

void init_page_buddy(void)
{
	for(i=0;i<MAX_BUDDY_PAGE_NUM;i++){
		INIT_LIST_HEAD(&page_buddy[i]);
	}
}

void init_page_map(void)
{
	int i;
	struct page *pg=(struct page *)KERNEL_PAGE_START;//addr for first page struct
	init_page_buddy();
	for(i=0;i<(KERNEL_PAGE_NUM);i++,pg++){	
		pg->vaddr=KERNEL_PAGING_START+i*(PAGE_SIZE);//eacn page's addr
		pg->flags=PAGE_AVAILABLE;
		INIT_LIST_HEAD(&(pg->list));
		//the mem first be devided into some bigest buddies
		if(i<(KERNEL_PAGE_NUM&(~PAGE_NUM_FOR_MAX_BUDDY))){
			//find each buddy's addr
			if((i&PAGE_NUM_FOR_MAX_BUDDY)==0){
				pg->order=PAGE_NUM_FOR_MAX_BUDDY-1;//a_buddy_head
			}
			else{
				pg->order=-1;//list_member
			}
			list_add_tail(&(pg->list),&(page_buddy[KERNEL_PAGE_NUM-1]));//added intolink
		}
		else{	//the rest mem which smaller then bigest
		     	//buddy were devided into smallest buddy
			pg->order=0;
			list_add_tail(&(pg->list),&page_buddy[0]);
		}
	}
}

//allocate buddy for especial order
struct page *get_pages_from_list(int order)
{
	unsigned int vaddr;
	int neworder=order;
	struct page *pg,*ret;
	struct list_head *tlst,*tlst1;
	//find buddy which is free
	for(;neworder<=MAX_BUDDY_PAGE_NUM;neworder++){
		if(list_empty(&page_buddy[neworder])){//find  free buddy which equ or big than ordered
			continue;
		}else{
			pg=list_entry(&page_buddy[neworder].next,struct page,list);//addr of page
			tlst=&(BUDDY_END(pg,neworder)->list);//last list of the buddy
			//fetch the buddy from the link
			tlst->next->prev=&page_buddy[neworder];
			page_buddy[neworder].next=tlst->next;
			goto OUT_OK;//divide the big budddy into some small buddy
		}
	}
	return NULL;

OUT_OK:
	for(neworder--;neworder<=order;neworder--){
		tlst1=&(BUDDY_END(pg,neworder)->list);
		tlst=&(pg->list);
		pg=NEXT_BUDDY_START(pg,neworder);
		list_entry(tlst,struct page,list)->order=neworder;
		list_add_chain_tail(tlst,tlst1,&page_buddy[neworder]);
	}
	pg->flags|=PAGE_BUDDY_BUSY;
	pg->order=order;
	return pg;//the allocated buddy
}



//free buddy
void put_pages_to_list(struct page * pg,int order)
{
	struct page *tprev,*tnext;
	if(!(pg->flags&PAGE_BUDDY_BUSY)) return;//检查buddy是否需要释放
	pg->flags&=~(PAGE_BUDDY_BUSY);//标志位清零
	//如果可能，逐次向上合并
	for(;order<MAX_BUDDY_PAGE_NUM;order++){
		tprev=PREV_BUDDY_START(pg,order);
		tnext=NEXT_BUDDY_START(pg,order);
		//与被释放buddy相邻的buddy如果是未被使用的，则合并之
		if((!(tnext->flags&PAGE_BUDDY_BUSY))&&(tnext->order==order)){
			pg->order++;//二者合并成上一级更大buddy
			tnext->order=-1;
			//将相邻buddy从原级链中去除
			list_remove_chain(&(tnext->list),&(BUDDY_END(tnext,order)->list));
			//合并两个buddy到上级buddy链中
			BUDDY_END(pg,order)->list.next=&(tnext->list);
			tnext->list.prev=&(BUDDY_END(pg,order)->list);
			continue;
		}else if((!(tprev->flags&PAGE_BUDDY_BUSY))&&(tprev->order==order)){
			pg->order=-1;
			list_remove_chain(&(pg->list),&(BUDDY_END(pg,order)->list));
			BUDDY_END(tprev,order)->list.next=&(pg->list);
			pg->list.prev=&(BUDDY_END(tprev,order)->list);
			pg=tprev;
			pg->order++;
			continue;
		}else{
			break;//没有可合并的了则跳出循环
		}
	}
	list_add_chain(&(pg->list),&(BUDDY_END(pg,order)->list),&page_buddy[order]);
}

//输入虚拟地址，得到该地址对应页地址
struct page *virt_to_page(unsigned int addr)
{
	unsigned int i;
	i=(addr-KERNEL_PAGING_START)>>PAGE_SHIFT;//该地址对应空间的第i个页
	if(i>KERNEL_PAGE_NUM) return NULL;//超出可分配空间
	return (struct page *)KERNEL_PAGE_START+i;//返回对应页结构的地址
}

//根据页结构地址得到虚拟地址
void *page_address(struct page *pg)
{
	return (void *)(pg->vaddr);
}

//分配order大小的buddy,返回标记过得页结构地址
struct page *alloc_pages(unsigned int flag,int order)
{
	struct page *pg;
	int i;
	pg=get_pages_from_list(order);
	if(!pg) return NULL;
	for(i=0;i<(1<<order);i++){
		(pg+i)->flags|=PAGE_DIRTY;
	}
	return pg;
}

//释放buddy,且清楚了标记
void free_pages(struct page *pg,int order)
{
	int i;
	for(i=0;i<(1<<order);i++){
		(pg+i)->flags&=~PAGE_DIRTY;
	}
	put_pages_to_list(pg,order);
}

//分配buddy大小的内存空间，并返回其虚拟地址
void *get_free_pages(unsigned int flag,int order)
{
	struct page *page;
	page=alloc_pages(flag,order);
	if(!page) return NULL;
	return page_address(page);
}

//根据虚拟地址，释放其buddy空间
void put_free_pages(void *addr,int order)
{
	free_pages(virt_to_page((unsigned int)addr),order);
}

/**********************Slab***********************/
//many buddys which have the same order consist of a slab,we use a kmem_cache to stand for
struct kmem_cache{
	unsigned int obj_size;		//what's the size of the object 
	unsigned int obj_nr;		//how many  objects there are in a slab
	unsigned int page_order;	//the order of the buddy for a object
	unsigned int flags;		//flags for some perpose
	struct page *head_page;		//the slab's first page
	struct page *end_page;		//the slab's last page
	void *nf_block;			//the next free buddy's address
};

//usage:mem_cache_creat--> mem_cache_alloc--> mem_cache_free--> mem_cache_destroy
int find_right_order(unsigned int size)
{
	int order;
	for(order=0;order<=KMEM_CACHE_MAX_ORDER;order++){
		if(size<=KMEM_CACHE_MAX_WAST*(1<<order))
			return order;
	}
	if(size>(1<<order))
		return -1;
	return order;
}

//整理产生空闲buddy链，并返回空闲buddy的个数
int kmem_cache_line_object(void *head,unsigned int size,int order)
{
	void **pl;
	char *p;
	pl=(void **)head;
	p=(char *)head+size;
	int i,s=PAGE_SIZE*(1<<order);
	for(i=0;s>size;i++,s-=size){
		*pl=(void *)p;
		pl=(void **)p;
		p+=size;
	}
	if(s==size)
		i++;
	return i;
}

//新建一个slab的cache，它由若干个buddy组成(新建时为1个)，每个buddy的大小需满足size大小的内存供给
struct kmem_cache *kmem_cache_creat(struct kmem_cache *cache,unsigned int size,unsigned int flags)
{
	void **nf_blk=&(cache->nf_block);
	int order=find_right_order(size);
	if(order==-1) return NULL;
	if((cache->head_page=alloc_pages(0,order))==NULL) return NULL;
	*nf_blk=page_address(cache->head_page);

	cache->obj_nr=kmem_cache_line_object(*nf_blk,size,order);
	cache->obj_size=size;
	cache->obj_order=order;
	cache->flags=flags;
	cache->end_page=BUDDY_END(cache->head_page,order);
	cache->end_page->list.next=NULL;

	return cache;
}

//cache中若有空闲buddy,则直接分配。若无，需向,系统申请后再分配
void *kmem_cache_alloc(struct kmem_cache *cache,unsigned int flag)
{
	void *p;
	struct page *pg;
	if(cache==NULL) return NULL;
	void **nf_block=&(cache->nf_block);
	unsigned int *nr=&(cache->obj_nr);
	int order=cache->page_order;

	if(!*nr){//无空闲buddy,需向系统申请一个,并将其加入到cache的buddy链中
		if((pg=alloc_pages(0,order))==NULL) return NULL;
		*nf_block=page_address(pg);
		cache_end_page->list.next=&(pg->list);
		cache_end_page=BUDDY_END(pg,order);
		cache_end_page->list.next=NULL;
		*nr+=kmem_cache_line_object(*nf_block,cache->obj_size,orded);
	}
	//有空闲buddy,直接分配并更新空闲buddy链
	(*nr)--;
	p=*nf_block;
	*nf_block=*(void **)p;
	pg=virt_to_page((unsigned int)p);
	pg->cachep=cache;
	return p;
}

//此处的释放仅是将buddy标记为空闲，并加入空闲buddy序列。本身还在slab的cache中，并不释放到内存中
void kmem_cache_free(struct kmem_cache *cache,void *objp)
{
	*(void **)objp=cache->nf_block;//新释放空间加入到空闲buddy链的表头
	cache->nf_block=objp;
	cache->obj_nr++;
}


//销毁某个slab cache,将其返还给内存
void kmem_cache_destroy(struct kmem_cache *cache)
{
	int order=cache->page_order;
	struct page *pg=cache->head_page;
	struct list_head *list;
	while(1){
		list=BUDDY_END(pg,order)->list.next;
		free_page(pg,order);//释放第一个buddy
		if(list)//如果还有后续buddy,则继续释放
			pg=list_entry(list,struct page,list);
		else 
			return ;
	}
}



/**********************kmalloc***********************/
//kmalloc is to define some size of slab caches which is used offen

int kmalloc_init(void)
{
	int i;
	for(i=0;i<KMALLOC_CACHE_SIZE;i++){
		if(kmem_cache_creat(&kmalloc_cache[i],(i+1)*KMALLOC_MINIMAL_SIZE_BIAS,0)==NULL) 
			return -1;
	}
	return 0;
}


void *kmalloc(unsigned int size)
{
	int index=kmalloc_cache_size_to_index(size);
	if(index>KMALLOC_CACHE_SIZE) return NULL;
	return kmem_cache_alloc(&kmalloc_cache[index],0);
}

void kfree(void *addr)
{
	struct page *pg;
	pg=virt_to_page(unsigned int addr);
	kmem_cache_free(pg->cachep,addr);
}




