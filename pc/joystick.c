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


void tx_cmd(uint8_t cmd, uint8_t param) {

	/* PACKET START INDICATOR
	 *
	 * each data packet between PC and µC to start with 0xAA!
	 * 0xAA CANT be transferred with rfm12 (see rfm12's preamble's)
	 * => use it also to indacte data-packet-start
	 */
	putc(0xAA, stdout);
	usleep(1);

	// write data itself
	putc(cmd, stdout);
	usleep(1);
	putc(param, stdout);
	usleep(1);

	// ensure the data is transmitted NOW
	fflush(stdout);
	// debug output
	fprintf(stderr, "%c: %x\n", cmd, param);
	usleep(5);
}

int main() {

	int fd;
	uint8_t num_of_axis = 0;
	struct js_event event;

	if ((fd = open(JOY_DEVICE, O_RDONLY)) == -1) {
		fprintf(stderr, "Couldn't open joystick\n");
		exit(-1);
	}

	ioctl(fd, JSIOCGAXES, &num_of_axis);

	if (num_of_axis < 2) {
		fprintf(stderr, "The joystick needs axis\n");
		exit(-2);
	}

	fcntl(fd, F_SETFL, O_NONBLOCK);

	int8_t dir;
	int8_t old_dir = 0;
	int8_t acc;
	int8_t old_acc = 0;

	while (1) {
		read(fd, &event, sizeof(event));
		if (event.type & JS_EVENT_AXIS) {
			// x-direction = direction
			if (event.number == 0) {
				dir = (event.value >> 8) & 0xff;

				if (dir < -127)
				  dir = -127;
				else if (dir == 0xAA)
					++dir;

				if (old_dir != dir)
					tx_cmd('D', dir);
				old_dir = dir;


			}
			// y-direction = throttle
			else if (event.number == 1) {
				acc = (event.value >> 8) & 0xff;

				if (acc < -127)
				  acc = -127;
				else if (acc == 0xAA)
					++acc;

				if (old_acc != acc)
					tx_cmd('S', acc);
				old_acc = acc;
				
			}

		}
		usleep(1);

	}

	close(fd);
	return 0;
}

/* vim: set sts=0 fenc=utf-8: */
