#include "intr.h"
	.h8300h
	.section .text

	.global	_intr_softerr
#	.type	_intr_softerr,@function
_intr_softerr:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.w	#SOFTVEC_TYPE_SOFTERR,r0
	jsr	@_Interrupt
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_syscall
#	.type	_intr_syscall,@function
_intr_syscall:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.w	#SOFTVEC_TYPE_SYSCALL,r0
	jsr	@_Interrupt
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_sci1recv
#	.type	_intr_sci1recv,@function
_intr_sci1recv:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.w	#SOFTVEC_TYPE_SCI1RECV,r0
	jsr	@_Interrupt
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_timer0
#	.type	_intr_timer0,@function
_intr_timer0:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.w	#SOFTVEC_TYPE_TIMER0,r0
	jsr	@_Interrupt
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

	.global	_intr_timer2
#	.type	_intr_timer2,@function
_intr_timer2:
	mov.l	er6,@-er7
	mov.l	er5,@-er7
	mov.l	er4,@-er7
	mov.l	er3,@-er7
	mov.l	er2,@-er7
	mov.l	er1,@-er7
	mov.l	er0,@-er7
	mov.l	er7,er1
	mov.w	#SOFTVEC_TYPE_TIMER2,r0
	jsr	@_Interrupt
	mov.l	@er7+,er0
	mov.l	@er7+,er1
	mov.l	@er7+,er2
	mov.l	@er7+,er3
	mov.l	@er7+,er4
	mov.l	@er7+,er5
	mov.l	@er7+,er6
	rte

