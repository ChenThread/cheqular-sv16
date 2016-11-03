uint16_t sun1_planes_base[256+4*40*100+64];
uint16_t *sun1_planes;

uint16_t sun1_cheqmask[100][16];
uint16_t sun1_cheqwidth_long[100];
uint16_t sun1_cheqwidth[100];
uint16_t sun1_cheqstep[100];
uint16_t sun1_cheqoffs[100];
uint16_t sun1_cheqctr = 0;

void sun1_update_scroll(void)
{
	// TODO!

	uint16_t *p = vmem + 20*4*100;
	p += 3;

	for(uint16_t y = 0; y < 100; y++) {
		uint16_t offs = sun1_cheqoffs[y]>>8;
		uint16_t step = sun1_cheqstep[y];
		uint16_t width = sun1_cheqwidth[y];

		for(uint16_t x = 0; x < 20*4; x += 4*5) {
			p[x+0*4] = sun1_cheqmask[y][offs];
			offs += step;
			if(offs >= width) { offs -= width; }
			p[x+1*4] = sun1_cheqmask[y][offs];
			offs += step;
			if(offs >= width) { offs -= width; }
			p[x+2*4] = sun1_cheqmask[y][offs];
			offs += step;
			if(offs >= width) { offs -= width; }
			p[x+3*4] = sun1_cheqmask[y][offs];
			offs += step;
			if(offs >= width) { offs -= width; }
			p[x+4*4] = sun1_cheqmask[y][offs];
			offs += step;
			if(offs >= width) { offs -= width; }
		}

		p += 20*4;
	}

	for(uint16_t y = 0; y < 100; y++) {
		sun1_cheqoffs[y] += sun1_cheqwidth_long[y];
		uint16_t width = sun1_cheqwidth[y]<<8;
		while(sun1_cheqoffs[y] >= width) {
			sun1_cheqoffs[y] -= width;
		}
	}

	sun1_cheqctr++;
	sun1_cheqctr &= 0xFF;
}

void sun1_main(void)
{
	sun1_planes = (uint16_t *)((0xFF+(uintptr_t)sun1_planes_base)&~0xFF);

	// Set palette appropriately
	for(uint16_t i = 0; i < 16; i+=2) {
		VID_PAL0[i+0] = 0x0000;
		VID_PAL0[i+1] = 0x0FFF;
	}

	vwait_reset();
	vwait(1);

	gswap1_update_text();
	vwait(1);

	// Clear two planes - top half
	for(uint16_t i = 0; i < 20*4*100; i += 4) {
		vmem[i+1] = 0x0000;
		vmem[i+2] = 0x0000;
		vmem[i+3] = 0x0000;
	}

	gswap1_update_text();
	vwait(1);

	// Set second half of two planes to %101
	for(uint16_t i = 20*4*100; i < 20*4*200; i += 4) {
		vmem[i+1] = 0xFFFF;
		vmem[i+2] = 0x0000;
		vmem[i+3] = 0xFFFF;
	}

	gswap1_update_text();
	vwait(1);

	// Load sun plane 0
	unpack_rle((uint8_t *)(sun1_planes+1), dat_s02_p0, sizeof(dat_s02_p0));

	gswap1_update_text();
	vwait(1);

	// Load sun plane 1
	unpack_rle((uint8_t *)(sun1_planes+2), dat_s02_p1, sizeof(dat_s02_p1));

	gswap1_update_text();
	vwait_reset();
	vwait(1);

	gswap1_update_text();
	vwait(1);

	// Palette fade
	for(uint16_t i = 0; i < 30; i++) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		uint16_t sv1 = i>>1;
		uint16_t sv2 = (i+1)>>2;
		sv1 = ((sv1&1)<<3)|(sv1>>1);
		sv2 = ((sv2&1)<<3)|(sv2>>1);
		VID_PAL0[0] = sv2*0x0001;
		VID_PAL0[4] = sv1*0x0110;
		VID_PAL0[6] = sv1*0x0100;
		VID_PAL0[4+1] = 0x0FFF^(sv1*0x0111);
	}

	// Raise sun
	for(uint16_t i = 0; i < 100; i++) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		uint16_t *p = vmem+(99-i)*20*4;
		for(uint16_t j = 0; j < (i+1)*20*4; j+=4*5) {
			p[j+1+4*0] = sun1_planes[j+1+4*0];
			p[j+2+4*0] = sun1_planes[j+2+4*0];
			p[j+1+4*1] = sun1_planes[j+1+4*1];
			p[j+2+4*1] = sun1_planes[j+2+4*1];
			p[j+1+4*2] = sun1_planes[j+1+4*2];
			p[j+2+4*2] = sun1_planes[j+2+4*2];
			p[j+1+4*3] = sun1_planes[j+1+4*3];
			p[j+2+4*3] = sun1_planes[j+2+4*3];
			p[j+1+4*4] = sun1_planes[j+1+4*4];
			p[j+2+4*4] = sun1_planes[j+2+4*4];
		}
	}

	// Generate chequerboard
	// TODO!
	for(uint16_t i = 0; i < 100; i++) {
		if((i&3) == 0) {
			gswap1_update_text();
			vwait(1);
			if(vbl_key_pressed != 0) { return; }
		}

		// Calculate texture width
		uint16_t tsubstep = (i+40)/12;
		uint16_t tsubstep_long = ((i+40)<<4)/12;
		sun1_cheqwidth[i] = tsubstep*2;
		sun1_cheqwidth_long[i] = tsubstep_long*2;

		// Get step
		sun1_cheqstep[i] = 16%(tsubstep*2);
		uint16_t longstep = (16<<4)%(tsubstep_long*2);

		// Get offset
		//sun1_cheqoffs[i] = ((sun1_cheqstep[i]*10) % (tsubstep*2));
		//sun1_cheqoffs[i] = ((tsubstep*2)-sun1_cheqoffs[i]) % (tsubstep*2);
		//sun1_cheqoffs[i] <<= 8;
		//sun1_cheqoffs[i] += 128;

		sun1_cheqoffs[i] = ((longstep*10) % (tsubstep_long*2));
		sun1_cheqoffs[i] = ((tsubstep_long*2)-sun1_cheqoffs[i]) % (tsubstep_long*2);
		sun1_cheqoffs[i] <<= 4;
		sun1_cheqoffs[i] += 128;

		// Construct base mask
		uint32_t v = 0;
		uint16_t s = 0;
		uint16_t xs = 0;
		uint32_t vbit = 0x80000000;
		uint32_t vword = 0xFFFFFFFF;
		for(uint16_t j = 0; j < 32; j++) {
			v |= vbit&vword;
			vbit >>= 1;
			s += 1;
			if(s >= tsubstep) {
				vword = ~vword;
				s = 0;
			}
		}

		// Fill remaining masks
		for(uint16_t j = 0; j < 16; j++) {
			sun1_cheqmask[i][j] = v>>(16-j);
		}
	}

	// Palette fade #2
	for(uint16_t i = 0; i < 30; i++) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		uint16_t sv2 = (i+1)>>2;
		sv2 = ((sv2&1)<<3)|(sv2>>1);
		VID_PAL0[8+2] = sv2*0x0010;
		sun1_update_scroll();
	}
	VID_PAL0[8] = 0x0030;

	// Parallax chequerboard
	while(music_offs_a < 16*4*10-1) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		sun1_update_scroll();
	}

	// Finish off
	//while(music_offs_a < 16*4*12-1) {
	while(music_offs_a > 8) {
		music_detune -= 3;
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
		uint16_t b = melt1_random()&0x1FF;
		if(b < 12*16) {
			VID_PAL0[b&15] ^= (1<<(b>>4));
		}
		sun1_update_scroll();
	}
}

