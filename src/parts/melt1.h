
uint8_t melt1_yacc[80];
uint8_t melt1_ytacc[80];
uint16_t melt1_masks[20];
uint16_t melt1_amt = 80*50;

uint16_t melt1_lfsr = 0x8000;

uint16_t melt1_random(void)
{
	if((melt1_lfsr&1) != 0) {
		melt1_lfsr >>= 1;
		melt1_lfsr ^= 0x9000;
	} else {
		melt1_lfsr >>= 1;
	}

	uint16_t a;
	uint16_t l = melt1_lfsr;
	a = (l+11)&15;
	if(a != 0) { l = (l<<a)|(l>>(16-a)); }

	return l;
}

void melt1_calc(void)
{
	// Clear temp accumulator
	memset(melt1_ytacc, 0, sizeof(melt1_ytacc));

	// Clear masks
	memset(melt1_masks, 0, sizeof(melt1_masks));

	const uint16_t mcount = 40;
	for(uint16_t i = 0; i < mcount && melt1_amt > 0; i++) {
		uint16_t x = (melt1_random() % 80);
		uint16_t lx = x;
		while(melt1_yacc[x] >= 50 || melt1_ytacc[x] != 0) {
			x++;
			if(x >= 80) {
				x = 0;
			}

			// this happens near the end of the melt sequence
			if(x == lx) {
				return;
			}
		}

		melt1_masks[x>>2] |= (0xF000>>((x<<2)&15));
		melt1_ytacc[x]++;
		melt1_yacc[x]++;
		melt1_amt--;
	}
}

void melt1_apply(void)
{
	uint16_t *p = vmem+80*(200-1);
	p += 3;
	for(uint16_t y = 0; y < 200; y++) {

		for(uint16_t x = 0, x4 = 0; x < 20; x++, x4 += 4) {
			uint16_t m = melt1_masks[x];
			uint16_t v0 = p[x4-80*0];
			uint16_t v1 = p[x4-80*4];
			p[x4] = v0^((v1^v0)&m);
		}

		p -= 80;
	}
}

void melt1_main(void)
{
	memset(melt1_yacc, 0, sizeof(melt1_yacc));

	// Melt
	while(melt1_amt > 0) {
		gswap1_update_text();
		melt1_calc();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		melt1_apply();
	}

	// TODO!
	VID_PAL0[0] = 0x0200;
	while(music_offs_a < 16*4*11) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
	}
}
