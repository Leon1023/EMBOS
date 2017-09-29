//一个基本的可变参数的宏定义及利用它编写的可变参数的输出测试函数
typedef char * va_list;

//该宏可求出:在变量n为某类型的情况下，当在满足4字节对齐时，尚需补足的字节数。
#define _INTSIZEOF(n)	((sizeof(n)+sizeof(int)-1)&~(sizeof(int)-1))

//该宏求得:变量v的下一个4字节对齐的地址，并赋给ap
#define va_start(ap,v)	(ap=(va_list)&v+_INTSIZEOF(v))

//该宏求得:以ap为地址的，与t变量同类型的变量值，同时ap自增1个t变量长度
#define va_arg(ap,t)	*((t *)((ap+=_INTSIZEOF(t))-_INTSIZEOF(t)))

//将ap清零
#define va_end		(ap=(va_list)0)

//向串口输出字符形式的num
void test_num(int num)
{
	*(char *)0xd0000020=(num+'0');
}

//依次输出函数的可变参数
void test_vparameter(int i,...)
{
	int c;
	va_list argv;//定义一个字符型指针
	va_start(argv,i);//初始化指针，其值为参数i的下一个可变参数的地址
	while(i--){//依次输出i个可变参数
		c=va_arg(argv,int);//依次取出整形可变参数赋给c
		test_num(c);//输出参数的字符型值到串口
	}
	va_end(argv);//清零
}

