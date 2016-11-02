const int16_t introtext_dirs[][2] = {
	{-4, 0},
	{ 0, 3},
	{ 3,-2},
};

char const*const introtext_list[] = {
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
uint16_t introtext_chequer_idx = 0;

void introtext_vwait(int wait_counter)
{
	for(int i = 0; i < wait_counter; i++) {
		// Update chequerboard
		chequer_update_plain(
			introtext_dirs[introtext_chequer_idx][0],
			introtext_dirs[introtext_chequer_idx][1]);

		// Wait for vblank
		vwait(1);
	}
}

void introtext_main(void)
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
		char const* char_ptr = introtext_list[line_idx];

		if(char_ptr == NULL) {
			//line_subidx = 0;
			line_idx++;

			introtext_vwait(90);

			// Clear text plane
			for(int i = 0; i < 160*200; i+=4) {
				vmem[i] = 0;
			}

			// End section if at end of texts
			if(introtext_list[line_idx] == NULL) {
				break;
			}

			// Change direction
			introtext_chequer_idx++;

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
			introtext_vwait(2);
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

