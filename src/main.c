#include "ST.h"

#include "mdat.h"
#include "font.h"

extern uint8_t end[];
extern volatile uint32_t vbl_counter;
uint32_t vbl_expected_counter;

uint16_t old_pal[16];
uint8_t old_mode;
uint8_t *old_vmem = NULL;

uint8_t vmem_base[256*2+8*40*200];
uint16_t *vmem;

const uint16_t music_note_periods[] = {
	3822, 3608, 3405, 3214, 3034, 2863,
	2703, 2551, 2408, 2273, 2145, 2025,
};

volatile uint8_t music_playing = 0;
uint8_t music_offs_a = 0;
uint8_t music_offs_d = 0;
uint8_t music_time_wait = 0;
void music_update(void)
{
	if(!music_playing) {
		return;
	}

	if(music_time_wait > 0) {
		music_time_wait--;
		return;
	}
	music_time_wait = 8;

	// Get new notes
	const uint16_t eper = 125000*8/(50*8);
	uint8_t na = mdat_pat_a[mdat_ord_a[music_offs_a>>4]][music_offs_a&15];
	uint8_t nd = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
	uint8_t outmask = 0x3F;

	if(na != 0) {
		uint16_t pa = music_note_periods[na&0xF];
		pa >>= (na>>4);
		PSG_REG = 0x00; PSG_DAT_W = pa;
		PSG_REG = 0x01; PSG_DAT_W = pa>>8;
		pa -= 5;
		PSG_REG = 0x02; PSG_DAT_W = pa;
		PSG_REG = 0x03; PSG_DAT_W = pa>>8;
		PSG_REG = 0x09; PSG_DAT_W = 0x1F;
		PSG_REG = 0x07; PSG_DAT_W = 0x34;
		outmask &= ~0x03;
	}

	if(nd != 0) {
		PSG_REG = 0x06; PSG_DAT_W = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
		outmask &= ~0x08;
	}

	if(na != 0 || nd != 0) {
		PSG_REG = 0x08; PSG_DAT_W = 0x1F;
	}

	PSG_REG = 0x0B; PSG_DAT_W = eper;
	PSG_REG = 0x0C; PSG_DAT_W = eper>>8;
	PSG_REG = 0x0D; PSG_DAT_W = 0x00;
	PSG_REG = 0x07; PSG_DAT_W = outmask;

	music_offs_a++;
	music_offs_a %= sizeof(mdat_ord_a)*16;
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

char const*const intro_texts[] = {
	"chen thread",
	"presents",
	NULL,
	"a rushed demo",
	"for",
	"sillyventure"
	"2016",
	NULL,
	"which asie",
	"probably never",
	"wanted me to",
	"write",
	NULL,
	NULL,
};

// [amount_left][old_offset]
uint16_t chequer_rotates[17][16];

void chequer_init()
{
	for(int i = 0; i < 17; i++) {
		uint16_t v = (1<<i)-1;
		for(int j = 0; j < 16; j++) {
			chequer_rotates[i][j] = v;
			v = (v<<1)|(v>>15);
		}
	}
}

uint16_t coffs_plain_x = 0;
uint16_t coffs_plain_y = 0;
void intro_text_vwait(int wait_counter)
{
	for(int i = 0; i < wait_counter; i++) {
		// Update chequerboard
		// X scroll
		int16_t mvx = -3;
		uint16_t old_offs_x = coffs_plain_x;
		coffs_plain_x += mvx;
		uint16_t new_offs_x = coffs_plain_x;
		uint16_t r = (mvx >= 0
			? chequer_rotates[mvx][old_offs_x&15]
			: chequer_rotates[-mvx][new_offs_x&15]
		);
		for(int x = 1; x < 80*200; x += 4) {
			vmem[x] ^= r;
		}

		// Wait for vblank
		vwait(1);
	}
}

void intro_text(void)
{
	//
	// Fill background with chequerboard
	for(int y = 0; y < 200; y+=32) {
		uint16_t *p = &vmem[80*y];
		for(int x = 0; x < 80*16; x+=8) {
			p[x+1] = 0xFFFF;
			p[x+5] = 0x0000;
			p[x+1+80*16] = 0x0000;
			p[x+5+80*16] = 0xFFFF;
		}
	}

	//
	VID_PAL0[1] = 0x0777;
	VID_PAL0[2] = 0x0001;
	VID_PAL0[3] = 0x0777;
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

			continue;
		}

		int slen = strlen(char_ptr);
		int slen2 = slen/2;
		screen_ptr = (uint8_t *)vmem;
		screen_ptr += 20*8*(8*(2*line_subidx+1));
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
	VID_PAL0[1] = 0x0777;
	vwait_reset();
	vwait(1);
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
	VID_SHIFT_MODE_ST = 0x00;
	VID_BASE_H = ((uint32_t)vmem)>>16;
	VID_BASE_M = ((uint32_t)vmem)>>8;
	VID_STRIDE_STE = 0;

	for(int i = 0; i < 4*40*200; i+=4) {
		*(uint64_t *)(uint16_t *)&vmem[i] = 0;
	}

	intro_text();
	plain_chequer();

	for(;;) {}

	// Restore palette
	vwait_force();
	for(int i = 0; i < 16; i++) {
		VID_PAL0[i] = old_pal[i];
	}
}
