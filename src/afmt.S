	.extern text_beg
	.extern text_beg_real
	.extern text_end
	.extern data_beg
	.extern data_end
	.extern bss_beg
	.extern bss_beg_real
	.extern bss_end
	.extern _TEXT_LEN
	.extern _DATA_LEN
	.extern _BSS_LEN

	.section .boot
file_start:
	.extern _start
_FIXUP_DUMMY_OFFS:
	/* enter supervisor mode */
	pea 0
	move.w #32, -(%a7)
	trap #1
	move.l %d0, oldstack
	addq.l #6, %a7

	/* turn interrupts off */
	move.w %sr, %d0
	move.w #0x2700, %sr

	/* replace interrupt handler */
	move.l 0x0070, old_vbl_handler
	move.l #vbl_handler, 0x0070

	/* turn them back on */
	move.w %d0, %sr

	/* jump to code */
	jsr _start

	/* turn interrupts off */
	move.w %sr, %d0
	move.w #0x2700, %sr

	/* restore interrupt handler */
	move.l old_vbl_handler, 0x0070

	/* turn them back on */
	move.w %d0, %sr

	/* leave supervisor mode */
	move.l oldstack, -(%sp)
	move.w #32, -(%a7)
	trap #1
	addq.l #6, %a7

	/* exit */
	move.w #0, -(%a7)
	trap #1
	addq.l #2, %a7
	donk: bra donk

	/* VBL HANDLER */
	.global vbl_handler
	.extern music_update
vbl_handler:
	movem.l %d0-%d7/%a0-%a6, -(%a7)

	/* Update music */
	jsr music_update

	/* Update VBL counter */
	addq.l #1, vbl_counter

	/* Query keyboard */
	move.w #11, -(%sp)
	trap #1
	addq.l #2, %sp
	eor.l %d0, vbl_key_pressed

	movem.l (%a7)+, %d0-%d7/%a0-%a6
	rte

_FIXUP_DUMMY:
	.long 0

	.section .data
	.long 0
	.global oldstack
oldstack: .long 0
	.long 0
	.global vbl_counter
vbl_counter: .long 0
	.global vbl_key_pressed
vbl_key_pressed: .long 0
old_vbl_handler: .long 0

/*
	.section .footer
sym_beg:
sym_end:
	.long _FIXUP_DUMMY - _FIXUP_DUMMY_OFFS + 0x1C
*/

