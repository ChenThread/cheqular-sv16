
char const*const gswap1_text[] = {
	"chen thread is:",
	"                    ",
	"greasemonkey/nz",
	"asie/pl",
	"          ",
	"we only make high quality demos",
	NULL,

	"  code: gm",
	" music: gm",
	"grafix: gm",
	NULL,

	"this is just a few silly routines",
	"i've been far as decided to mess",
	"around with because i have never",
	"written a proper ST demo before",
	" ",
	"so just sit back and sleep while",
	"you wait for the next demo",
	NULL,

	"greasemonkey greets:",
	"                    ",
	"botb - dma-sc - crtc - mudlord - 505",
	"booze - farbrausch - jakim - titan",
	"wudsn - censor - dhs - lamers",
	"checkpoint - 8bitbubsy - and many more",
	"who aren't watching this demo",
	//"----------------------------------------",
	NULL,

	"asie greets:",
	"                    ",
	"he wasn't involved in this demo,",
	"but if you can find him in the crowd,",
	"run up to him and start cheering",
	NULL,

	"asie currently wants to kill:",
	"                    ",
	"greasemonkey",
	NULL,

	"asm is for lamers,",
	"true sceners code their demos in C",
	"                    ",
	"but BASIC is still for lamers",
	NULL,

	"if i had more time to prepare,",
	"i would have given you a better",
	"3d floor that doesn't distort",
	"horribly like this one",
	NULL,

	"there's never a time to not honk",
	"                    ",
	"honks out for chen",
	"                    ",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	"HONK HONK HONK HONK HONK HONK HONK HONK",
	NULL,

	//"----------------------------------------",
	NULL,

	NULL,
	NULL,
};

char const* gswap1_tptr = NULL;
uint8_t *gswap1_screen_ptr = NULL;
int16_t gswap1_tidx = 0;
int16_t gswap1_ty = -1;
int16_t gswap1_text_wait = 20;

uint8_t gswap1_in_sun1 = 0;

void gswap1_update_text(void)
{
	if(--gswap1_text_wait > 0) {
		return;
	}
	if(gswap1_tptr == NULL) {
		gswap1_tptr = gswap1_text[gswap1_tidx++];
		if(gswap1_ty == -1) {
			if(gswap1_tptr == NULL) {
				// TODO: make this work properly
				gswap1_tidx--;
				gswap1_text_wait = 180;
				return;
			}

			// Clear text plane
			for(uint16_t i = 0; i < 80*200; i+=4) {
				vmem[i+0] = 0;
			}

			// Calculate size
			for(gswap1_ty = 0; gswap1_text[gswap1_tidx-1+gswap1_ty] != NULL; gswap1_ty++) {
				// pass
			}

			if(gswap1_in_sun1 != 0) {
				gswap1_ty = 100+2;
			} else {
				gswap1_ty = -gswap1_ty*6;
				gswap1_ty += 100-2;
			}

		} else {
			gswap1_ty += 12;
			if(gswap1_ty+6 > 200) {
				gswap1_ty = 0;
			}
		}
		if(gswap1_tptr == NULL) {
			gswap1_text_wait = 180;
			gswap1_ty = -1;
			return;
		}

		int slen = strlen(gswap1_tptr);
		int slen2 = slen/2;
		gswap1_screen_ptr = (uint8_t *)vmem;
		gswap1_screen_ptr += 20*8*(gswap1_ty);
		gswap1_screen_ptr += 20*4-(slen2&~1)*4+1-(slen2&1);
		gswap1_screen_ptr -= 8;
	}

	gswap1_text_wait = 2;
	uint16_t c = *(gswap1_tptr++) - 0x20;
	c <<= 3;
	gswap1_screen_ptr[0*80*2] = font_data[c+0];
	gswap1_screen_ptr[1*80*2] = font_data[c+1];
	gswap1_screen_ptr[2*80*2] = font_data[c+2];
	gswap1_screen_ptr[3*80*2] = font_data[c+3];
	gswap1_screen_ptr[4*80*2] = font_data[c+4];
	gswap1_screen_ptr[5*80*2] = font_data[c+5];
	gswap1_screen_ptr[6*80*2] = font_data[c+6];
	gswap1_screen_ptr[7*80*2] = font_data[c+7];
	if((((uintptr_t)gswap1_screen_ptr)&1) != 0) {
		gswap1_screen_ptr += 7;
	} else {
		gswap1_screen_ptr += 1;
	}

	if(gswap1_tptr[0] == '\x00') {
		gswap1_tptr = NULL;
	}
}

void gswap1_main(void)
{
	uint8_t soffs = title_soffs;

	// X first
	chequer_sync_gswap_from_plain();
	while(music_offs_a < 16*10-8 || ((coffs_gswap_x0^coffs_gswap_x1)&31) != 16) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		gswap1_update_text();
		chequer_update_gswap_x(mvx-1, mvx+1, mvy);
		vwait(1);
		soffs++;
		if(vbl_key_pressed != 0) { return; }
	}
	coffs_plain_x = coffs_gswap_x0;
	while(music_offs_a < 16*10) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		gswap1_update_text();
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
		if(vbl_key_pressed != 0) { return; }
	}

	// Y second
	chequer_sync_gswap_from_plain();
	while(music_offs_a < 16*12-8 || ((coffs_gswap_y0^coffs_gswap_y1)&31) != 16) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		gswap1_update_text();
		chequer_update_gswap_y(mvx, mvy-1, mvy+1);
		vwait(1);
		soffs++;
		if(vbl_key_pressed != 0) { return; }
	}
	coffs_plain_y = coffs_gswap_y0;
	while(music_offs_a < 16*12) {
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
		int16_t mvy = -((sin_tab[soffs]+2)>>4);
		gswap1_update_text();
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
		if(vbl_key_pressed != 0) { return; }
	}

	// Rotations
	int16_t side = 1;
	while(music_offs_a < 16*15-16-8) {
		chequer_sync_gswap_from_plain();
		do {
			int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
			int16_t mvy = -((sin_tab[soffs]+2)>>4);
			gswap1_update_text();
			chequer_update_gswap_x(mvx-side, mvx+side, mvy);
			vwait(1);
			soffs++;
			if(vbl_key_pressed != 0) { return; }
		} while(((coffs_gswap_x0^coffs_gswap_x1)&31) != 16);
		coffs_plain_x = coffs_gswap_x0;
		side = -side;
		chequer_sync_gswap_from_plain();
		do {
			int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]+2)>>4;
			int16_t mvy = -((sin_tab[soffs]+2)>>4);
			gswap1_update_text();
			chequer_update_gswap_y(mvx, mvy-side, mvy+side);
			vwait(1);
			soffs++;
			if(vbl_key_pressed != 0) { return; }
		} while(((coffs_gswap_y0^coffs_gswap_y1)&31) != 16);
		coffs_plain_y = coffs_gswap_y0;
	}

	// Slowdown
	while(music_offs_a < 16*15-8) {
		int16_t amp = 16*15-8-music_offs_a;
		int16_t mvx = (sin_tab[(soffs+0x40)&0xFF]);
		int16_t mvy = -((sin_tab[soffs]));
		mvx *= amp;
		mvy *= amp;
		mvx += 128;
		mvy += 128;
		mvx >>= 8;
		mvy >>= 8;
		gswap1_update_text();
		chequer_update_plain(mvx, mvy);
		vwait(1);
		soffs++;
		if(vbl_key_pressed != 0) { return; }
	}
}

