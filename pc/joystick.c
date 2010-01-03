#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>

#include <inttypes.h>

#include <linux/joystick.h>

#define JOY_DEVICE "/dev/input/js0"


inline void tx_cmd(uint8_t cmd, uint8_t param) {

	/* PACKET START INDICATOR
	 *
	 * each data packet between PC and µC to start with 0xAA!
	 * 0xAA CANT be transferred with rfm12 (see rfm12's preamble's)
	 * => use it also to indacte data-packet-start
	 */
	putc(0xAA, stdout);

	// write data itself
	putc(cmd, stdout);
	putc(param, stdout);

	// ensure the data is transmitted NOW
	fflush(stdout);

}

int main() {

	int fd;
	uint8_t num_of_axis = 0, num_of_buttons = 0;
	struct js_event event;

	//int16_t *axis   = NULL;
	//int16_t *button = NULL;

	//char    *name_of_joystick[80];


	if ((fd = open(JOY_DEVICE, O_RDONLY)) == -1) {
		fprintf(stderr, "Couldn't open joystick\n");
		exit(-1);
	}

	ioctl(fd, JSIOCGAXES, &num_of_axis);

	if (num_of_axis < 2) {
		fprintf(stderr, "The joystick needs axis\n");
		exit(-2);
	}
	// ioctl(fd, JSIOCGBUTTONS,  &num_of_buttons);
	// ioctl(fd, JSIOCGNAME(80), &name_of_joystick);

	// axis   = (int16_t *) calloc(num_of_axis,   sizeof(int16_t));
	// button = (int8_t  *) calloc(num_of_buttons, sizeof(int8_t));

	//printf("%d axis\n\t%d buttons\n\n",
	//			name_of_joystick,
	//		num_of_axis,
	//		num_of_buttons);
	

	fcntl(fd, F_SETFL, O_NONBLOCK);

	int8_t dir;
	// int8_t dir_old = 0;
	int8_t acc;
	// int8_t acc_old = 0;



	while (1) {
		//read(fd, &js, sizeof(struct js_event));
		read(fd, &event, sizeof(event));
		/*
		switch (js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_AXIS:
				axis[js.number] = js.value;
				dir = axis[0] >> 8;

				break;
			case JS_EVENT_BUTTON:
				button[js.number] = js.value;
				break;

		}
		*/
		if (event.type & JS_EVENT_AXIS) {
			// x-direction = direction
			if (event.number == 0) {
				dir = (event.value >> 8) & 0xff;

				if (dir < -127)
				  dir = -127;
				else if (dir == 0xAA)
					++dir;

				tx_cmd('D', dir);

			}
			// y-direction = throttle
			else if (event.number == 1) {
				acc = (event.value >> 8) & 0xff;

				if (acc < -127)
				  acc = -127;
				else if (acc == 0xAA)
					++acc;

				tx_cmd('S', acc);
				
			}

		}

		/*
		//dir = axis[0] >> 8;
		//acc = axis[1] >> 8;


		if ((acc != acc_old)) {
			acc_old = acc;

			send('S', acc);

			fprintf(stderr, "0x%" PRIx8 "\n", acc);
			usleep(1);

		} 

		if ((dir != dir_old) ) {
			dir_old = dir;

			send('D', dir);
			usleep(1)
		} 
		
		//fprintf(stderr, "X: %6d Y: %6d\n", axis[0] >> 8, axis[1] >> 8);
		//usleep(1);
		// */

	}

	close(fd);
	return 0;
}

/* vim: set sts=0: */
