
.text
#if defined(__arm64)
//.align 16
_downResample_neon:
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
	ld4	{v0.8h,v1.8h,v2.8h,v3.8h},	[x7],	#64		//	load Y
	ld4	{v4.8h,v5.8h,v6.8h,v7.8h},	[x10],	#64
	ld1	{v8.16b},	[x7],	#16
	ld1	{v9.16b},	[x10],	#16

	ld2	{v10.8h,v11.8h},	[x8],	#32		//load U
	ld2	{v12.8h,v13.8h},	[x11],	#32
	ld1	{v14.d}[0],	[x8],	#8
	ld1	{v15.d}[0],	[x11],	#8

	ld2	{v16.8h,v17.8h},	[x9],	#32		//load v
	ld2	{v18.8h,v19.8h},	[x12],	#32
	ld1	{v14.d}[1],	[x9],	#8
	ld1	{v15.d}[1],	[x12],	#8

	uaddlp	v0.8h,	v0.16b		//同行相邻元素相加 Y
	uaddlp	v1.8h,	v1.16b
	uaddlp	v2.8h,	v2.16b
	uaddlp	v3.8h,	v3.16b
	uaddlp	v8.8h,	v8.16b

	uaddlp	v10.8h,	v10.16b		//同行相邻元素相加 U
	uaddlp	v11.8h,	v11.16b

	uaddlp	v16.8h,	v16.16b		//同行相邻元素相加 v
	uaddlp	v17.8h,	v17.16b
	uaddlp	v14.8h,	v14.16b

	uadalp	v0.8h,	v4.16b		//同行相加累加到上一行的同行相加结果上 Y
	uadalp	v1.8h,	v5.16b
	uadalp	v2.8h,	v6.16b
	uadalp	v3.8h,	v7.16b
	uadalp	v8.8h,	v9.16b

	uadalp	v10.8h,	v12.16b		//同行相加累加到上一行的同行相加结果上 U
	uadalp	v11.8h,	v13.16b

	uadalp	v16.8h,	v18.16b
	uadalp	v17.8h,	v19.16b		//同行相加累加到上一行的同行相加结果上 v
	uadalp	v14.8h,	v15.16b

	add	v4.8h,	v0.8h,	v31.8h		//加过加2	Y
	add	v5.8h,	v1.8h,	v31.8h
	add	v6.8h,	v2.8h,	v31.8h
	add	v7.8h,	v3.8h,	v31.8h
	add	v9.8h,	v8.8h,	v31.8h

	add	v12.8h,	v10.8h,	v31.8h		//加过加2 U
	add	v13.8h,	v11.8h,	v31.8h

	add	v18.8h,	v16.8h,	v31.8h		//加过加2 v
	add	v19.8h,	v17.8h,	v31.8h
	add	v15.8h,	v14.8h,	v31.8h

	sqshrun	v0.8b,	v4.8h,	#2	//右移饱和 Y
	sqshrun	v1.8b,	v5.8h,	#2
	sqshrun	v2.8b,	v6.8h,	#2
	sqshrun	v3.8b,	v7.8h,	#2
	sqshrun	v8.8b,	v9.8h,	#2

	sqshrun	v10.8b,	v12.8h,	#2
	sqshrun	v11.8b,	v13.8h,	#2
	sqshrun	v16.8b,	v18.8h,	#2
	sqshrun	v17.8b,	v19.8h,	#2
	sqshrun	v14.8b,	v15.8h,	#2

	st4	{v0.8b,v1.8b,v2.8b,v3.8b},	[x13],	#32
	st1	{v8.8b},	[x13],	#8

	st2	{v10.8b,v11.8b},	[x14],	#16
	st1	{v14.s}[0],	[x14],	#4

	st2	{v16.8b,v17.8b},	[x15],	#16
	st1	{v14.s}[1],	[x15],	#4


	subs	x16,	x16,	#80
	b.ne	width_loop1

	br	lr
	
width_loop2:
	ld4	{v0.8h,v1.8h,v2.8h,v3.8h},	[x7],	#64		//	load Y
	ld4	{v4.8h,v5.8h,v6.8h,v7.8h},	[x10],	#64
	ld1	{v8.16b},	[x7],	#16
	ld1	{v9.16b},	[x10],	#16
	
	uaddlp	v0.8h,	v0.16b		//同行相邻元素相加 Y
	uaddlp	v1.8h,	v1.16b
	uaddlp	v2.8h,	v2.16b
	uaddlp	v3.8h,	v3.16b
	uaddlp	v8.8h,	v8.16b
	
	uadalp	v0.8h,	v4.16b		//同行相加累加到上一行的同行相加结果上 Y
	uadalp	v1.8h,	v5.16b
	uadalp	v2.8h,	v6.16b
	uadalp	v3.8h,	v7.16b
	uadalp	v8.8h,	v9.16b
	
	add	v4.8h,	v0.8h,	v31.8h		//加过加2	Y
	add	v5.8h,	v1.8h,	v31.8h
	add	v6.8h,	v2.8h,	v31.8h
	add	v7.8h,	v3.8h,	v31.8h
	add	v9.8h,	v8.8h,	v31.8h
	
	sqshrun	v0.8b,	v4.8h,	#2	//右移饱和 Y
	sqshrun	v1.8b,	v5.8h,	#2
	sqshrun	v2.8b,	v6.8h,	#2
	sqshrun	v3.8b,	v7.8h,	#2
	sqshrun	v8.8b,	v9.8h,	#2
	
	st4	{v0.8b,v1.8b,v2.8b,v3.8b},	[x13],	#32
	st1	{v8.8b},	[x13],	#8
	
	subs	x16,	x16,	#80
	b.ne	width_loop2

	br	lr

#elif   defined(__arm)

.align 4
@32位汇编
_downResample_neon:
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
	vld2.u16	{d0,d1,d2,d3},	[r7]!	@装载第一行Y数据 32个
	vld2.u16	{d4,d5,d6,d7},	[r4]!	@装载第二行Y数据
	vld2.u16	{d8,d9,d10,d11},	[r7]!	@继续装载Y数据 32个
	vld2.u16	{d12,d13,d14,d15},	[r4]!
	
	vld2.u16	{d16,d17,d18,d19},	[r8]!	@装载第一行U数据 32个
	vld2.u16	{d20,d21,d22,d23},	[r5]!	@装载第二行U数据
	
	vpaddl.u8	q0,	q0	@第一行相邻两个像素值相加
	vpaddl.u8	q1,	q1
	vpaddl.u8	q4,	q4
	vpaddl.u8	q5,	q5
	vpaddl.u8	q8,	q8
	vpaddl.u8	q9,	q9
	
	vpadal.u8	q0,	q2	@第二行相邻两个像素值相加并累加到第一行相加的结果上
	vpadal.u8	q1,	q3
	vpadal.u8	q4,	q6
	vpadal.u8	q5,	q7
	vpadal.u8	q8,	q10
	vpadal.u8	q9,	q11
	
	vqrshrn.u16	d0,	q0,	#2	@对四个像素的和除以4
	vqrshrn.u16	d1,	q1,	#2
	vqrshrn.u16	d2,	q4,	#2
	vqrshrn.u16	d3,	q5,	#2
	vqrshrn.u16	d4,	q8,	#2
	vqrshrn.u16	d5,	q9,	#2
	
	
	vld2.u16	{d6,d7,d8,d9},	[r9]!	@装载第一行V数据 32个
	vld2.u16	{d10,d11,d12,d13},	[r6]!	@装载第二行V数据
	vld1.u16	{d14,d15},	[r7]!	@装载Y数据 16个
	vld1.u16	{d16,d17},	[r4]!
	vld1.u8	{d18},	[r8]!	@装载U数据 8个
	vld1.u8	{d20},	[r5]!
	vld1.u8	{d19},	[r9]!	@装载V数据8个
	vld1.u8	{d21},	[r6]!
	
	vpaddl.u8	q3,	q3	@第一行相邻两个像素相加
	vpaddl.u8	q4,	q4
	vpaddl.u8	q7,	q7
	vpaddl.u8	q9,	q9
	
	vpadal.u8	q3,	q5	@第二行相邻两个像素相加并累加到第一行的结果上
	vpadal.u8	q4,	q6
	vpadal.u8	q7,	q8
	vpadal.u8	q9,	q10
	
	vqrshrn.u16	d6,	q3,	#2	@对4个像素的相加值除以2
	vqrshrn.u16	d7,	q4,	#2
	vqrshrn.u16	d8,	q7,	#2
	vqrshrn.u16	d9,	q9,	#2
	
	vst2.u8	{d0,d1},	[r10]!	@存储Y数据
	vst2.u8	{d2,d3},	[r10]!
	vst2.u8	{d4,d5},	[r11]!	@存储U数据
	vst2.u8	{d6,d7},	[r12]!	@存储V数据
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
#endif

.global _downResample_neon