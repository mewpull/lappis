#include "video.c"

#define ZOOM 1.0
#define ITERATIONS 20

int32_t mandel(double cr, double ci) {
	double zr = 0;
	double zi = 0;

	double zr1 = 0;
	double zi1 = 0;
	for (uint32_t i = 0; i < ITERATIONS; i++) {
		zr1 = zr*zr - zi*zi + cr;
		zi1 = zr*zi * 2.0 + ci;

		zr = zr1;
		zi = zi1;

		if (zr*zr + zi*zi > 4.0f) {
			return i;
		}
	}
	return -1;
}

void to_mandel_space(double x, double y, double *cr, double *ci) {
	*cr = 2.0/ZOOM * (2.0*(x / WIDTH) - 1);
	*ci = (WIDTH/HEIGHT)*2.0/ZOOM * (2.0*(y / HEIGHT) - 1);
}

void draw_mandel() {
	unsigned char black[3] = {0x00, 0x00, 0x00};
	double cr = 0;
	double ci = 0;
	for (unsigned int x = 0; x < WIDTH; x++) {
		for (unsigned int y = 0; y < HEIGHT; y++) {
			to_mandel_space(x, y, &cr, &ci);
			int32_t escapes_in = mandel(cr, ci);

			if (escapes_in == -1) {
				set_pixel(x, y, black);
			} else {
				unsigned char c[3] = {(256/ITERATIONS)*escapes_in, (256/ITERATIONS)*escapes_in, (256/ITERATIONS)*escapes_in};
				set_pixel(x, y, c);
			}
		}
	}
}