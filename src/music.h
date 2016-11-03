const uint16_t music_note_periods[] = {
	3822, 3608, 3405, 3214, 3034, 2863,
	2703, 2551, 2408, 2273, 2145, 2025,
};

volatile uint8_t music_playing = 0;
int16_t music_detune = 0;
uint16_t music_offs_a = 0*4*16;
uint16_t music_offs_c = 0*4*16;
uint16_t music_offs_d = 0*4*16;
uint8_t music_time_wait = 0;
uint8_t music_vol_a = 0;
uint8_t music_vol_b = 0;
uint8_t music_vol_c = 0;
uint8_t music_prev_c = 0;
void music_update(void)
{
	if(!music_playing) {
		return;
	}

	//music_detune -= 2; 

	if(music_time_wait > 0) {
		music_time_wait--;
		if(music_vol_a > 0) {
			music_vol_a--;
			if(music_vol_a < 15) {
				PSG_REG = 0x08; PSG_DAT_W = music_vol_a;
			}
		}
		if(music_vol_b > 0) {
			music_vol_b--;
			if(music_vol_b < 15) {
				PSG_REG = 0x09; PSG_DAT_W = music_vol_b;
			}
		}
		if(music_vol_c > 12*2) {
			music_vol_c--;
			//PSG_REG = 0x0A; PSG_DAT_W = 0x10|(music_vol_c>>1);
		}
		return;
	}
	music_time_wait = 8;

	// Get new notes
	uint8_t na = mdat_pat_a[mdat_ord_a[music_offs_a>>4]][music_offs_a&15];
	uint8_t nc = mdat_pat_c[mdat_ord_c[music_offs_c>>4]][music_offs_c&15];
	uint8_t nd = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
	uint8_t outmask = 0x3F;

	if(music_detune != 0) {
		nc = 0;
		//nd = 0;
		music_time_wait += (-music_detune+(1<<8))>>9;
	}

	// Force a pop
	PSG_REG = 0x08; PSG_DAT_W = 0x0F;

	if(na != 0) {
		uint16_t pa0 = music_note_periods[na&0xF];
		pa0 >>= (na>>4);
		uint16_t pa = pa0;
		if(music_detune != 0) {
			na = (na&0xF) + (na>>4)*12;
			na += music_detune>>8;
			if(na < 0x01 || na >= 0x80) {
				na = 1;
			}
			na = (na%12) + ((na/12)<<4);
			pa = pa0 = music_note_periods[na&0xF];
			pa0 >>= (na>>4);
			if(na != 1) {
				uint8_t na1 = na+1;
				if((na1&0xF) >= 0xC) {
					na1 += 4;
				}
				uint16_t pa1 = music_note_periods[na1&0xF];
				pa1 >>= (na1>>4);
				int32_t pa0_base = pa0<<8;
				int32_t pa1_base = pa1-pa0;
				pa1_base *= (music_detune&0xFF);
				int32_t pa_base = pa0_base + pa1_base;
				pa_base += 128;
				pa_base >>= 8;
				pa = pa_base;
				if(pa > 0xFFF) {
					pa = 0xFFF;
				}
			}
		}
		PSG_REG = 0x00; PSG_DAT_W = pa;
		PSG_REG = 0x01; PSG_DAT_W = pa>>8;
		outmask &= ~0x01;
		if(nc == 0) {
			pa -= 5;
			PSG_REG = 0x02; PSG_DAT_W = pa;
			PSG_REG = 0x03; PSG_DAT_W = pa>>8;
			music_vol_b = 0x0F+1;
			PSG_REG = 0x09; PSG_DAT_W = 0x0F;
			outmask &= ~0x02;
		}
	}

	if(nc != 0) {
		uint16_t pc = music_note_periods[nc&0xF];
		pc >>= (nc>>4);
		pc >>= 1;
		PSG_REG = 0x04; PSG_DAT_W = pc;
		PSG_REG = 0x05; PSG_DAT_W = pc>>8;

		if(music_prev_c != nc) {
			PSG_REG = 0x0A; PSG_DAT_W = 0x1F;
			music_vol_c = 0x0F*2;
			PSG_REG = 0x02; PSG_DAT_W = pc;
			PSG_REG = 0x03; PSG_DAT_W = pc>>8;
			music_vol_b = 0x0F;
			PSG_REG = 0x09; PSG_DAT_W = 0x0F;
		}
		pc += 2;
		pc >>= 2;
		PSG_REG = 0x0B; PSG_DAT_W = pc;
		PSG_REG = 0x0C; PSG_DAT_W = pc>>8;
		PSG_REG = 0x0D; PSG_DAT_W = 0x08;

		outmask &= ~0x06;
	} else {
		PSG_REG = 0x0A; PSG_DAT_W = 0x00;
	}
	music_prev_c = nc;

	if(nd != 0) {
		PSG_REG = 0x06; PSG_DAT_W = mdat_pat_d[mdat_ord_d[music_offs_d>>4]][music_offs_d&15];
		outmask &= ~0x08;
	}

	if(na != 0 || nd != 0) {
		music_vol_a = 0x0F+2;
		PSG_REG = 0x08; PSG_DAT_W = 0x0F;
	}

	PSG_REG = 0x07; PSG_DAT_W = outmask;

	music_offs_a++;
	music_offs_a %= sizeof(mdat_ord_a)*16;
	music_offs_c++;
	music_offs_c %= sizeof(mdat_ord_c)*16;
	music_offs_d++;
	music_offs_d %= sizeof(mdat_ord_d)*16;
}

