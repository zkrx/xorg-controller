#ifndef BALLMOUSE_CONFIG_H
#define BALLMOUSE_CONFIG_H

/* Minimum value that we must produce to register a movement.  This is not adjusted for velocity and
 * 	can be quite large */
#define ABS_MIN		500

/* ... and this is the adjustment we make for mouse velocity on the display, since the native coordinate
 * 	system is quite large. */
#define VELOCITY	.009

/* Farther scaling to apply to the wheel velocity later. */
#define WVEL		.2

/* Because the input subsystem needs to know, we'll cap our relative motion at this number. */
#define	MAXVAL		255

/* What will we call our output device? */
#define	DEVNAME		"BallMouse SpaceBall Proxy Server"


#endif

