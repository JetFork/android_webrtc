.text
.align 4

	
ScalePlaneBox_neon:

	stmfd	sp!,	{r4-r12,lr}	
	ldr	r4,	[sp,#40]	@src_stride
	ldr	r5,	[sp,#44]	@dst_stride
	ldr	r6,	[sp,#48]	@0 src row
	ldr	r9,	[sp,#52]	@0 dst row
	sub	r12,	r4,	r0	@(stride - widht) src
	add	r7,	r6,	r4	@1 src row
	add	r8,	r7,	r4	@2 src row
	add	r10,	r9,	r2	@1 dst row
	
	
ScalePlaneBox_height_loop:
	mov	r11,	r2	@width loop
	bl	ScalePlaneBox_width_loop
	
	add	r6,	r8,	r12
	add	r7,	r6,	r4
	add	r8,	r7,	r4
	
	mov	r9,	r10
	add	r10,	r9,	r2
	
	subs	r3,	r3,	#2
	bne	ScalePlaneBox_height_loop
	
	ldmfd	sp!,	{r4-r12,pc}
	
ScalePlaneBox_width_loop:
	vld3.u8	{d0,d1,d2},	[r6]!
	vld3.u8	{d4,d5,d6},	[r7]!
	vld3.u8	{d8,d9,d10},	[r6]!
	vld3.u8	{d12,d13,d14},	[r7]!
	
	vld3.u8	{d16,d18,d20},	[r8]!
	vld3.u8	{d17,d19,d21},	[r8]!
	
	
	@计算第一行
	vaddl.u8	q0,	d0,	d1
	vaddl.u8	q2,	d4,	d5
	vaddl.u8	q4,	d8,	d9
	vaddl.u8	q6,	d12,	d13
	vaddl.u8	q1,	d2,	d6	@1,3,5,7,9,11,13,15
	vaddl.u8	q3,	d10,	d14	@17,19,21,23,25,27,29,31
		
	vadd.u16	q0,	q2	@0,2,4,6,8,10,12,14
	vadd.u16	q4,	q6	@16,18,20,22,24,26,28,30
				
	vqrshrn.u16	d0,	q0,	#2
	vqrshrn.u16	d1,	q4,	#2
	vqrshrn.u16	d2,	q1,	#1
	vqrshrn.u16	d3,	q3,	#1
	
	@计算第二行
	vaddl.u8	q7,	d16,	d18
	vaddl.u8	q8,	d17,	d19
	vqrshrn.u16	d18,	q7,	#1
	vqrshrn.u16	d19,	q8,	#1	
	
	
	@
	vst2.u8	{d0,d2},	[r9]!
	vst2.u8	{d1,d3},	[r9]!
	vst2.u8	{d18,d20},	[r10]!
	vst2.u8	{d19,d21},	[r10]!
	
	subs	r11,	r11,	#32
	bne	ScalePlaneBox_width_loop
	
	mov	pc,	lr
	
	.global ScalePlaneBox_neon
	