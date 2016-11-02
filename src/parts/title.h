int16_t title_soffs = 0;

void title_main()
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
	while(music_offs_a < 16*7-8) {
		chequer_update_plain(
			(sin_tab[(soffs+0x40)&0xFF]+2)>>4,
			-((sin_tab[soffs]+2)>>4));
		vwait(1);
		soffs++;
	}

	int16_t title_line_dead = 0;
	while(music_offs_a < 16*8 || title_line_dead < 200) {
		chequer_update_plain(
			(sin_tab[(soffs+0x40)&0xFF]+2)>>4,
			-((sin_tab[soffs]+2)>>4));
		for(int16_t i = 0; i < 2; i++) {
			if(title_line_dead < 200) {
				int16_t ty_low = (title_line_dead/25);
				int16_t ty_high = (title_line_dead%25)<<3;

				// Bitflip ty_low
				ty_low = 0
					|((ty_low>>2)&1)
					|((ty_low   )&2)
					|((ty_low<<2)&4)
					;

				// Merge
				int16_t ty = ty_high|ty_low;

				// ASSERTION
				while(ty < 0 || ty >= 200) {}

				// Clear line
				uint16_t *p = &vmem[20*4*ty];
				for(int16_t x = 0; x < 20*4; x+=4) {
					p[x+0] = 0;
					p[x+1] = 0;
				}

				// Next line!
				title_line_dead++;
			}
		}
		vwait(1);
		soffs++;
	}

	title_soffs = soffs;
}

