//这是通用双向链表list_head的基本操作函数，包括链表结构的定义，初始化，增加，删除等。
//之后又定义了通用宏list_entry，它能依据list_head结构体的地址ptr,倒求出任意包含list_head结构体
//的结构体的地址。

//通用双向结构体的定义
struct list_head {
	struct list_head *prev, *next;
};

/*
 * 包含了通用双向链表的内存页对应的结构体page的定义
struct page {
	unsigned int vaddr;
	unsigned int flags;
	int order;
	struct kmem_cache *cachep;
	struct list_head list;
};
*/

//初始化结构体list_head的头指针
static inline void INIT_LIST_HEAD(struct list_head *list){
	list->prev=list;
	list->next=list;
}

//将新的结构体指针节点new_lst插入到链表中的prev和next节点之间
static inline void __list_add(struct list_head *new_lst,struct list_head *prev,struct list_head *next){
	new_lst->next=next;
	next->prev=new_lst;
	prev->next=new_lst;
	new_lst->prev=prev;
}

//将节点new_lst插入到链表头后面
static inline void list_add(struct list_head *new_lst,struct list_head *head)
{
	__list_add(new_lst,head,head->next);
}


//将新节点插入到链尾
static inline void list_add_tail(struct list_head *new_lst,struct list_head *head)
{
	__list_add(new_lst,head->prev,head);
}


//删除位于prev和next中间的节点
static inline void __list_del(struct list_head *prev,struct list_head *next)
{
	prev->next=next;
	next->prev=prev;
}


//删除节点entry
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev,entry->next);
}


//删除链表中包含ch到ct间的所有节点
static inline void list_remove_chain(struct list_head *ch,struct list_head *ct)
{
	ch->prev->next=ct->next;
	ct->next->prev=ch-prev;
}


//将节点链ch->ct插入到表头
static inline void list_add_chain(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
	ct-next=head->next;
	head->next->prev=ct;
	head->next=ch;
	ch->prev=head;
}

//将节点链插入到链表尾
static inline void list_add_chain_tail(struct list_head *ch,struct list_head *ct,struct list_head *head)
{
	ch->prev=head->prev;
	head->prev->next=ch;
	ct-next=head;
	head->prev=ct;
}

//判定链表是否为空，是则返回1
static inline int list_empty(struct list_head head)
{
	return head==head->next;
}

//member在结构体type中距结构体地址的偏移量
#define offsetof(TYPE,MEMBER) ((unsigned int)&((TYPE *)0)->MEMBER)

//依据ptr返回其所在结构体type的地址
#define container_of(ptr,type,member) ({	\
		const typeof( ((type *)0)->member ) *__mptr=(ptr);\
		(type *)( (char *)__mptr - offsetof(type,member) );})
//同上
#define list_entry(ptr,type,member) container_of(ptr,type,member)

//遍历链表head
#define list_for_each(pos,head) for(pos=(head)-next;pos!=(head);pos=pos->next)



