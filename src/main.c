#include "ST.h"

#include "mdat.h"
#include "dat/font.h"
#include "dat/s01.h"

extern uint8_t end[];
extern volatile uint32_t vbl_counter;
extern volatile int32_t vbl_key_pressed;
uint32_t vbl_expected_counter;

uint16_t old_pal[16];
uint8_t old_mode = 0;
uint8_t old_vmem_h = 0;
uint8_t old_vmem_m = 0;
uint8_t old_vmem_l = 0;
uint8_t old_stride = 0;

uint8_t vmem_base[256*2+160*(200+32+16)];
uint16_t *vmem;

const int8_t sin_tab[256] = {
	0, 2, 3, 5, 6, 8, 9, 11, 12, 14, 16, 17, 19, 20, 22, 23,
	24, 26, 27, 29, 30, 32, 33, 34, 36, 37, 38, 39, 41, 42, 43, 44,
	45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59,
	59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 63, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 60, 60,
	59, 59, 58, 57, 56, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46,
	45, 44, 43, 42, 41, 39, 38, 37, 36, 34, 33, 32, 30, 29, 27, 26,
	24, 23, 22, 20, 19, 17, 16, 14, 12, 11, 9, 8, 6, 5, 3, 2,
	0, -2, -3, -5, -6, -8, -9, -11, -12, -14, -16, -17, -19, -20, -22, -23,
	-24, -26, -27, -29, -30, -32, -33, -34, -36, -37, -38, -39, -41, -42, -43, -44,
	-45, -46, -47, -48, -49, -50, -51, -52, -53, -54, -55, -56, -56, -57, -58, -59,
	-59, -60, -60, -61, -61, -62, -62, -62, -63, -63, -63, -64, -64, -64, -64, -64,
	-64, -64, -64, -64, -64, -64, -63, -63, -63, -62, -62, -62, -61, -61, -60, -60,
	-59, -59, -58, -57, -56, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46,
	-45, -44, -43, -42, -41, -39, -38, -37, -36, -34, -33, -32, -30, -29, -27, -26,
	-24, -23, -22, -20, -19, -17, -16, -14, -12, -11, -9, -8, -6, -5, -3, -2,
};

#include "music.h"

void unpack_rle(uint8_t *target, uint8_t *dat, uint16_t len)
{
	for(uint16_t i = 0; i < len; i+=3) {
		uint8_t run_len = *(dat++);
		uint8_t run_byte = *(dat++);
		do {
			target[0] = run_byte;
			if((((uintptr_t)target)&1) != 0) {
				target += 7;
			} else {
				target += 1;
			}
		} while((--run_len) != 0);
		target[0] = *(dat++);
		if((((uintptr_t)target)&1) != 0) {
			target += 7;
		} else {
			target += 1;
		}
	}
}

void vwait_force()
{
	uint32_t basectr = vbl_counter;
	while(basectr == vbl_counter) {
		//
	}
}

void vwait(int times)
{
	vbl_expected_counter += times;
	while(((int32_t)(vbl_expected_counter-vbl_counter)) > 0) {
		//
	}
}

void vwait_reset(void)
{
	vbl_expected_counter = vbl_counter;
}

// CHEQUERBOARDS GO HERE
#include "chequers/plain.h"
#include "chequers/gswap.h"

void chequer_init()
{
	for(int16_t j = 0; j < 16; j++) {
		chequer_rotates[0][j] = 0;
	}

	for(int16_t i = 1; i <= 16; i++) {
		uint16_t v = (1<<i)-1;
		for(int16_t j = 0; j < 16; j++) {
			chequer_rotates[32-i][(j+i)&15] = v;
			chequer_rotates[i][j] = v;
			v = (v<<1)|(v>>15);
		}
	}
}

// PARTS GO HERE
#include "parts/introtext.h"
#include "parts/title.h"
#include "parts/gswap1.h"
#include "parts/melt1.h"

void _start(void)
{
	vmem = (uint16_t *)((0xFF+160*8+(uintptr_t)vmem_base)&~0xFF);

	// Save palette
	for(int i = 0; i < 16; i++) {
		old_pal[i] = VID_PAL0[i];
	}

	// Generate tables
	chequer_init();

	// Set up vbl timing
	vwait_reset();
	vwait(1);

	music_playing = 1;

	// Perform fade
	for(int i = 1; i < 16; i++) {
		vwait(1);
		for(int j = 0; j < 16; j++) {
			uint16_t v = old_pal[j];
			v = ((v<<1)&0x0EEE)|((v>>3)&0x0111);
			uint16_t r = v&0x0F00;
			uint16_t g = v&0x00F0;
			uint16_t b = v&0x000F;
			r = (r < (i<<8) ? 0 : r-(i<<8));
			g = (g < (i<<4) ? 0 : g-(i<<4));
			b = (b < (i<<0) ? 0 : b-(i<<0));
			v = r|g|b;
			v = ((v<<3)&0x0888)|((v>>1)&0x0777);
			VID_PAL0[j] = v;
		}
	}

	// Set up video
	vwait_force();
	old_mode = VID_SHIFT_MODE_ST;
	old_vmem_h = VID_BASE_H;
	old_vmem_m = VID_BASE_M;
	old_vmem_l = VID_BASE_L_STE;
	old_stride = VID_STRIDE_STE;
	VID_SHIFT_MODE_ST = 0x00;
	VID_BASE_H = ((uint32_t)vmem)>>16;
	VID_BASE_M = ((uint32_t)vmem)>>8;
	VID_STRIDE_STE = 0;

	for(int i = 0; i < 80*200; i+=4) {
		*(uint64_t *)(uint16_t *)&vmem[i] = 0;
	}

	// PARTS - run them
	if(vbl_key_pressed == 0) { introtext_main(); }
	if(vbl_key_pressed == 0) { title_main(); }
	if(vbl_key_pressed == 0) { gswap1_main(); }
	if(vbl_key_pressed == 0) { melt1_main(); }

	// Restore video
	VID_SHIFT_MODE_ST = old_mode;
	VID_BASE_H = old_vmem_h;
	VID_BASE_M = old_vmem_m;
	VID_BASE_L_STE = old_vmem_l;
	VID_STRIDE_STE = old_stride;

	// Restore palette
	vwait_force();
	for(int i = 0; i < 16; i++) {
		VID_PAL0[i] = old_pal[i];
	}

	// Kill sound
	music_playing = 0;
	PSG_REG = 0x07; PSG_DAT_W = 0x3F;
	PSG_REG = 0x0D; PSG_DAT_W = 0x00;
}
