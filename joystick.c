#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <inttypes.h>

#include <linux/joystick.h>

#define JOY_DEVICE "/dev/input/js0"

int main() {

	int joy_fd, num_of_axis = 0, num_of_buttons = 0;

	int16_t *axis   = NULL;
	int8_t  *button = NULL;

	//char    *name_of_joystick[80];

	struct js_event js;

	if ((joy_fd = open(JOY_DEVICE, O_RDONLY)) == -1) {
		fprintf(stderr, "Couldn't open joystick\n");
	}

	ioctl(joy_fd, JSIOCGAXES,     &num_of_axis);
	ioctl(joy_fd, JSIOCGBUTTONS,  &num_of_buttons);
//	ioctl(joy_fd, JSIOCGNAME(80), &name_of_joystick);

	axis   = (int16_t *) calloc(num_of_axis,   sizeof(int16_t));
	button = (int8_t *) calloc(num_of_buttons, sizeof(int8_t));

	printf("%d axis\n\t%d buttons\n\n",
//			name_of_joystick,
			num_of_axis,
			num_of_buttons);

	fcntl(joy_fd, F_SETFL, O_NONBLOCK);

	int8_t dir;
	int8_t dir_old = 0;
	int8_t acc;
	int8_t acc_old = 0;



	while (1) {
//		read(joy_fd, &js, sizeof(struct js_event));
		read(joy_fd, &js, sizeof(js));

		switch (js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				break;
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;

		}

		dir = axis[0] >> 8;
		acc = axis[1] >> 8;


		if ((acc != acc_old)) {
			acc_old = acc;

			//putchar('_');
			//putchar('S');
			if (acc < -125)
			  acc = -125;
			if (acc == 0xAA)
				++acc;


			//printf("_S");

			fputs("_S", stdout);
			putc(acc, stdout);

			fflush(stdout);
			fprintf(stderr, "0x%" PRIx8 "\n", acc);
		}
		 else {
			fputs("_S",stdout);
			putc(acc_old < -125 ? -125 : acc_old, stdout);
			fflush(stdout);
		}

		if ((dir != dir_old) ) {

			dir_old = dir;

//			putchar('_');
//			putchar('D');
			if (dir < -125)
			  dir=-125;
			if (dir == 0xAA)
				++dir;


			fputs( "_D", stdout);

			putc(dir, stdout);
			
			fflush(stdout);
		} 
		 else {

			fputs("_D", stdout);
			putc(dir_old < -125 ? -125 : dir_old, stdout);
			fflush(stdout);
		}




		
		//fprintf(stderr, "X: %6d Y: %6d\n", axis[0] >> 8, axis[1] >> 8);

	}

	close(joy_fd);
	return 0;
}

/* vim: set sts=0: */
