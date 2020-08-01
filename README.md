## BallMouse
Translates a Linux evdev device from a SpaceBall into something that actually looks like a mouse.

They tell me necessity is the mother of invention.  Here's an invention that I found necessary when I got an old RS232 SpaceBall 4000 FLX and wanted to use it as a mouse on a Linux system with Xorg.  The device can be attached to the input subsystem with **inputattach**, but the device that shows up when you do looks to the X server like either a joystick or a keyboard, or maybe a combination of both, or not quite either one.  It won't autodetect and do anything useful, and on my system, even with explicit configuration, it would consistently fail to deliver core pointer events.

### Enter *BallMouse*

**BallMouse** searches /dev/input for the first thing that looks like a SpaceBall, and additionally registers itself via **uevent** as a device that looks quite like an extra complicated mouse.  The new device is reported as "BallMouse," but basically provides all of the data coming from the SpaceBall, in relative instead of absolute coordinates, and with some coordinate scaling done.  The buttons are remapped slightly to make it look more like a mouse as well.  This allows **Xorg** to detect and use the SpaceBall without any configuration.

### Build/Installation instructions

   * Make sure you have libevdev on your system and the correct includes for it.
   * Edit the **Makefile**, making sure the library and include paths for libevdev are correct.
   * Edit **config.h**, in case you want some of the adjustments to your mouse output to be a bit different than mine.
   * **make**
   * Put **BallMouse** into */usr/local/bin* and **BallMouse.service** in */etc/systemd/system*, or -- at your option -- edit **BallMouse.service** and change it to load **BallMouse** from whatever location you like, putting the **BallMouse** executable there.

### Usage

First, plug your spaceball in and get an **evdev** device for it.  If you have a USB spaceball, you're probably already done.  If you have a serial SpaceBall, you'll need to run **inputattach** on the serial port, probably with the **-sbl** flag. 

I have the following in */etc/conf.d/inputattach*:

    IAPARAMS=(
      "-sbl /dev/ttyS0"
    )

The **inputattach** systemd service can then be used to bring the device up during boot.  That done, simply also load the **BallMouse** service, or just run **BallMouse**.  It should create a new **evdev** device, which is your emulated mouse.

### ...but what's the actual problem?

 Honestly, in trying to write this thing, I found that literally any device which presents buttons BTN_C, BTN_SOUTH, or BTN_EAST, all of which my SpaceBall does, fails to match the MatchIsMouse rule in *Xorg.conf*.  That's probably not the only problem, considering that it wouldn't let me manually configure the device either, but it won't help.  To be thorough, I made sure to include actual mouse buttons (LEFT, RIGHT, MIDDLE), offer only relative coordinates, and even put the word "mouse" in the device name.  Probably not all of that is required.  Maybe most of it isn't, but it works now.
