
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
  6e:	af 93       	push	r26
  70:	bf 93       	push	r27
  72:	ef 93       	push	r30
  74:	ff 93       	push	r31
  76:	80 91 00 00 	lds	r24, 0x0000
  7a:	81 11       	cpse	r24, r1
  7c:	00 c0       	rjmp	.+0      	; 0x7e <__vector_10+0x2a>
  7e:	8f ef       	ldi	r24, 0xFF	; 255
  80:	89 bd       	out	0x29, r24	; 41
  82:	81 e0       	ldi	r24, 0x01	; 1
  84:	80 93 00 00 	sts	0x0000, r24
  88:	8c b5       	in	r24, 0x2c	; 44
  8a:	8f 7b       	andi	r24, 0xBF	; 191
  8c:	8c bd       	out	0x2c, r24	; 44
  8e:	8c b5       	in	r24, 0x2c	; 44
  90:	8f 7c       	andi	r24, 0xCF	; 207
  92:	8c bd       	out	0x2c, r24	; 44
  94:	bc 98       	cbi	0x17, 4	; 23
  96:	10 92 00 00 	sts	0x0000, r1
  9a:	10 92 00 00 	sts	0x0000, r1
  9e:	a0 91 00 00 	lds	r26, 0x0000
  a2:	e0 e0       	ldi	r30, 0x00	; 0
  a4:	f0 e0       	ldi	r31, 0x00	; 0
  a6:	86 b3       	in	r24, 0x16	; 22
  a8:	82 95       	swap	r24
  aa:	81 70       	andi	r24, 0x01	; 1
  ac:	a8 2b       	or	r26, r24
  ae:	a0 93 00 00 	sts	0x0000, r26
  b2:	96 b3       	in	r25, 0x16	; 22
  b4:	b0 91 00 00 	lds	r27, 0x0000
  b8:	92 95       	swap	r25
  ba:	91 70       	andi	r25, 0x01	; 1
  bc:	9a 23       	and	r25, r26
  be:	89 2f       	mov	r24, r25
  c0:	0e 2e       	mov	r0, r30
  c2:	00 c0       	rjmp	.+0      	; 0xc4 <__vector_10+0x70>
  c4:	88 0f       	add	r24, r24
  c6:	0a 94       	dec	r0
  c8:	02 f4       	brpl	.+0      	; 0xca <__vector_10+0x76>
  ca:	8b 2b       	or	r24, r27
  cc:	80 93 00 00 	sts	0x0000, r24
  d0:	81 e0       	ldi	r24, 0x01	; 1
  d2:	8e 0f       	add	r24, r30
  d4:	80 93 00 00 	sts	0x0000, r24
  d8:	31 96       	adiw	r30, 0x01	; 1
  da:	e8 30       	cpi	r30, 0x08	; 8
  dc:	f1 05       	cpc	r31, r1
  de:	01 f4       	brne	.+0      	; 0xe0 <__vector_10+0x8c>
  e0:	10 92 00 00 	sts	0x0000, r1
  e4:	e0 e0       	ldi	r30, 0x00	; 0
  e6:	f0 e0       	ldi	r31, 0x00	; 0
  e8:	86 b3       	in	r24, 0x16	; 22
  ea:	98 2f       	mov	r25, r24
  ec:	92 95       	swap	r25
  ee:	9f 70       	andi	r25, 0x0F	; 15
  f0:	89 2f       	mov	r24, r25
  f2:	81 70       	andi	r24, 0x01	; 1
  f4:	80 93 00 00 	sts	0x0000, r24
  f8:	86 b3       	in	r24, 0x16	; 22
  fa:	82 95       	swap	r24
  fc:	8f 70       	andi	r24, 0x0F	; 15
  fe:	89 23       	and	r24, r25
 100:	68 2f       	mov	r22, r24
 102:	61 70       	andi	r22, 0x01	; 1
 104:	70 e0       	ldi	r23, 0x00	; 0
 106:	cb 01       	movw	r24, r22
 108:	0e 2e       	mov	r0, r30
 10a:	00 c0       	rjmp	.+0      	; 0x10c <__vector_10+0xb8>
 10c:	88 0f       	add	r24, r24
 10e:	0a 94       	dec	r0
 110:	02 f4       	brpl	.+0      	; 0x112 <__vector_10+0xbe>
 112:	80 93 00 00 	sts	0x0000, r24
 116:	81 e0       	ldi	r24, 0x01	; 1
 118:	8e 0f       	add	r24, r30
 11a:	80 93 00 00 	sts	0x0000, r24
 11e:	31 96       	adiw	r30, 0x01	; 1
 120:	e8 30       	cpi	r30, 0x08	; 8
 122:	f1 05       	cpc	r31, r1
 124:	01 f4       	brne	.+0      	; 0x126 <__vector_10+0xd2>
 126:	10 92 00 00 	sts	0x0000, r1
 12a:	60 e0       	ldi	r22, 0x00	; 0
 12c:	70 e0       	ldi	r23, 0x00	; 0
 12e:	86 b3       	in	r24, 0x16	; 22
 130:	98 2f       	mov	r25, r24
 132:	92 95       	swap	r25
 134:	9f 70       	andi	r25, 0x0F	; 15
 136:	89 2f       	mov	r24, r25
 138:	81 70       	andi	r24, 0x01	; 1
 13a:	80 93 00 00 	sts	0x0000, r24
 13e:	86 b3       	in	r24, 0x16	; 22
 140:	82 95       	swap	r24
 142:	8f 70       	andi	r24, 0x0F	; 15
 144:	89 23       	and	r24, r25
 146:	48 2f       	mov	r20, r24
 148:	41 70       	andi	r20, 0x01	; 1
 14a:	50 e0       	ldi	r21, 0x00	; 0
 14c:	ca 01       	movw	r24, r20
 14e:	06 2e       	mov	r0, r22
 150:	00 c0       	rjmp	.+0      	; 0x152 <__vector_10+0xfe>
 152:	88 0f       	add	r24, r24
 154:	0a 94       	dec	r0
 156:	02 f4       	brpl	.+0      	; 0x158 <__vector_10+0x104>
 158:	80 93 00 00 	sts	0x0000, r24
 15c:	81 e0       	ldi	r24, 0x01	; 1
 15e:	86 0f       	add	r24, r22
 160:	80 93 00 00 	sts	0x0000, r24
 164:	6f 5f       	subi	r22, 0xFF	; 255
 166:	7f 4f       	sbci	r23, 0xFF	; 255
 168:	68 30       	cpi	r22, 0x08	; 8
 16a:	71 05       	cpc	r23, r1
 16c:	01 f4       	brne	.+0      	; 0x16e <__vector_10+0x11a>
 16e:	10 92 00 00 	sts	0x0000, r1
 172:	40 e0       	ldi	r20, 0x00	; 0
 174:	50 e0       	ldi	r21, 0x00	; 0
 176:	86 b3       	in	r24, 0x16	; 22
 178:	98 2f       	mov	r25, r24
 17a:	92 95       	swap	r25
 17c:	9f 70       	andi	r25, 0x0F	; 15
 17e:	89 2f       	mov	r24, r25
 180:	81 70       	andi	r24, 0x01	; 1
 182:	80 93 00 00 	sts	0x0000, r24
 186:	86 b3       	in	r24, 0x16	; 22
 188:	82 95       	swap	r24
 18a:	8f 70       	andi	r24, 0x0F	; 15
 18c:	89 23       	and	r24, r25
 18e:	28 2f       	mov	r18, r24
 190:	21 70       	andi	r18, 0x01	; 1
 192:	30 e0       	ldi	r19, 0x00	; 0
 194:	c9 01       	movw	r24, r18
 196:	04 2e       	mov	r0, r20
 198:	00 c0       	rjmp	.+0      	; 0x19a <__vector_10+0x146>
 19a:	88 0f       	add	r24, r24
 19c:	0a 94       	dec	r0
 19e:	02 f4       	brpl	.+0      	; 0x1a0 <__vector_10+0x14c>
 1a0:	80 93 00 00 	sts	0x0000, r24
 1a4:	81 e0       	ldi	r24, 0x01	; 1
 1a6:	84 0f       	add	r24, r20
 1a8:	80 93 00 00 	sts	0x0000, r24
 1ac:	4f 5f       	subi	r20, 0xFF	; 255
 1ae:	5f 4f       	sbci	r21, 0xFF	; 255
 1b0:	48 30       	cpi	r20, 0x08	; 8
 1b2:	51 05       	cpc	r21, r1
 1b4:	01 f4       	brne	.+0      	; 0x1b6 <__vector_10+0x162>
 1b6:	bc 9a       	sbi	0x17, 4	; 23
 1b8:	84 e1       	ldi	r24, 0x14	; 20
 1ba:	88 bb       	out	0x18, r24	; 24
 1bc:	ff 91       	pop	r31
 1be:	ef 91       	pop	r30
 1c0:	bf 91       	pop	r27
 1c2:	af 91       	pop	r26
 1c4:	9f 91       	pop	r25
 1c6:	8f 91       	pop	r24
 1c8:	7f 91       	pop	r23
 1ca:	6f 91       	pop	r22
 1cc:	5f 91       	pop	r21
 1ce:	4f 91       	pop	r20
 1d0:	3f 91       	pop	r19
 1d2:	2f 91       	pop	r18
 1d4:	0f 90       	pop	r0
 1d6:	0f be       	out	0x3f, r0	; 63
 1d8:	0f 90       	pop	r0
 1da:	1f 90       	pop	r1
 1dc:	18 95       	reti
 1de:	80 91 00 00 	lds	r24, 0x0000
 1e2:	81 30       	cpi	r24, 0x01	; 1
 1e4:	01 f0       	breq	.+0      	; 0x1e6 <__vector_10+0x192>
 1e6:	80 e4       	ldi	r24, 0x40	; 64
 1e8:	8b bd       	out	0x2b, r24	; 43
 1ea:	8a e0       	ldi	r24, 0x0A	; 10
 1ec:	89 bd       	out	0x29, r24	; 41
 1ee:	10 92 00 00 	sts	0x0000, r1
 1f2:	c2 9a       	sbi	0x18, 2	; 24
 1f4:	00 c0       	rjmp	.+0      	; 0x1f6 <__vector_10+0x1a2>
 1f6:	10 92 00 00 	sts	0x0000, r1
 1fa:	10 92 00 00 	sts	0x0000, r1
 1fe:	10 92 00 00 	sts	0x0000, r1
 202:	10 92 00 00 	sts	0x0000, r1
 206:	85 e3       	ldi	r24, 0x35	; 53
 208:	89 bd       	out	0x29, r24	; 41
 20a:	82 e0       	ldi	r24, 0x02	; 2
 20c:	80 93 00 00 	sts	0x0000, r24
 210:	8f eb       	ldi	r24, 0xBF	; 191
 212:	8b bd       	out	0x2b, r24	; 43
 214:	8c b5       	in	r24, 0x2c	; 44
 216:	80 63       	ori	r24, 0x30	; 48
 218:	8c bd       	out	0x2c, r24	; 44
 21a:	8c b5       	in	r24, 0x2c	; 44
 21c:	80 64       	ori	r24, 0x40	; 64
 21e:	8c bd       	out	0x2c, r24	; 44
 220:	00 c0       	rjmp	.+0      	; 0x222 <__vector_10+0x1ce>

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
