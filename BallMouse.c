
/* This is a simple service that finds the first SpaceBall device in
 * /dev/input, and creates a clone of it that delivers more mouse-like
 * events to the system. */

#include		<stdio.h>
#include		<string.h>
#include		<fcntl.h>
#include		<sys/errno.h>
#include		<stdlib.h>
#include		<unistd.h>
#include		<dirent.h>
#include		<libevdev/libevdev.h>
#include		<linux/uinput.h>
#include		<poll.h>
#include		"config.h"

static inline void event(int ofd, int type, int code, int val) {
	struct input_event out;
	out.type = type;
	out.code = code;
	out.value = val;
	/*Do some simple remapping to be more mouse-like*/
	if (out.type == EV_KEY) {
		if (out.code == BTN_7)		out.code = BTN_LEFT;
		if (out.code == BTN_1)		out.code = BTN_MIDDLE;
		if (out.code == BTN_2)		out.code = BTN_RIGHT;
		if (out.code == BTN_C)		out.code = BTN_1;
		if (out.code == BTN_EAST)	out.code = BTN_2;
		if (out.code == BTN_SOUTH)	out.code = BTN_7;
	}
	if (out.type == EV_REL) {
		/* Seems like we need to invert Y for right-hand operation. */
		if (out.code == REL_Y) out.value = -out.value;
		if (out.code == REL_RZ) {
			out.code = REL_WHEEL;
			out.value = out.value*WVEL;
		}
	}

	write(ofd,&out,sizeof(out));
}

int main(int argc, char **argv) {
	struct libevdev *spaceball = NULL;
	struct input_event in, out;
	int ifd;
	int ofd;
	int code = 1;
	struct uinput_setup mouse;
	DIR *evdir;
	struct dirent *evfile;
	struct pollfd inpoll[1];

	/* Set up the input device. */

	if (chdir("/dev/input")) {
		printf("Can't set directory /dev/input.\n");
		exit(1);
	}
	evdir = opendir(".");
	if (evdir == NULL) {
		printf("Can't open directory for /dev/input\n");
		exit(1);
	}

	while ((evfile = readdir(evdir)) != NULL) {
		ifd = open(evfile->d_name,O_RDONLY|O_NONBLOCK);
		code = libevdev_new_from_fd(ifd,&spaceball);
		if (code < 0) { //Can't create evdev instance.
			close(ifd);
			continue;
		}
		/* A real spaceball -- rather than another BallMouse
		 * will have EV_ABS events. */
		if(strstr(libevdev_get_name(spaceball),"SpaceBall") && libevdev_has_event_type(spaceball,EV_ABS)) {
			printf("Attaching to device: %s\n",libevdev_get_name(spaceball));
			break;	//This is it.  We just do the first one
		}
		/* Otherwise free resources up for the next iteration. */
		libevdev_free(spaceball);
		spaceball = NULL;
		close(ifd);
	}

	if (spaceball == NULL) {
		printf("Can't find appropriate device.\n");
		exit(1);
	}

	/* Set up monitoring for input data */
	inpoll[0].fd = ifd;
	inpoll[0].events = POLLIN||POLLRDBAND;

	/* Now set up the output. */
	ofd = open("/dev/uinput",O_WRONLY|O_NONBLOCK);
	memset(&mouse,0,sizeof(mouse)); //Probably not actually required.
	
	//Describe the fake mouse
	mouse.id.bustype	= BUS_USB;
	mouse.id.vendor		= 0x05A8;	//Just use the one for Spacetec
	mouse.id.product	= 0xFFE0;
	strcpy(mouse.name,DEVNAME);

	ioctl(ofd,UI_SET_EVBIT,EV_REL);
	ioctl(ofd,UI_SET_RELBIT,REL_X);
	ioctl(ofd,UI_SET_RELBIT,REL_Y);
	ioctl(ofd,UI_SET_RELBIT,REL_Z);
	ioctl(ofd,UI_SET_RELBIT,REL_WHEEL);
	ioctl(ofd,UI_SET_RELBIT,REL_HWHEEL);
	ioctl(ofd,UI_SET_RELBIT,REL_RX);
	ioctl(ofd,UI_SET_RELBIT,REL_RY);
	ioctl(ofd,UI_SET_RELBIT,REL_RZ);
	ioctl(ofd,UI_SET_EVBIT,EV_KEY);
	ioctl(ofd,UI_SET_KEYBIT,BTN_1);
	ioctl(ofd,UI_SET_KEYBIT,BTN_LEFT);
	ioctl(ofd,UI_SET_KEYBIT,BTN_2);
	ioctl(ofd,UI_SET_KEYBIT,BTN_3);
	ioctl(ofd,UI_SET_KEYBIT,BTN_4);
	ioctl(ofd,UI_SET_KEYBIT,BTN_5);
	ioctl(ofd,UI_SET_KEYBIT,BTN_6);
	ioctl(ofd,UI_SET_KEYBIT,BTN_RIGHT);
	ioctl(ofd,UI_SET_KEYBIT,BTN_7);
	ioctl(ofd,UI_SET_KEYBIT,BTN_8);
	ioctl(ofd,UI_SET_KEYBIT,BTN_9);
	ioctl(ofd,UI_SET_KEYBIT,BTN_MIDDLE);

/*	These buttons appear in the actual device, but if we report them here, MatchIsMouse doesn't match. */
//	ioctl(ofd,UI_SET_KEYBIT,BTN_C);
//	ioctl(ofd,UI_SET_KEYBIT,BTN_EAST);
//	ioctl(ofd,UI_SET_KEYBIT,BTN_SOUTH);
/*	... so replace them with other buttons.	*/


	ioctl(ofd,UI_DEV_SETUP,&mouse);
	code = ioctl(ofd,UI_DEV_CREATE);
	if (code) {
		printf("Can't create output device.\n");
		exit(1);
	}


	/* Events are processed here. */

	code = 0;
	while((code == 0)||(code == 1)||(code == -EAGAIN)) {
		code = libevdev_next_event(spaceball,LIBEVDEV_READ_FLAG_NORMAL,&in);
		if (code == -EAGAIN) {
			while (poll(inpoll,1,500) == 0); //0 is a timeout, so we just go again.
			continue;
		}
		if (code ==0) { /*This is a real event */
			/* We just kind of fake this one by sending a clean SYN event for each one we get. */
			if (in.type == EV_SYN) {
				if((in.code == SYN_REPORT) && (in.value == 0)) {
					event(ofd,EV_SYN,SYN_REPORT,0);	
				}
				continue;
			}
			if (in.type == EV_ABS) {//Handle absolute motion.
				if (abs(in.value) > ABS_MIN) {	/* Allow a bit of a dead spot near the center. */
					event(ofd,EV_REL,in.code,in.value*VELOCITY);
					continue;
				}
			}
			if (in.type == EV_KEY) {//Handle button events.
				event(ofd,in.type,in.code,in.value);
				continue;
			}
		}
	}
	exit(0);
}


