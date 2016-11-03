uint16_t sun1_planes_base[256+4*40*100+64];
uint16_t *sun1_planes;

uint16_t sun1_cheqfield[100][32];

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
	}

	gswap1_update_text();
	vwait(1);

	// Set second half of two planes to %01
	for(uint16_t i = 20*4*100; i < 20*4*200; i += 4) {
		vmem[i+1] = 0xFFFF;
		vmem[i+2] = 0x0000;
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
		uint16_t sv1 = i>>1;
		uint16_t sv2 = (i+1)>>2;
		sv1 = ((sv1&1)<<3)|(sv1>>1);
		sv2 = ((sv2&1)<<3)|(sv2>>1);
		VID_PAL0[0] = sv2*0x0001;
		VID_PAL0[4] = sv1*0x0110;
		VID_PAL0[6] = sv1*0x0100;
		VID_PAL0[4+1] = 0x0FFF^(sv1*0x0111);
	}
	VID_PAL0[8] = 0x0030;

	// Generate chequerboard
	// TODO!
	for(uint16_t i = 0; i < 100; i++) {
		if((i&7) == 0) {
			gswap1_update_text();
			vwait(1);
		}

		//sun1_cheqfield
		//uint16_t
	}

	// Raise sun
	for(uint16_t i = 0; i < 100; i++) {
		gswap1_update_text();
		vwait(1);
		uint16_t *p = vmem+(99-i)*20*4;
		for(uint16_t j = 0; j < (i+1)*20*4; j+=4) {
			p[j+1] = sun1_planes[j+1];
			p[j+2] = sun1_planes[j+2];
		}
	}

	// Parallax chequerboard

	// TODO!
	//VID_PAL0[0] = 0x0200;
	while(music_offs_a < 16*4*11) {
		gswap1_update_text();
		vwait(1);
		if(vbl_key_pressed != 0) { return; }
	}
}

