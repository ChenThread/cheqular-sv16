// [amount_left][old_offset]
uint16_t chequer_rotates[32][16];
uint16_t coffs_plain_x = 0;
uint16_t coffs_plain_y = 0;

void chequer_update_plain(int16_t mvx, int16_t mvy)
{
	// X scroll
	uint16_t old_offs_x = coffs_plain_x;
	coffs_plain_x += mvx;
	uint16_t new_offs_x = coffs_plain_x;
	uint16_t old_offs_y = coffs_plain_y;
	coffs_plain_y += mvy;
	uint16_t new_offs_y = coffs_plain_y;
	uint16_t r = chequer_rotates[mvx&31][old_offs_x&15];
	uint16_t *p = vmem+3;

	// Y scroll
	uint16_t flipy0 = 999;
	uint16_t flipy1 = 999;
	uint16_t flipyi = 0;
	if(old_offs_y != new_offs_y) {
		if(mvy < 0) {
			flipy0 = new_offs_y&15;
			flipy1 = old_offs_y&15;
		} else {
			flipy0 = old_offs_y&15;
			flipy1 = new_offs_y&15;
		}
		if(flipy0 > flipy1) {
			r ^= 0xFFFF;
			flipy0 ^= flipy1;
			flipy1 ^= flipy0;
			flipy0 ^= flipy1;
		}
	}
	uint16_t flipyd = flipy0^flipy1;

	for(int16_t y = 0; y < 200; y += 1) {
		if(y == flipy0) {
			r = ~r;
			flipy0 ^= flipyd;
			flipy0 += flipyi;
			flipyi ^= 16;
		}

		p[ 0*4] ^= r;
		p[ 1*4] ^= r;
		p[ 2*4] ^= r;
		p[ 3*4] ^= r;
		p[ 4*4] ^= r;
		p[ 5*4] ^= r;
		p[ 6*4] ^= r;
		p[ 7*4] ^= r;
		p[ 8*4] ^= r;
		p[ 9*4] ^= r;
		p[10*4] ^= r;
		p[11*4] ^= r;
		p[12*4] ^= r;
		p[13*4] ^= r;
		p[14*4] ^= r;
		p[15*4] ^= r;
		p[16*4] ^= r;
		p[17*4] ^= r;
		p[18*4] ^= r;
		p[19*4] ^= r;
		p += 80;
	}
}

