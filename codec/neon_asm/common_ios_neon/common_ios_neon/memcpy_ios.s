
.text
#if defined(__arm64)
_memcpy_neon:
		lsr			x3, x2,	#9
		subs		x3,	x3,	#0
		b.eq				cpy.256
cpy.512:
		ld1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x1],	#64
		ld1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x1],	#64
		ld1			{v8.2d, v9.2d, v10.2d, v11.2d}, [x1],	#64
		ld1			{v12.2d, v13.2d, v14.2d, v15.2d}, [x1],	#64
		ld1			{v16.2d, v17.2d, v18.2d, v19.2d}, [x1],	#64
		ld1			{v20.2d, v21.2d, v22.2d, v23.2d}, [x1],	#64
		ld1			{v24.2d, v25.2d, v26.2d, v27.2d}, [x1],	#64
		ld1			{v28.2d, v29.2d, v30.2d, v31.2d}, [x1],	#64

		st1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x0],	#64
		st1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x0],	#64
		st1			{v8.2d, v9.2d, v10.2d, v11.2d}, [x0],	#64
		st1			{v12.2d, v13.2d, v14.2d, v15.2d}, [x0],	#64
		st1			{v16.2d, v17.2d, v18.2d, v19.2d}, [x0],	#64
		st1			{v20.2d, v21.2d, v22.2d, v23.2d}, [x0],	#64
		st1			{v24.2d, v25.2d, v26.2d, v27.2d}, [x0],	#64
		st1			{v28.2d, v29.2d, v30.2d, v31.2d}, [x0],	#64

		subs			x3,	x3, #1
		b.ne				cpy.512
cpy.256:
		ands			x3, x2, #0x100
		b.eq 			cpy.128

		ld1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x1],	#64
		ld1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x1],	#64
		ld1			{v8.2d, v9.2d, v10.2d, v11.2d}, [x1],	#64
		ld1			{v12.2d, v13.2d, v14.2d, v15.2d}, [x1],	#64

		st1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x0],	#64
		st1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x0],	#64
		st1			{v8.2d, v9.2d, v10.2d, v11.2d}, [x0],	#64
		st1			{v12.2d, v13.2d, v14.2d, v15.2d}, [x0],	#64
cpy.128:	
		ands			x3, x2, #0x080
		b.eq 			cpy.64

		ld1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x1],	#64
		ld1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x1],	#64
		
		st1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x0],	#64
		st1			{v4.2d, v5.2d, v6.2d, v7.2d}, [x0],	#64
cpy.64:
		ands			x3, x2, #0x040
		b.eq 			cpy.32

		ld1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x1],	#64
		
		st1			{v0.2d, v1.2d, v2.2d, v3.2d}, [x0],	#64
cpy.32:
		ands			x3,	x2, #0x020
		b.eq 			cpy.16

		ld1			{v0.2d,v1.2d},	[x1],	#32
		
		st1			{v0.2d,v1.2d},	[X0],	#32
cpy.16:
		ands			x3, x2, #0x010
		b.eq 			cpy.8

		ld1			{v0.2d},	[x1],	#16
		
		st1			{v0.2d},	[x0],	#16
cpy.8:
		ands			x3, x2, #0x08
		b.eq 			cpy.4

		ld1			{v0.d}[0],	[x1],	#8
		
		st1			{v0.d}[0],	[x0],	#8
cpy.4:
		ands			x3, x2, #0x04
		b.eq 			cpy.2

		ld1			{v0.s}[0],	[x1],	#4
		
		st1			{v0.s}[0],	[x0],	#4
cpy.2:
		ands			x3, x2, #0x02
		b.eq 			cpy.1

		ld1			{v0.h}[0],	[x1],	#2

		st1			{v0.h}[0],	[x0],	#2
cpy.1:	
		ands			x3, x2, #0x01
		b.eq 			endofcpy

		ld1			{v0.b}[0],	[x1],	#1
		
		st1			{v0.b}[0],	[x0],	#1
		
endofcpy:	
		ret

#elif   defined(__arm)

.align 4

_memcpy_neon:
		stmfd 			sp!, {r4-r12,lr}
		mov				lr, r0
		@---------------------
		@eor				r4, r4, r4
		@---------------------

		movs			r3, r2, lsr #8
		beq				cpy.128				
cpy.256:
		@---------------------
		@add				r4, r4, #256
		@---------------------
		vld1.64			{D0, D1, D2, D3}, [r1]!
		vld1.64			{D4, D5, D6, D7}, [r1]!
		vld1.64			{D8, D9, D10, D11}, [r1]!
		vld1.64			{D12, D13, D14, D15}, [r1]!
		vld1.64			{D16, D17, D18, D19}, [r1]!
		vld1.64			{D20, D21, D22, D23}, [r1]!
		vld1.64			{D24, D25, D26, D27}, [r1]!
		vld1.64			{D28, D29, D30, D31}, [r1]!
		
		vst1.64			{D0, D1, D2, D3}, [lr]!
		vst1.64			{D4, D5, D6, D7}, [lr]!
		vst1.64			{D8, D9, D10, D11}, [lr]!
		vst1.64			{D12, D13, D14, D15}, [lr]!
		vst1.64			{D16, D17, D18, D19}, [lr]!
		vst1.64			{D20, D21, D22, D23}, [lr]!
		vst1.64			{D24, D25, D26, D27}, [lr]!
		vst1.64			{D28, D29, D30, D31}, [lr]!

		subs			r3, #1
		bne				cpy.256
		@-----
cpy.128:	
		ands			r3, r2, #0x080
		beq 			cpy.64
		@---------------------
		@add				r4, r4, #128
		@---------------------
		
		vld1.64			{D0, D1, D2, D3}, [r1]!
		vld1.64			{D4, D5, D6, D7}, [r1]!
		vld1.64			{D8, D9, D10, D11}, [r1]!
		vld1.64			{D12, D13, D14, D15}, [r1]!
		
		vst1.64			{D0, D1, D2, D3}, [lr]!
		vst1.64			{D4, D5, D6, D7}, [lr]!
		vst1.64			{D8, D9, D10, D11}, [lr]!
		vst1.64			{D12, D13, D14, D15}, [lr]!
		
cpy.64:
		ands			r3, r2, #0x040
		beq 			cpy.32
		@---------------------
		@add				r4, r4, #64
		@---------------------
		vld1.64			{D0, D1, D2, D3}, [r1]!
		vld1.64			{D4, D5, D6, D7}, [r1]!
		
		vst1.64			{D0, D1, D2, D3}, [lr]!
		vst1.64			{D4, D5, D6, D7}, [lr]!
		
cpy.32:
		ands			r3, r2, #0x020
		beq 			cpy.16	
		@---------------------
		@add				r4, r4, #32
		@---------------------
		
		vld1.64			{D0, D1, D2, D3}, [r1]!
		
		vst1.64			{D0, D1, D2, D3}, [lr]!
		
cpy.16:
		ands			r3, r2, #0x010
		beq 			cpy.8		
		@---------------------
		@add				r4, r4, #16
		@---------------------
		
		vld1.64			{D0, D1}, [r1]!
		
		vst1.64			{D0, D1}, [lr]!
		
cpy.8:
		ands			r3, r2, #0x08
		beq 			cpy.4	
		@---------------------
		@add				r4, r4, #8
		@---------------------
		
		vld1.64			{D0}, [r1]!
		
		vst1.64			{D0}, [lr]!
		
cpy.4:
		ands			r3, r2, #0x04
		beq 			cpy.2		
		@---------------------
		@add				r4, r4, #4
		@---------------------
		
		vld1.32			{D0[0]}, [r1]!
		
		vst1.32			{D0[0]}, [lr]!
		
cpy.2:
		ands			r3, r2, #0x02
		beq 			cpy.1
		@---------------------
		@add				r4, r4, #2
		@---------------------
		
		vld1.16			{D0[0]}, [r1]!
		
		vst1.16			{D0[0]}, [lr]!
			
cpy.1:	
		ands			r3, r2, #0x01
		beq 			endofcpy
		@---------------------
		@add				r4, r4, #1
		@---------------------	
		
		vld1.8			{D0[0]}, [r1]!
		
		vst1.8			{D0[0]}, [lr]!
		
		
endofcpy:	
	
		ldmfd			sp!, {r4-r12, pc}
#endif
		
	.global _memcpy_neon










