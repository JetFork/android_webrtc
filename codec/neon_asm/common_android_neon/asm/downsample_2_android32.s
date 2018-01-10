
.text
.align 4

downResample_9_16neon:
	stmfd	sp!,	{r4-r12,lr}
	ldr	r7,	[r0]
	ldr	r8,	[r0,#4]
	ldr	r9,	[r0,#8]
	ldr	r10,	[r1]
	ldr	r11,	[r1,#4]
	ldr	r12,	[r1,#8]
	add	r4,	r7,	r2
	add	r5,	r8,	r2,	lsr	#1
	add	r6,	r9,	r2,	lsr	#1
	mov	r0,	r3,	lsr	#1
	
	
downsample_height_loop1:
	mov	r1,	r2	@width loop
	bl	downsample_width_loop1
	
	mov	r7,	r4
	add	r4,	r4,	r2
	mov	r8,	r5
	add	r5,	r5,	r2,	lsr	#1
	mov	r9,	r6
	add	r6,	r6,	r2,	lsr	#1
	
	subs	r3,	r3,	#4
	bne	downsample_height_loop1
	
downsample_height_loop2:
	mov	r1,	r2
	bl	downsample_width_loop2
	mov	r7,	r4
	add	r4,	r4,	r2
	subs	r0,	r0,	#2
	bne	downsample_height_loop2
	
	ldmfd	sp!,	{r4-r12,pc}
	
downsample_width_loop1:
	vld1.u16	{d0,d1,d2,d3},	[r7]!
	vld1.u16	{d4,d5,d6,d7},	[r4]!   @Y32 
	vld1.u16	{d8,d9,d10},	[r7]!	
	vld1.u16	{d12,d13,d14},	[r4]!	 @Y24
	vld1.u32	{d11[0]},	[r7]!  @Y 4
	vld1.u32	{d15[0]},	[r4]!  @Y 4
	
	vld1.u16	{d16,d17,d18},	[r8]!	
	vld1.u16	{d20,d21,d22},	[r5]!	 @U24
	vld1.u32	{d19[0]},	[r8]!  @U 4
	vld1.u32	{d23[0]},	[r5]!  @U 4
	vld1.u16	{d19[2]},	[r8]!	@U2
	vld1.u16	{d23[2]},	[r5]!	@U2
	
	vld1.u16	{d24,d25,d26},	[r9]!	
	vld1.u16	{d28,d29,d30},	[r6]!	 @V24
	vld1.u32	{d27[0]},	[r9]!  @V 4
	vld1.u32	{d31[0]},	[r6]!  @V 4
	vld1.u16	{d27[2]},	[r9]!	@V2
	vld1.u16	{d31[2]},	[r6]!	@V2

	
	vpaddl.u8	q0,	q0
	vpaddl.u8	q1,	q1
	vpaddl.u8	q4,	q4
	vpaddl.u8	q5,	q5
	
	vpaddl.u8	q8,	q8	
	vpaddl.u8	q9,	q9
	vpaddl.u8	q12,	q12	
	vpaddl.u8	q13,	q13
	
	vpadal.u8	q0,	q2
	vpadal.u8	q1,	q3
	vpadal.u8	q4,	q6
	vpadal.u8	q5,	q7
	
	vpadal.u8	q8,	q10
	vpadal.u8	q9,	q11
	vpadal.u8	q12,	q14
	vpadal.u8	q13,	q15
	
	vqrshrn.u16	d0,	q0,	#2
	vqrshrn.u16	d1,	q1,	#2
	vqrshrn.u16	d2,	q4,	#2
	vqrshrn.u16	d3,	q5,	#2
	
	vqrshrn.u16	d4,	q8,	#2
	vqrshrn.u16	d5,	q9,	#2
	vqrshrn.u16	d6,	q12,	#2
	vqrshrn.u16	d7,	q13,	#2

	
	
	vst1.u8	{d0,d1,d2},	[r10]!   @Y24
	vst1.u32	{d3[0]},	[r10]!  @Y4
	vst1.u16	{d3[2]},	[r10]!	@Y2
	
	vst1.u8	{d4},	[r11]!   
	vst1.u32	{d5[0]},	[r11]!  @U4
	vst1.u16	{d5[2]},	[r11]!  @U2
	vst1.u8		{d5[6]},	[r11]!	@U1

	vst1.u8	{d6},	[r12]!		
	vst1.u32	{d7[0]},	[r12]!  @V4
	vst1.u16	{d7[2]},	[r12]!  @V2
	vst1.u8		{d7[6]},	[r12]!	@V1
	
	subs	r1,	r1,	#60
	bne	downsample_width_loop1
	
	mov	pc,	lr
	
downsample_width_loop2:
	vld1.u16	{d0,d1,d2,d3},	[r7]!
	vld1.u16	{d4,d5,d6,d7},	[r4]!   @Y32 
	vld1.u16	{d8,d9,d10},	[r7]!	
	vld1.u16	{d12,d13,d14},	[r4]!	 @Y24
	vld1.u32	{d11[0]},	[r7]!  @Y 4
	vld1.u32	{d15[0]},	[r4]!  @Y 4
	
	vpaddl.u8	q0,	q0
	vpaddl.u8	q1,	q1
	vpaddl.u8	q4,	q4
	vpaddl.u8	q5,	q5
	
	vpadal.u8	q0,	q2
	vpadal.u8	q1,	q3
	vpadal.u8	q4,	q6
	vpadal.u8	q5,	q7
	
	vqrshrn.u16	d0,	q0,	#2
	vqrshrn.u16	d1,	q1,	#2
	vqrshrn.u16	d2,	q4,	#2
	vqrshrn.u16	d3,	q5,	#2
	
	vst1.u8	{d0,d1,d2},	[r10]!   @Y24
	vst1.u32	{d3[0]},	[r10]!  @Y4
	vst1.u16	{d3[2]},	[r10]!	@Y2
		
	subs	r1,	r1,	#60
	bne	downsample_width_loop2
	
	mov	pc,	lr
.global downResample_9_16neon
