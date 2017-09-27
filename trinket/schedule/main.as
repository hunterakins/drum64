
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
  5e:	8f 93       	push	r24
  60:	80 91 00 00 	lds	r24, 0x0000
  64:	81 11       	cpse	r24, r1
  66:	00 c0       	rjmp	.+0      	; 0x68 <__vector_10+0x14>
  68:	10 92 00 00 	sts	0x0000, r1
  6c:	10 92 00 00 	sts	0x0000, r1
  70:	10 92 00 00 	sts	0x0000, r1
  74:	10 92 00 00 	sts	0x0000, r1
  78:	8c ed       	ldi	r24, 0xDC	; 220
  7a:	89 bd       	out	0x29, r24	; 41
  7c:	81 e0       	ldi	r24, 0x01	; 1
  7e:	80 93 00 00 	sts	0x0000, r24
  82:	8c b5       	in	r24, 0x2c	; 44
  84:	8f 7b       	andi	r24, 0xBF	; 191
  86:	8c bd       	out	0x2c, r24	; 44
  88:	8c b5       	in	r24, 0x2c	; 44
  8a:	8f 7c       	andi	r24, 0xCF	; 207
  8c:	8c bd       	out	0x2c, r24	; 44
  8e:	bc 98       	cbi	0x17, 4	; 23
  90:	10 92 00 00 	sts	0x0000, r1
  94:	10 92 00 00 	sts	0x0000, r1
  98:	86 b3       	in	r24, 0x16	; 22
  9a:	82 95       	swap	r24
  9c:	81 70       	andi	r24, 0x01	; 1
  9e:	88 0f       	add	r24, r24
  a0:	88 0f       	add	r24, r24
  a2:	88 0f       	add	r24, r24
  a4:	88 bb       	out	0x18, r24	; 24
  a6:	86 b3       	in	r24, 0x16	; 22
  a8:	82 95       	swap	r24
  aa:	81 70       	andi	r24, 0x01	; 1
  ac:	88 0f       	add	r24, r24
  ae:	88 0f       	add	r24, r24
  b0:	88 0f       	add	r24, r24
  b2:	88 bb       	out	0x18, r24	; 24
  b4:	80 91 00 00 	lds	r24, 0x0000
  b8:	8f 5f       	subi	r24, 0xFF	; 255
  ba:	80 93 00 00 	sts	0x0000, r24
  be:	88 30       	cpi	r24, 0x08	; 8
  c0:	00 f0       	brcs	.+0      	; 0xc2 <__vector_10+0x6e>
  c2:	10 92 00 00 	sts	0x0000, r1
  c6:	86 b3       	in	r24, 0x16	; 22
  c8:	82 95       	swap	r24
  ca:	81 70       	andi	r24, 0x01	; 1
  cc:	88 0f       	add	r24, r24
  ce:	88 0f       	add	r24, r24
  d0:	88 0f       	add	r24, r24
  d2:	88 bb       	out	0x18, r24	; 24
  d4:	86 b3       	in	r24, 0x16	; 22
  d6:	82 95       	swap	r24
  d8:	81 70       	andi	r24, 0x01	; 1
  da:	88 0f       	add	r24, r24
  dc:	88 0f       	add	r24, r24
  de:	88 0f       	add	r24, r24
  e0:	88 bb       	out	0x18, r24	; 24
  e2:	80 91 00 00 	lds	r24, 0x0000
  e6:	8f 5f       	subi	r24, 0xFF	; 255
  e8:	80 93 00 00 	sts	0x0000, r24
  ec:	88 30       	cpi	r24, 0x08	; 8
  ee:	00 f0       	brcs	.+0      	; 0xf0 <__vector_10+0x9c>
  f0:	10 92 00 00 	sts	0x0000, r1
  f4:	86 b3       	in	r24, 0x16	; 22
  f6:	82 95       	swap	r24
  f8:	81 70       	andi	r24, 0x01	; 1
  fa:	88 0f       	add	r24, r24
  fc:	88 0f       	add	r24, r24
  fe:	88 0f       	add	r24, r24
 100:	88 bb       	out	0x18, r24	; 24
 102:	86 b3       	in	r24, 0x16	; 22
 104:	82 95       	swap	r24
 106:	81 70       	andi	r24, 0x01	; 1
 108:	88 0f       	add	r24, r24
 10a:	88 0f       	add	r24, r24
 10c:	88 0f       	add	r24, r24
 10e:	88 bb       	out	0x18, r24	; 24
 110:	80 91 00 00 	lds	r24, 0x0000
 114:	8f 5f       	subi	r24, 0xFF	; 255
 116:	80 93 00 00 	sts	0x0000, r24
 11a:	88 30       	cpi	r24, 0x08	; 8
 11c:	00 f0       	brcs	.+0      	; 0x11e <__vector_10+0xca>
 11e:	10 92 00 00 	sts	0x0000, r1
 122:	86 b3       	in	r24, 0x16	; 22
 124:	82 95       	swap	r24
 126:	81 70       	andi	r24, 0x01	; 1
 128:	88 0f       	add	r24, r24
 12a:	88 0f       	add	r24, r24
 12c:	88 0f       	add	r24, r24
 12e:	88 bb       	out	0x18, r24	; 24
 130:	86 b3       	in	r24, 0x16	; 22
 132:	82 95       	swap	r24
 134:	81 70       	andi	r24, 0x01	; 1
 136:	88 0f       	add	r24, r24
 138:	88 0f       	add	r24, r24
 13a:	88 0f       	add	r24, r24
 13c:	88 bb       	out	0x18, r24	; 24
 13e:	80 91 00 00 	lds	r24, 0x0000
 142:	8f 5f       	subi	r24, 0xFF	; 255
 144:	80 93 00 00 	sts	0x0000, r24
 148:	88 30       	cpi	r24, 0x08	; 8
 14a:	00 f0       	brcs	.+0      	; 0x14c <__vector_10+0xf8>
 14c:	bc 9a       	sbi	0x17, 4	; 23
 14e:	84 e1       	ldi	r24, 0x14	; 20
 150:	88 bb       	out	0x18, r24	; 24
 152:	81 e0       	ldi	r24, 0x01	; 1
 154:	80 93 00 00 	sts	0x0000, r24
 158:	8f 91       	pop	r24
 15a:	0f 90       	pop	r0
 15c:	0f be       	out	0x3f, r0	; 63
 15e:	0f 90       	pop	r0
 160:	1f 90       	pop	r1
 162:	18 95       	reti
 164:	80 91 00 00 	lds	r24, 0x0000
 168:	81 30       	cpi	r24, 0x01	; 1
 16a:	01 f0       	breq	.+0      	; 0x16c <__vector_10+0x118>
 16c:	80 e4       	ldi	r24, 0x40	; 64
 16e:	8b bd       	out	0x2b, r24	; 43
 170:	86 e0       	ldi	r24, 0x06	; 6
 172:	89 bd       	out	0x29, r24	; 41
 174:	10 92 00 00 	sts	0x0000, r1
 178:	c2 9a       	sbi	0x18, 2	; 24
 17a:	8f 91       	pop	r24
 17c:	0f 90       	pop	r0
 17e:	0f be       	out	0x3f, r0	; 63
 180:	0f 90       	pop	r0
 182:	1f 90       	pop	r1
 184:	18 95       	reti
 186:	8e e1       	ldi	r24, 0x1E	; 30
 188:	89 bd       	out	0x29, r24	; 41
 18a:	82 e0       	ldi	r24, 0x02	; 2
 18c:	80 93 00 00 	sts	0x0000, r24
 190:	8f eb       	ldi	r24, 0xBF	; 191
 192:	8b bd       	out	0x2b, r24	; 43
 194:	8c b5       	in	r24, 0x2c	; 44
 196:	80 63       	ori	r24, 0x30	; 48
 198:	8c bd       	out	0x2c, r24	; 44
 19a:	8c b5       	in	r24, 0x2c	; 44
 19c:	80 64       	ori	r24, 0x40	; 64
 19e:	8c bd       	out	0x2c, r24	; 44
 1a0:	00 c0       	rjmp	.+0      	; 0x1a2 <__vector_10+0x14e>

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
