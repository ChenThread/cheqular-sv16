uint16_t coffs_gswap_x0 = 0;
uint16_t coffs_gswap_x1 = 0;
uint16_t coffs_gswap_y0 = 0;
uint16_t coffs_gswap_y1 = 0;

void chequer_sync_gswap_from_plain(void)
{
	coffs_gswap_x0 = coffs_plain_x;
	coffs_gswap_x1 = coffs_plain_x+16;
	coffs_gswap_y0 = coffs_plain_y;
	coffs_gswap_y1 = coffs_plain_y+16;
}

void chequer_update_gswap_x(int16_t mvx0, int16_t mvx1, int16_t mvy)
{
	uint16_t old_offs_x0 = coffs_gswap_x0;
	coffs_gswap_x0 += mvx0;
	uint16_t new_offs_x0 = coffs_gswap_x0;
	uint16_t old_offs_x1 = coffs_gswap_x1;
	coffs_gswap_x1 += mvx1;
	uint16_t new_offs_x1 = coffs_gswap_x1;
	uint16_t old_offs_y = coffs_plain_y;
	coffs_plain_y += mvy;
	uint16_t new_offs_y = coffs_plain_y;

	uint16_t *p = vmem+3;

	// X scroll
	uint16_t r0 = chequer_rotates[new_offs_x0&31][0];
	uint16_t r1 = chequer_rotates[new_offs_x1&31][0];
	uint16_t rd = r0^r1;
	uint16_t r = (((new_offs_y)&16) == 0 ? r0 : r1);

	// Y scroll
	uint16_t gflipy = ((new_offs_y)&15);

	for(int16_t y = 0; y < 200; y += 1) {
		if(y == gflipy) {
			r ^= rd;
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

void chequer_update_gswap_y(int16_t mvx, int16_t mvy0, int16_t mvy1)
{
	uint16_t old_offs_x = coffs_plain_x;
	coffs_plain_x += mvx;
	uint16_t new_offs_x = coffs_plain_x;
	uint16_t old_offs_y = coffs_gswap_y0;
	coffs_gswap_y0 += mvy0;
	uint16_t new_offs_y0 = coffs_gswap_y0;
	uint16_t old_offs_y1 = coffs_gswap_y1;
	coffs_gswap_y1 += mvy1;
	uint16_t new_offs_y1 = coffs_gswap_y1;

	uint16_t *p = vmem+3;

	// X scroll
	uint16_t r0 = chequer_rotates[new_offs_x&31][0];
	uint16_t r1 = ~r0;
	uint16_t rm0 = r0;
	uint16_t rm1 = r1;
	if(((new_offs_y0)&16) != 0) {
		r0 ^= rm0;
		r1 ^= rm1;
	}
	if(((new_offs_y1)&16) == 0) {
		r0 ^= rm1;
		r1 ^= rm0;
	}

	// Y scroll
	uint16_t gflipy0 = ((new_offs_y0)&15);
	uint16_t gflipy1 = ((new_offs_y1)&15);

	for(int16_t y = 0; y < 200; y += 1) {
		if(y == gflipy0) {
			r0 ^= rm0;
			r1 ^= rm1;
			gflipy0 += 16;
		}
		if(y == gflipy1) {
			r0 ^= rm1;
			r1 ^= rm0;
			gflipy1 += 16;
		}
		uint16_t ra = r0;
		uint16_t rb = r1;

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

