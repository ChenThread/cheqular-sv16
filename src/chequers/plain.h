// [amount_left][old_offset]
uint16_t chequer_rotates[32][16];
uint16_t coffs_plain_x = 0;
uint16_t coffs_plain_y = 0;

void chequer_update_plain(int16_t mvx, int16_t mvy)
{
	uint16_t old_offs_x = coffs_plain_x;
	coffs_plain_x += mvx;
	uint16_t new_offs_x = coffs_plain_x;
	uint16_t old_offs_y = coffs_plain_y;
	coffs_plain_y += mvy;
	uint16_t new_offs_y = coffs_plain_y;

	uint16_t *p = vmem+3;

	// X scroll
	uint16_t r = chequer_rotates[new_offs_x&31][0];
	if(((new_offs_y)&16) != 0) {
		r = ~r;
	}

	// Y scroll
	uint16_t gflipy = ((new_offs_y)&15);

	for(int16_t y = 0; y < 200; y += 1) {
		if(y == gflipy) {
			r = ~r;
			gflipy += 16;
		}
		uint16_t ra = r;
		uint16_t rb = ~r;

		p[ 0*4] = ra;
		p[ 1*4] = rb;
		p[ 2*4] = ra;
		p[ 3*4] = rb;
		p[ 4*4] = ra;
		p[ 5*4] = rb;
		p[ 6*4] = ra;
		p[ 7*4] = rb;
		p[ 8*4] = ra;
		p[ 9*4] = rb;
		p[10*4] = ra;
		p[11*4] = rb;
		p[12*4] = ra;
		p[13*4] = rb;
		p[14*4] = ra;
		p[15*4] = rb;
		p[16*4] = ra;
		p[17*4] = rb;
		p[18*4] = ra;
		p[19*4] = rb;
		p += 80;
	}
}

