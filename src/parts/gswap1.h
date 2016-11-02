void gswap1_main(void)
{
	uint8_t soffs = title_soffs;

	// X first
	chequer_sync_gswap_from_plain();
	while(music_offs_a < 16*10-8 || ((coffs_gswap_x0^coffs_gswap_x1)&31) != 16) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		chequer_update_gswap_x(mvx-1, mvx+1, mvy);
		vwait(1);
		soffs++;
	}
	coffs_plain_x = coffs_gswap_x0;
	while(music_offs_a < 16*10) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
	}

	// Y second
	chequer_sync_gswap_from_plain();
	while(music_offs_a < 16*12-8 || ((coffs_gswap_y0^coffs_gswap_y1)&15) != 0) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		chequer_update_gswap_y(mvx, mvy-1, mvy+1);
		vwait(1);
		soffs++;
	}
	coffs_plain_y = coffs_gswap_y0;
	while(music_offs_a < 16*12) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
	}

	// Rotations
	int16_t side = 1;
	while(music_offs_a < 16*16-8) {
		chequer_sync_gswap_from_plain();
		do {
			int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
			int16_t mvy = -((sin_tab[soffs]+2)>>4);
			chequer_update_gswap_x(mvx-side, mvx+side, mvy);
			vwait(1);
			soffs++;
		} while(((coffs_gswap_x0^coffs_gswap_x1)&31) != 16);
		coffs_plain_x = coffs_gswap_x0;
		side = -side;
		chequer_sync_gswap_from_plain();
		do {
			int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
			int16_t mvy = -((sin_tab[soffs]+2)>>4);
			chequer_update_gswap_y(mvx, mvy-side, mvy+side);
			vwait(1);
			soffs++;
		} while(((coffs_gswap_y0^coffs_gswap_y1)&15) != 0);
		coffs_plain_y = coffs_gswap_y0;
	}

	while(music_offs_a < 16*4*11) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
	}
}

