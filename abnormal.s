.equ DISABLE_IRQ,0x80	//cpsr_I:bits(7)
.equ DISABLE_FIQ,0x40	//cpsr_I:bits(6)
.equ SYS_MOD,0x1f	//cpsr_I:bits(4-0)
.equ IRQ_MOD,0x12
.equ FIQ_MOD,0x11
.equ SVC_MOD,0x13
.equ ABT_MOD,0x17
.equ UND_MOD,0x1b
.equ MOD_MASK,0x1f

//汇编下的宏定义，.macro及.endm是关键字，两者之间的所有语句是宏体，CHANGE_TO_SVC是宏名，
.macro CHANGE_TO_SVC 
	msr cpsr_c,#(SVC_MOD|DISABLE_IRQ|DISABLE_FIQ)
.endm

//宏名后可以跟参数，参数在语句里的使用通过斜杠表示(\参数)
.macro CHANGE_TO_IRQ 
	msr cpsr_c,#(SVC_MOD|DISABLE_IRQ|DISABLE_IRQ)
.endm

.global __vector_undefined
.global __vector_swi
.global __vector_prefetch_abort
.global __vector_data_abort
.global __vector_reserved
.global __vector_irq
.global __vector_fiq

.text
.code 32
__vector_undefined:
	nop
__vector_swi:
	nop
__vector_prefetch_abort:
	nop
__vector_data_abort:
	nop
__vector_reserved:
	nop
__vector_irq:		@为了使能中断嵌套，我们把中断处理函数放在管理模式下运行
	sub r14,r14,#4	@因为，即使在中断函数处理过程中又发生新的中断，此时仅仅是跳转到irq模式
	str r14,[r13,#-0x4]	@而不影响正在处理原中断函数模式下的寄存器
	mrs r14,spsr
	str r14,[r13,#-0x8]
	str r0,[r13,#-0xc]
	mov r0,r13
	CHANGE_TO_SVC
	str r14,[r13,#-0x8]!
	ldr r14,[r0,#-0x4]
	str r14,[r13,#-0x4]!
	ldr r14,[r0,#-0x8]
	ldr r0,[r0,#-0xc]
	stmdb r13!,{r0-r3,r14}
	bl common_irq_handler()
	ldmia r13!,{r0-r3,r14}
	msr spsr,r14		@将保存的被中断的运行状态保存至管理模式的spsr中
	ldmfd r13!,{pc}^	@从管理模式直接返回到被中断前的运行状态

__vector_fiq:
	nop
