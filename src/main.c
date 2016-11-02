#include "ST.h"

extern uint8_t end[];
extern volatile uint32_t vbl_counter;
uint32_t vbl_expected_counter;

uint16_t old_pal[16];
uint8_t old_mode;
uint8_t *old_vmem = NULL;

uint8_t vmem_base[256+160*200];
uint16_t *vmem;

const uint16_t music_note_periods[] = {
	3822, 3608, 3405, 3214, 3034, 2863,
	2703, 2551, 2408, 2273, 2145, 2025,
};

const uint8_t mdat_ord_a[] = {
	0, 1, 0, 2,
};
const uint8_t mdat_ord_d[] = {
	0, 1, 0, 2,
};
const uint8_t mdat_pat_a[][16] = {
	{
		0x20, 0x20, 0x30,
		0x20, 0x20, 0x2A,
		0x20, 0x20, 0x28,
		0x00, 0x20, 0x27,
		0x00, 0x20, 0x25,
		0x00,
	},

	{
		0x20, 0x20, 0x25,
		0x20, 0x20, 0x23,
		0x20, 0x20, 0x25,
		0x00, 0x20, 0x26,
		0x00, 0x20, 0x27,
		0x00,
	},

	{
		0x20, 0x20, 0x25,
		0x20, 0x20, 0x23,
		0x20, 0x25,
		0x00, 0x25, 0x26,
		0x27, 0x23, 0x20,
		0x1A, 0x20,
	},
};

const uint8_t mdat_pat_d[][16] = {
	{
		0x1F, 0x03, 0x03, 0x1F,
		0x10, 0x03, 0x03, 0x1F,
		0x1F, 0x03, 0x1F, 0x03,
		0x10, 0x03, 0x03, 0x1F,
	},

	{
		0x1F, 0x07, 0x07, 0x1F,
		0x10, 0x07, 0x07, 0x1F,
		0x07, 0x10, 0x07, 0x1F,
		0x10, 0x07, 0x07, 0x10,
	},

	{
		0x1F, 0x07, 0x07, 0x1F,
		0x10, 0x07, 0x07, 0x1F,
		0x07, 0x10, 0x07, 0x1F,
		0x10, 0x10, 0x10, 0x10,
	},
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
	PSG_REG = 0x07; PSG_DAT_W = 0x3F;
	PSG_REG = 0x06; PSG_DAT_W = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
	PSG_REG = 0x0B; PSG_DAT_W = eper;
	PSG_REG = 0x0C; PSG_DAT_W = eper>>8;
	PSG_REG = 0x08; PSG_DAT_W = 0x1F;
	PSG_REG = 0x0D; PSG_DAT_W = 0x00;
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
	} else {
		PSG_REG = 0x07; PSG_DAT_W = 0x37;
	}
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

	// Set up vbl timing
	vwait_reset();
	vwait(1);

	music_playing = 1;

	// Perform fade
	for(int i = 1; i < 16; i++) {
		vwait(2);
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

	for(int i = 0; i < 4*40*200; i+=4) {
		*(uint64_t *)(uint16_t *)&vmem[i] = 0;
	}

	plain_chequer();

	for(;;) {}

	// Restore palette
	vwait_force();
	for(int i = 0; i < 16; i++) {
		VID_PAL0[i] = old_pal[i];
	}
}
