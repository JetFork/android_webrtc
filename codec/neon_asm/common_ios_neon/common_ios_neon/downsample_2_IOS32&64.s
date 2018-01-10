
.text
#if defined(__arm64)
//.align 16
_downResample_9_16neon:
	//x0:char *src[3],
	//x1:char *dst[3],
	//x2:int iw,原始图像宽
	//x3:int ih,原始图像高
	//x4:int ow, 采样后图像宽
	//x5:int oh, 采样后图像高
	//x6:int step，定义为2

	mov	x17,	lr


	ldr	x7,	[x0]	//Y1 adress
	ldr	x8,	[x0,#8]	//U1 adress
	ldr	x9,	[x0,#16]	//v1 adress
	ldr	x13,	[x1]	//dest Y adress
	ldr	x14,	[x1,#8]	//dest U adress
	ldr	x15,	[x1,#16]	//dest v adress
	add	x10,	x7,	x2	//Y2 adress
	add	x11,	x8,	x2,	asr	#1	//U2 adress
	add	x12,	x9,	x2,	asr	#1	//v2 adress
    mov x0, x3
	movi	v31.8h,	#2

height_loop1:
	mov	x16,	x2
	bl	width_loop1

	mov	x7,	x10
	add	x10,	x10,	x2
	
	mov	x8,	x11
	add	x11,	x11,	x2,	asr	#1

	mov	x9,	x12
	add	x12,	x12,	x2,	asr	#1

	subs	x3,	x3,	#4
	b.ne	height_loop1
//第二次循环计算剩下的一半Y	
height_loop2:
	mov	x16,	x2
	bl	width_loop2
	mov	x7,	x10
	add	x10,	x10,	x2
	subs	x0,	x0,	#4
	b.ne	height_loop2

	//RET
	mov	lr,	x17

	ret


width_loop1:
	ld1	{v0.8h,v1.8h,v2.8h},	[x7],	#48		//	load Y
	ld1	{v4.8h,v5.8h,v6.8h},	[x10],	#48
	ld1	{v8.d}[0],	[x7],	#8
	ld1	{v9.d}[0],	[x10],	#8
	ld1	{v8.s}[2],	[x7],	#4
	ld1	{v9.s}[2],	[x10],	#4
	
	
	ld1	{v10.16b},	[x8],	#16				//load 	U
	ld1	{v12.16b},	[x11],	#16
	ld1	{v11.d}[0],	[x8],	#8
	ld1	{v13.d}[0],	[x11],	#8
	ld1	{v11.s}[2],	[x8],	#4
	ld1	{v13.s}[2],	[x11],	#4
	ld1 {v11.h}[6], [x8], #2
	ld1	{v13.h}[6],	[x11],	#2


	ld1	{v16.16b},	[x9],	#16				//load 	V
	ld1	{v18.16b},	[x12],	#16
	ld1	{v17.d}[0],	[x9],	#8
	ld1	{v19.d}[0],	[x12],	#8
	ld1	{v17.s}[2],	[x9],	#4
	ld1	{v19.s}[2],	[x12],	#4
	ld1 {v17.h}[6], [x9], #2
	ld1	{v19.h}[6],	[x12],	#2

	uaddlp	v0.8h,	v0.16b		//同行相邻元素相加 Y
	uaddlp	v1.8h,	v1.16b
	uaddlp	v2.8h,	v2.16b
	uaddlp	v8.8h,	v8.16b

	uaddlp	v10.8h,	v10.16b		//同行相邻元素相加 U
	uaddlp	v11.8h,	v11.16b

	uaddlp	v16.8h,	v16.16b		//同行相邻元素相加 v
	uaddlp	v17.8h,	v17.16b

	uadalp	v0.8h,	v4.16b		//同行相加累加到上一行的同行相加结果上 Y
	uadalp	v1.8h,	v5.16b
	uadalp	v2.8h,	v6.16b
	uadalp	v8.8h,	v9.16b

	uadalp	v10.8h,	v12.16b		//同行相加累加到上一行的同行相加结果上 U
	uadalp	v11.8h,	v13.16b

	uadalp	v16.8h,	v18.16b
	uadalp	v17.8h,	v19.16b		//同行相加累加到上一行的同行相加结果上 v

	add	v4.8h,	v0.8h,	v31.8h		//加过加2	Y
	add	v5.8h,	v1.8h,	v31.8h
	add	v6.8h,	v2.8h,	v31.8h
	add	v9.8h,	v8.8h,	v31.8h

	add	v12.8h,	v10.8h,	v31.8h		//加过加2 U
	add	v13.8h,	v11.8h,	v31.8h

	add	v18.8h,	v16.8h,	v31.8h		//加过加2 v
	add	v19.8h,	v17.8h,	v31.8h

	sqshrun	v0.8b,	v4.8h,	#2	//右移饱和 Y
	sqshrun	v1.8b,	v5.8h,	#2
	sqshrun	v2.8b,	v6.8h,	#2
	sqshrun	v8.8b,	v9.8h,	#2

	sqshrun	v10.8b,	v12.8h,	#2
	sqshrun	v11.8b,	v13.8h,	#2
	sqshrun	v16.8b,	v18.8h,	#2
	sqshrun	v17.8b,	v19.8h,	#2

	st1	{v0.8b,v1.8b},	[x13],	#16    //Y
	st1	{v2.8b},	[x13],	#8
	st1	{v8.s}[0],	[x13],	#4
	st1	{v8.h}[2],	[x13],	#2

	st1	{v10.8b},	[x14],	#8     //U
	st1	{v11.s}[0],	[x14],	#4
	st1	{v11.h}[2],	[x14],	#2
	st1	{v11.b}[6],	[x14],	#1

	st1	{v16.8b},	[x15],	#8     //V
	st1	{v17.s}[0],	[x15],	#4
	st1	{v17.h}[2],	[x15],	#2
	st1	{v17.b}[6],	[x15],	#1


	subs	x16,	x16,	#60
	b.ne	width_loop1

	br	lr
	
width_loop2:
	ld1	{v0.8h,v1.8h,v2.8h},	[x7],	#48		//	load Y
	ld1	{v4.8h,v5.8h,v6.8h},	[x10],	#48
	ld1	{v8.d}[0],	[x7],	#8
	ld1	{v9.d}[0],	[x10],	#8
	ld1	{v8.s}[2],	[x7],	#4
	ld1	{v9.s}[2],	[x10],	#4
	
	uaddlp	v0.8h,	v0.16b		//同行相邻元素相加 Y
	uaddlp	v1.8h,	v1.16b
	uaddlp	v2.8h,	v2.16b
	uaddlp	v8.8h,	v8.16b
	
	uadalp	v0.8h,	v4.16b		//同行相加累加到上一行的同行相加结果上 Y
	uadalp	v1.8h,	v5.16b
	uadalp	v2.8h,	v6.16b
	uadalp	v8.8h,	v9.16b
	
	add	v4.8h,	v0.8h,	v31.8h		//加过加2	Y
	add	v5.8h,	v1.8h,	v31.8h
	add	v6.8h,	v2.8h,	v31.8h
	add	v9.8h,	v8.8h,	v31.8h
	
	sqshrun	v0.8b,	v4.8h,	#2	//右移饱和 Y
	sqshrun	v1.8b,	v5.8h,	#2
	sqshrun	v2.8b,	v6.8h,	#2
	sqshrun	v8.8b,	v9.8h,	#2
	
	st1	{v0.8b,v1.8b},	[x13],	#16    //Y
	st1	{v2.8b},	[x13],	#8
	st1	{v8.s}[0],	[x13],	#4
	st1	{v8.h}[2],	[x13],	#2
	
	subs	x16,	x16,	#60
	b.ne	width_loop2

	br	lr

#elif   defined(__arm)

.align 4
@32位汇编
_downResample_9_16neon:
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
	
@首次循环计算
downsample_height_loop1:
	mov	r1,	r2	@width loop
	bl	downsample_width_loop1
	
	mov	r7,	r4		@第一行Y指针偏移
	add	r4,	r4,	r2	@第二行Y指针偏移
	mov	r8,	r5	@第一行U指针偏移
	add	r5,	r5,	r2,	lsr	#1	@第二行U指针偏移
	mov	r9,	r6	@第一行V指针偏移
	add	r6,	r6,	r2,	lsr	#1	@第二行V指针偏移
	
	subs	r3,	r3,	#4	@首次高循环次数递减
	bne	downsample_height_loop1	
	
@第二次循环
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
	
	vst1.u8	{d4},	[r11]!   @U8
	vst1.u32	{d5[0]},	[r11]!  @U4
	vst1.u16	{d5[2]},	[r11]!  @U2
	vst1.u8		{d5[6]},	[r11]!

	vst1.u8	{d6},	[r12]!		@V8
	vst1.u32	{d7[0]},	[r12]!  @V4
	vst1.u16	{d7[2]},	[r12]!  @V2
	vst1.u8		{d7[6]},	[r12]!
	
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
#endif

.global _downResample_9_16neon