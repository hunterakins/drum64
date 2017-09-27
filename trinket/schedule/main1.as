
obj/main.o:     file format elf32-avr


Disassembly of section .text:

00000000 <timer0_init>:
   0:	8a b5       	in	r24, 0x2a	; 42
   2:	82 60       	ori	r24, 0x02	; 2
   4:	8a bd       	out	0x2a, r24	; 42
   6:	83 b7       	in	r24, 0x33	; 51
   8:	82 60       	ori	r24, 0x02	; 2
   a:	83 bf       	out	0x33, r24	; 51
   c:	12 be       	out	0x32, r1	; 50
   e:	88 e0       	ldi	r24, 0x08	; 8
  10:	89 bd       	out	0x29, r24	; 41
  12:	89 b7       	in	r24, 0x39	; 57
  14:	80 61       	ori	r24, 0x10	; 16
  16:	89 bf       	out	0x39, r24	; 57
  18:	10 92 00 00 	sts	0x0000, r1
  1c:	78 94       	sei
  1e:	08 95       	ret

00000020 <timer1_init>:
  20:	8c b5       	in	r24, 0x2c	; 44
  22:	80 63       	ori	r24, 0x30	; 48
  24:	8c bd       	out	0x2c, r24	; 44
  26:	80 b7       	in	r24, 0x30	; 48
  28:	80 63       	ori	r24, 0x30	; 48
  2a:	80 bf       	out	0x30, r24	; 48
  2c:	8c b5       	in	r24, 0x2c	; 44
  2e:	80 64       	ori	r24, 0x40	; 64
  30:	8c bd       	out	0x2c, r24	; 44
  32:	80 b7       	in	r24, 0x30	; 48
  34:	81 60       	ori	r24, 0x01	; 1
  36:	80 bf       	out	0x30, r24	; 48
  38:	1f bc       	out	0x2f, r1	; 47
  3a:	84 e0       	ldi	r24, 0x04	; 4
  3c:	87 bd       	out	0x27, r24	; 39
  3e:	80 e4       	ldi	r24, 0x40	; 64
  40:	8b bd       	out	0x2b, r24	; 43
  42:	8f ef       	ldi	r24, 0xFF	; 255
  44:	8d bd       	out	0x2d, r24	; 45
  46:	08 95       	ret

00000048 <spi_init>:
  48:	6c 9a       	sbi	0x0d, 4	; 13
  4a:	08 95       	ret

0000004c <spi_transfer>:
  4c:	8f b9       	out	0x0f, r24	; 15
  4e:	7f 9b       	sbis	0x0f, 7	; 15
  50:	00 c0       	rjmp	.+0      	; 0x52 <spi_transfer+0x6>
  52:	08 95       	ret

00000054 <__vector_10>:
  54:	1f 92       	push	r1
  56:	0f 92       	push	r0
  58:	0f b6       	in	r0, 0x3f	; 63
  5a:	0f 92       	push	r0
  5c:	11 24       	eor	r1, r1
  5e:	2f 93       	push	r18
  60:	3f 93       	push	r19
  62:	4f 93       	push	r20
  64:	5f 93       	push	r21
  66:	6f 93       	push	r22
  68:	7f 93       	push	r23
  6a:	8f 93       	push	r24
  6c:	9f 93       	push	r25
  6e:	ef 93       	push	r30
  70:	ff 93       	push	r31
  72:	80 91 00 00 	lds	r24, 0x0000
  76:	81 11       	cpse	r24, r1
  78:	00 c0       	rjmp	.+0      	; 0x7a <__vector_10+0x26>
  7a:	10 92 00 00 	sts	0x0000, r1
  7e:	10 92 00 00 	sts	0x0000, r1
  82:	10 92 00 00 	sts	0x0000, r1
  86:	10 92 00 00 	sts	0x0000, r1
  8a:	8c ed       	ldi	r24, 0xDC	; 220
  8c:	89 bd       	out	0x29, r24	; 41
  8e:	81 e0       	ldi	r24, 0x01	; 1
  90:	80 93 00 00 	sts	0x0000, r24
  94:	8c b5       	in	r24, 0x2c	; 44
  96:	8f 7b       	andi	r24, 0xBF	; 191
  98:	8c bd       	out	0x2c, r24	; 44
  9a:	8c b5       	in	r24, 0x2c	; 44
  9c:	8f 7c       	andi	r24, 0xCF	; 207
  9e:	8c bd       	out	0x2c, r24	; 44
  a0:	bc 98       	cbi	0x17, 4	; 23
  a2:	10 92 00 00 	sts	0x0000, r1
  a6:	10 92 00 00 	sts	0x0000, r1
  aa:	81 e0       	ldi	r24, 0x01	; 1
  ac:	96 b3       	in	r25, 0x16	; 22
  ae:	94 fb       	bst	r25, 4
  b0:	22 27       	eor	r18, r18
  b2:	20 f9       	bld	r18, 0
  b4:	30 e0       	ldi	r19, 0x00	; 0
  b6:	a9 01       	movw	r20, r18
  b8:	44 0f       	add	r20, r20
  ba:	55 1f       	adc	r21, r21
  bc:	44 0f       	add	r20, r20
  be:	55 1f       	adc	r21, r21
  c0:	44 0f       	add	r20, r20
  c2:	55 1f       	adc	r21, r21
  c4:	05 2e       	mov	r0, r21
  c6:	00 0c       	add	r0, r0
  c8:	66 0b       	sbc	r22, r22
  ca:	77 0b       	sbc	r23, r23
  cc:	40 93 00 00 	sts	0x0000, r20
  d0:	50 93 00 00 	sts	0x0000, r21
  d4:	60 93 00 00 	sts	0x0000, r22
  d8:	70 93 00 00 	sts	0x0000, r23
  dc:	96 b3       	in	r25, 0x16	; 22
  de:	94 fb       	bst	r25, 4
  e0:	ee 27       	eor	r30, r30
  e2:	e0 f9       	bld	r30, 0
  e4:	f0 e0       	ldi	r31, 0x00	; 0
  e6:	af 01       	movw	r20, r30
  e8:	44 0f       	add	r20, r20
  ea:	55 1f       	adc	r21, r21
  ec:	44 0f       	add	r20, r20
  ee:	55 1f       	adc	r21, r21
  f0:	44 0f       	add	r20, r20
  f2:	55 1f       	adc	r21, r21
  f4:	05 2e       	mov	r0, r21
  f6:	00 0c       	add	r0, r0
  f8:	66 0b       	sbc	r22, r22
  fa:	77 0b       	sbc	r23, r23
  fc:	40 93 00 00 	sts	0x0000, r20
 100:	50 93 00 00 	sts	0x0000, r21
 104:	60 93 00 00 	sts	0x0000, r22
 108:	70 93 00 00 	sts	0x0000, r23
 10c:	80 93 00 00 	sts	0x0000, r24
 110:	8f 5f       	subi	r24, 0xFF	; 255
 112:	89 30       	cpi	r24, 0x09	; 9
 114:	01 f4       	brne	.+0      	; 0x116 <__vector_10+0xc2>
 116:	10 92 00 00 	sts	0x0000, r1
 11a:	86 b3       	in	r24, 0x16	; 22
 11c:	82 95       	swap	r24
 11e:	81 70       	andi	r24, 0x01	; 1
 120:	88 0f       	add	r24, r24
 122:	88 0f       	add	r24, r24
 124:	88 0f       	add	r24, r24
 126:	88 bb       	out	0x18, r24	; 24
 128:	86 b3       	in	r24, 0x16	; 22
 12a:	82 95       	swap	r24
 12c:	81 70       	andi	r24, 0x01	; 1
 12e:	88 0f       	add	r24, r24
 130:	88 0f       	add	r24, r24
 132:	88 0f       	add	r24, r24
 134:	88 bb       	out	0x18, r24	; 24
 136:	80 91 00 00 	lds	r24, 0x0000
 13a:	8f 5f       	subi	r24, 0xFF	; 255
 13c:	80 93 00 00 	sts	0x0000, r24
 140:	88 30       	cpi	r24, 0x08	; 8
 142:	00 f0       	brcs	.+0      	; 0x144 <__vector_10+0xf0>
 144:	10 92 00 00 	sts	0x0000, r1
 148:	86 b3       	in	r24, 0x16	; 22
 14a:	82 95       	swap	r24
 14c:	81 70       	andi	r24, 0x01	; 1
 14e:	88 0f       	add	r24, r24
 150:	88 0f       	add	r24, r24
 152:	88 0f       	add	r24, r24
 154:	88 bb       	out	0x18, r24	; 24
 156:	86 b3       	in	r24, 0x16	; 22
 158:	82 95       	swap	r24
 15a:	81 70       	andi	r24, 0x01	; 1
 15c:	88 0f       	add	r24, r24
 15e:	88 0f       	add	r24, r24
 160:	88 0f       	add	r24, r24
 162:	88 bb       	out	0x18, r24	; 24
 164:	80 91 00 00 	lds	r24, 0x0000
 168:	8f 5f       	subi	r24, 0xFF	; 255
 16a:	80 93 00 00 	sts	0x0000, r24
 16e:	88 30       	cpi	r24, 0x08	; 8
 170:	00 f0       	brcs	.+0      	; 0x172 <__vector_10+0x11e>
 172:	10 92 00 00 	sts	0x0000, r1
 176:	86 b3       	in	r24, 0x16	; 22
 178:	82 95       	swap	r24
 17a:	81 70       	andi	r24, 0x01	; 1
 17c:	88 0f       	add	r24, r24
 17e:	88 0f       	add	r24, r24
 180:	88 0f       	add	r24, r24
 182:	88 bb       	out	0x18, r24	; 24
 184:	86 b3       	in	r24, 0x16	; 22
 186:	82 95       	swap	r24
 188:	81 70       	andi	r24, 0x01	; 1
 18a:	88 0f       	add	r24, r24
 18c:	88 0f       	add	r24, r24
 18e:	88 0f       	add	r24, r24
 190:	88 bb       	out	0x18, r24	; 24
 192:	80 91 00 00 	lds	r24, 0x0000
 196:	8f 5f       	subi	r24, 0xFF	; 255
 198:	80 93 00 00 	sts	0x0000, r24
 19c:	88 30       	cpi	r24, 0x08	; 8
 19e:	00 f0       	brcs	.+0      	; 0x1a0 <__vector_10+0x14c>
 1a0:	bc 9a       	sbi	0x17, 4	; 23
 1a2:	84 e1       	ldi	r24, 0x14	; 20
 1a4:	88 bb       	out	0x18, r24	; 24
 1a6:	81 e0       	ldi	r24, 0x01	; 1
 1a8:	80 93 00 00 	sts	0x0000, r24
 1ac:	ff 91       	pop	r31
 1ae:	ef 91       	pop	r30
 1b0:	9f 91       	pop	r25
 1b2:	8f 91       	pop	r24
 1b4:	7f 91       	pop	r23
 1b6:	6f 91       	pop	r22
 1b8:	5f 91       	pop	r21
 1ba:	4f 91       	pop	r20
 1bc:	3f 91       	pop	r19
 1be:	2f 91       	pop	r18
 1c0:	0f 90       	pop	r0
 1c2:	0f be       	out	0x3f, r0	; 63
 1c4:	0f 90       	pop	r0
 1c6:	1f 90       	pop	r1
 1c8:	18 95       	reti
 1ca:	80 91 00 00 	lds	r24, 0x0000
 1ce:	81 30       	cpi	r24, 0x01	; 1
 1d0:	01 f0       	breq	.+0      	; 0x1d2 <__vector_10+0x17e>
 1d2:	80 e4       	ldi	r24, 0x40	; 64
 1d4:	8b bd       	out	0x2b, r24	; 43
 1d6:	86 e0       	ldi	r24, 0x06	; 6
 1d8:	89 bd       	out	0x29, r24	; 41
 1da:	10 92 00 00 	sts	0x0000, r1
 1de:	c2 9a       	sbi	0x18, 2	; 24
 1e0:	00 c0       	rjmp	.+0      	; 0x1e2 <__vector_10+0x18e>
 1e2:	8e e1       	ldi	r24, 0x1E	; 30
 1e4:	89 bd       	out	0x29, r24	; 41
 1e6:	82 e0       	ldi	r24, 0x02	; 2
 1e8:	80 93 00 00 	sts	0x0000, r24
 1ec:	8f eb       	ldi	r24, 0xBF	; 191
 1ee:	8b bd       	out	0x2b, r24	; 43
 1f0:	8c b5       	in	r24, 0x2c	; 44
 1f2:	80 63       	ori	r24, 0x30	; 48
 1f4:	8c bd       	out	0x2c, r24	; 44
 1f6:	8c b5       	in	r24, 0x2c	; 44
 1f8:	80 64       	ori	r24, 0x40	; 64
 1fa:	8c bd       	out	0x2c, r24	; 44
 1fc:	00 c0       	rjmp	.+0      	; 0x1fe <__vector_10+0x1aa>

Disassembly of section .text.startup:

00000000 <main>:
   0:	00 d0       	rcall	.+0      	; 0x2 <main+0x2>
   2:	00 d0       	rcall	.+0      	; 0x4 <main+0x4>
   4:	6c 9a       	sbi	0x0d, 4	; 13
   6:	8a e1       	ldi	r24, 0x1A	; 26
   8:	87 bb       	out	0x17, r24	; 23
   a:	00 c0       	rjmp	.+0      	; 0xc <__zero_reg__+0xb>

Disassembly of section .comment:

00000000 <.comment>:
   0:	00 47       	sbci	r16, 0x70	; 112
   2:	43 43       	sbci	r20, 0x33	; 51
   4:	3a 20       	and	r3, r10
   6:	28 47       	sbci	r18, 0x78	; 120
   8:	4e 55       	subi	r20, 0x5E	; 94
   a:	29 20       	and	r2, r9
   c:	34 2e       	mov	r3, r20
   e:	39 2e       	mov	r3, r25
  10:	32 00       	.word	0x0032	; ????
