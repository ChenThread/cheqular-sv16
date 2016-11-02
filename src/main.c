#include "ST.h"

#include "mdat.h"
#include "dat/font.h"
#include "dat/s01.h"

extern uint8_t end[];
extern volatile uint32_t vbl_counter;
uint32_t vbl_expected_counter;

uint16_t old_pal[16];
uint8_t old_mode = 0;
uint8_t old_vmem_h = 0;
uint8_t old_vmem_m = 0;
uint8_t old_vmem_l = 0;
uint8_t old_stride = 0;

uint8_t vmem_base[256*2+8*40*200];
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

const uint16_t music_note_periods[] = {
	3822, 3608, 3405, 3214, 3034, 2863,
	2703, 2551, 2408, 2273, 2145, 2025,
};

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

volatile uint8_t music_playing = 0;
int16_t music_detune = 0;
uint16_t music_offs_a = 0*4*16;
uint16_t music_offs_c = 0*4*16;
uint16_t music_offs_d = 0*4*16;
uint8_t music_time_wait = 0;
uint8_t music_vol_a = 0;
uint8_t music_vol_b = 0;
uint8_t music_vol_c = 0;
uint8_t music_prev_c = 0;
void music_update(void)
{
	if(!music_playing) {
		return;
	}

	//music_detune -= 2; 

	if(music_time_wait > 0) {
		music_time_wait--;
		if(music_vol_a > 0) {
			music_vol_a--;
			if(music_vol_a < 15) {
				PSG_REG = 0x08; PSG_DAT_W = music_vol_a;
			}
		}
		if(music_vol_b > 0) {
			music_vol_b--;
			if(music_vol_b < 15) {
				PSG_REG = 0x09; PSG_DAT_W = music_vol_b;
			}
		}
		if(music_vol_c > 12*2) {
			music_vol_c--;
			//PSG_REG = 0x0A; PSG_DAT_W = 0x10|(music_vol_c>>1);
		}
		return;
	}
	music_time_wait = 8;

	// Get new notes
	uint8_t na = mdat_pat_a[mdat_ord_a[music_offs_a>>4]][music_offs_a&15];
	uint8_t nc = mdat_pat_c[mdat_ord_c[music_offs_c>>4]][music_offs_c&15];
	uint8_t nd = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
	uint8_t outmask = 0x3F;

	if(music_detune != 0) {
		nc = 0;
		nd = 0;
	}

	// Force a pop
	PSG_REG = 0x08; PSG_DAT_W = 0x0F;

	if(na != 0) {
		uint16_t pa0 = music_note_periods[na&0xF];
		pa0 >>= (na>>4);
		uint16_t pa = pa0;
		if(music_detune != 0) {
			na = (na&0xF) + (na>>4)*12;
			na += music_detune>>8;
			if(na < 0x01 || na >= 0x80) {
				na = 1;
			}
			na = (na%12) + ((na/12)<<4);
			pa = pa0 = music_note_periods[na&0xF];
			pa0 >>= (na>>4);
			if(na != 1) {
				uint8_t na1 = na+1;
				if((na1&0xF) >= 0xC) {
					na1 += 4;
				}
				uint16_t pa1 = music_note_periods[na1&0xF];
				pa1 >>= (na1>>4);
				int32_t pa0_base = pa0<<8;
				int32_t pa1_base = pa1-pa0;
				pa1_base *= (music_detune&0xFF);
				int32_t pa_base = pa0_base + pa1_base;
				pa_base += 128;
				pa_base >>= 8;
				pa = pa_base;
				if(pa > 0xFFF) {
					pa = 0xFFF;
				}
			}
		}
		PSG_REG = 0x00; PSG_DAT_W = pa;
		PSG_REG = 0x01; PSG_DAT_W = pa>>8;
		outmask &= ~0x01;
		if(nc == 0) {
			pa -= 5;
			PSG_REG = 0x02; PSG_DAT_W = pa;
			PSG_REG = 0x03; PSG_DAT_W = pa>>8;
			music_vol_b = 0x0F+1;
			PSG_REG = 0x09; PSG_DAT_W = 0x0F;
			outmask &= ~0x02;
		}
	}

	if(nc != 0) {
		uint16_t pc = music_note_periods[nc&0xF];
		pc >>= (nc>>4);
		pc >>= 1;
		PSG_REG = 0x04; PSG_DAT_W = pc;
		PSG_REG = 0x05; PSG_DAT_W = pc>>8;

		if(music_prev_c != nc) {
			PSG_REG = 0x0A; PSG_DAT_W = 0x1F;
			music_vol_c = 0x0F*2;
			PSG_REG = 0x02; PSG_DAT_W = pc;
			PSG_REG = 0x03; PSG_DAT_W = pc>>8;
			music_vol_b = 0x0F;
			PSG_REG = 0x09; PSG_DAT_W = 0x0F;
		}
		pc += 2;
		pc >>= 2;
		PSG_REG = 0x0B; PSG_DAT_W = pc;
		PSG_REG = 0x0C; PSG_DAT_W = pc>>8;
		PSG_REG = 0x0D; PSG_DAT_W = 0x08;

		outmask &= ~0x06;
	} else {
		PSG_REG = 0x0A; PSG_DAT_W = 0x00;
	}
	music_prev_c = nc;

	if(nd != 0) {
		PSG_REG = 0x06; PSG_DAT_W = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
		outmask &= ~0x08;
	}

	if(na != 0 || nd != 0) {
		music_vol_a = 0x0F+2;
		PSG_REG = 0x08; PSG_DAT_W = 0x0F;
	}

	PSG_REG = 0x07; PSG_DAT_W = outmask;

	music_offs_a++;
	music_offs_a %= sizeof(mdat_ord_a)*16;
	music_offs_c++;
	music_offs_c %= sizeof(mdat_ord_c)*16;
	music_offs_d++;
	music_offs_d %= sizeof(mdat_ord_d)*16;
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

const int16_t intro_dirs[][2] = {
	{-4, 0},
	{ 0, 3},
	{ 3,-2},
};

char const*const intro_texts[] = {
	"chen thread",
	"presents",
	":3",
	NULL,
	"a demo for",
	"sillyventure"
	"2016",
	"written in",
	"2 days",
	NULL,
	"which asie",
	"probably never",
	"wanted me to",
	"write",
	NULL,
	NULL,
};
uint16_t intro_chequer_idx = 0;

// [amount_left][old_offset]
uint16_t chequer_rotates[32][16];
uint16_t coffs_plain_x = 0;
uint16_t coffs_plain_y = 0;

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

void chequer_update_plain(int16_t mvx, int16_t mvy)
{
	// X scroll
	uint16_t old_offs_x = coffs_plain_x;
	coffs_plain_x += mvx;
	uint16_t new_offs_x = coffs_plain_x;
	uint16_t old_offs_y = coffs_plain_y;
	coffs_plain_y += mvy;
	uint16_t new_offs_y = coffs_plain_y;
	uint16_t r = chequer_rotates[mvx&31][old_offs_x&15];
	uint16_t *p = vmem+3;

	// Y scroll
	// TODO: fix line bug
	uint16_t flipy0 = 999;
	uint16_t flipy1 = 999;
	uint16_t flipyi = 0;
	if(old_offs_y != new_offs_y) {
		if(mvy < 0) {
			flipy0 = new_offs_y&15;
			flipy1 = old_offs_y&15;
		} else {
			flipy0 = old_offs_y&15;
			flipy1 = new_offs_y&15;
		}
		if(flipy0 > flipy1) {
			r ^= 0xFFFF;
			flipy0 ^= flipy1;
			flipy1 ^= flipy0;
			flipy0 ^= flipy1;
		}
	}
	uint16_t flipyd = flipy0^flipy1;

	for(int16_t y = 0; y < 200; y += 1) {
		if(y == flipy0) {
			r = ~r;
			flipy0 ^= flipyd;
			flipy0 += flipyi;
			flipyi ^= 16;
		}

		p[ 0*4] ^= r;
		p[ 1*4] ^= r;
		p[ 2*4] ^= r;
		p[ 3*4] ^= r;
		p[ 4*4] ^= r;
		p[ 5*4] ^= r;
		p[ 6*4] ^= r;
		p[ 7*4] ^= r;
		p[ 8*4] ^= r;
		p[ 9*4] ^= r;
		p[10*4] ^= r;
		p[11*4] ^= r;
		p[12*4] ^= r;
		p[13*4] ^= r;
		p[14*4] ^= r;
		p[15*4] ^= r;
		p[16*4] ^= r;
		p[17*4] ^= r;
		p[18*4] ^= r;
		p[19*4] ^= r;
		p += 80;
	}
}

void intro_text_vwait(int wait_counter)
{
	for(int i = 0; i < wait_counter; i++) {
		// Update chequerboard
		chequer_update_plain(
			intro_dirs[intro_chequer_idx][0],
			intro_dirs[intro_chequer_idx][1]);

		// Wait for vblank
		vwait(1);
	}
}

void intro_text(void)
{
	//
	// Fill background with chequerboard
	for(uint16_t y = 0; y < 200; y+=32) {
		uint16_t *p = &vmem[80*y];
		for(uint16_t x = 0; x < 80*16; x+=8) {
			p[x+3] = 0xFFFF;
			p[x+7] = 0x0000;
			p[x+3+80*16] = 0x0000;
			p[x+7+80*16] = 0xFFFF;
		}
	}

	//
	VID_PAL0[0] = 0x0000;
	VID_PAL0[8] = 0x0001;
	VID_PAL0[1] = 0x0777;
	VID_PAL0[9] = 0x0777;
	vwait_reset();

	int line_idx = 0;
	int line_subidx = 0;
	uint8_t *screen_ptr = NULL;
	for(;;) {
		char const* char_ptr = intro_texts[line_idx];

		if(char_ptr == NULL) {
			//line_subidx = 0;
			line_idx++;

			intro_text_vwait(90);

			// Clear text plane
			for(int i = 0; i < 160*200; i+=4) {
				vmem[i] = 0;
			}

			// End section if at end of texts
			if(intro_texts[line_idx] == NULL) {
				break;
			}

			// Change direction
			intro_chequer_idx++;

			// Change palette
			VID_PAL0[8] <<= 4;

			continue;
		}

		int slen = strlen(char_ptr);
		int slen2 = slen/2;
		screen_ptr = (uint8_t *)vmem;
		screen_ptr += 20*8*(8*(2*line_subidx+1)+4);
		screen_ptr += 20*4-(slen2&~1)*4+1-(slen2&1);

		for(int x = 0; x < slen; x++) {
			uint16_t c = *(char_ptr++) - 0x20;
			c <<= 3;
			screen_ptr[0*80*2] = font_data[c+0];
			screen_ptr[1*80*2] = font_data[c+1];
			screen_ptr[2*80*2] = font_data[c+2];
			screen_ptr[3*80*2] = font_data[c+3];
			screen_ptr[4*80*2] = font_data[c+4];
			screen_ptr[5*80*2] = font_data[c+5];
			screen_ptr[6*80*2] = font_data[c+6];
			screen_ptr[7*80*2] = font_data[c+7];
			intro_text_vwait(2);
			if((((uintptr_t)screen_ptr)&1) != 0) {
				screen_ptr += 7;
			} else {
				screen_ptr += 1;
			}
		}

		line_subidx++;
		line_idx++;
	}

}

void plain_chequer()
{
	// hide image upload
	for(int i = 0; i < 8; i++) {
		VID_PAL0[0+i] = 0x0000;
		VID_PAL0[8+i] = 0x0123;
	}

	vwait(1);
	unpack_rle((uint8_t *)(vmem+0), dat_s01_p0, sizeof(dat_s01_p0));
	unpack_rle((uint8_t *)(vmem+1), dat_s01_p1, sizeof(dat_s01_p1));
	while(music_offs_a < 16*4) {
		chequer_update_plain(4, 1);
		vwait(1);
	}

	VID_PAL0[0+1] = 0x0777;
	VID_PAL0[8+1] = 0x0777;
	VID_PAL0[0+2] = 0x0000;
	VID_PAL0[8+2] = 0x0000;
	VID_PAL0[0+3] = 0x0010;
	VID_PAL0[8+3] = 0x0022;

	while(music_offs_a < 16*5) {
		chequer_update_plain(4, 1);
		vwait(1);
	}

	uint8_t soffs = 0;
	while(music_offs_a < 16*24) {
		chequer_update_plain(
			(sin_tab[(soffs+0x40)&0xFF]+2)>>4,
			-((sin_tab[soffs]+2)>>4));
		vwait(1);
		soffs++;
	}
}

void _start(void)
{
	vmem = (uint16_t *)((0xFF+(uintptr_t)vmem_base)&~0xFF);

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

	for(int i = 0; i < 4*40*200; i+=4) {
		*(uint64_t *)(uint16_t *)&vmem[i] = 0;
	}

	intro_text();
	plain_chequer();

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
