
.text
.align 4

downResample_neon:
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
	vld2.u16	{d0,d1,d2,d3},	[r7]!
	vld2.u16	{d4,d5,d6,d7},	[r4]!
	vld2.u16	{d8,d9,d10,d11},	[r7]!
	vld2.u16	{d12,d13,d14,d15},	[r4]!
	
	vld2.u16	{d16,d17,d18,d19},	[r8]!
	vld2.u16	{d20,d21,d22,d23},	[r5]!
	
	vpaddl.u8	q0,	q0
	vpaddl.u8	q1,	q1
	vpaddl.u8	q4,	q4
	vpaddl.u8	q5,	q5
	vpaddl.u8	q8,	q8
	vpaddl.u8	q9,	q9
	
	vpadal.u8	q0,	q2
	vpadal.u8	q1,	q3
	vpadal.u8	q4,	q6
	vpadal.u8	q5,	q7
	vpadal.u8	q8,	q10
	vpadal.u8	q9,	q11
	
	vqrshrn.u16	d0,	q0,	#2
	vqrshrn.u16	d1,	q1,	#2
	vqrshrn.u16	d2,	q4,	#2
	vqrshrn.u16	d3,	q5,	#2
	vqrshrn.u16	d4,	q8,	#2
	vqrshrn.u16	d5,	q9,	#2
	
	
	vld2.u16	{d6,d7,d8,d9},	[r9]!
	vld2.u16	{d10,d11,d12,d13},	[r6]!
	vld1.u16	{d14,d15},	[r7]!
	vld1.u16	{d16,d17},	[r4]!
	vld1.u8	{d18},	[r8]!
	vld1.u8	{d20},	[r5]!
	vld1.u8	{d19},	[r9]!
	vld1.u8	{d21},	[r6]!
	
	vpaddl.u8	q3,	q3
	vpaddl.u8	q4,	q4
	vpaddl.u8	q7,	q7
	vpaddl.u8	q9,	q9
	
	vpadal.u8	q3,	q5
	vpadal.u8	q4,	q6
	vpadal.u8	q7,	q8
	vpadal.u8	q9,	q10
	
	vqrshrn.u16	d6,	q3,	#2
	vqrshrn.u16	d7,	q4,	#2
	vqrshrn.u16	d8,	q7,	#2
	vqrshrn.u16	d9,	q9,	#2
	
	vst2.u8	{d0,d1},	[r10]!
	vst2.u8	{d2,d3},	[r10]!
	vst2.u8	{d4,d5},	[r11]!
	vst2.u8	{d6,d7},	[r12]!
	vst1.u8	{d8},	[r10]!
	vst1.u32	{d9[0]},	[r11]!
	vst1.u32	{d9[1]},	[r12]!
	
	subs	r1,	r1,	#80
	bne	downsample_width_loop1
	
	mov	pc,	lr
	
downsample_width_loop2:
	vld2.u16	{d0,d1,d2,d3},	[r7]!
	vld2.u16	{d4,d5,d6,d7},	[r4]!
	vld2.u16	{d8,d9,d10,d11},	[r7]!
	vld2.u16	{d12,d13,d14,d15},	[r4]!
	vld1.u16	{d16,d17},	[r7]!
	vld1.u16	{d18,d19},	[r4]!
	
	vpaddl.u8	q0,	q0
	vpaddl.u8	q1,	q1
	vpaddl.u8	q4,	q4
	vpaddl.u8	q5,	q5
	
		vpaddl.u8	q8,	q8
	
	vpadal.u8	q0,	q2
	vpadal.u8	q1,	q3
	vpadal.u8	q4,	q6
	vpadal.u8	q5,	q7
	
		vpadal.u8	q8,	q9
	
	vqrshrn.u16	d0,	q0,	#2
	vqrshrn.u16	d1,	q1,	#2
	vqrshrn.u16	d2,	q4,	#2
	vqrshrn.u16	d3,	q5,	#2
	
		vqrshrn.u16	d4,	q8,	#2
		
	vst2.u8	{d0,d1},	[r10]!
	vst2.u8	{d2,d3},	[r10]!
	
		vst1.u8	{d4},	[r10]!
		
	subs	r1,	r1,	#80
	bne	downsample_width_loop2
	
	mov	pc,	lr
.global downResample_neon
