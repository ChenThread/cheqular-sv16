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
	while(music_offs_a < 16*4*11) {
		chequer_update_plain(
			(sin_tab[(soffs+0x40)&0xFF]+2)>>4,
			-((sin_tab[soffs]+2)>>4));
		vwait(1);
		soffs++;
	}
}

