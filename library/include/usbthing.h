

#ifndef USBTHING_H
#define USBTHING_H

#include <stdint.h>
#include "libusb-1.0/libusb.h"

struct usbthing_s {
	libusb_device_handle *handle;
};

int USBTHING_init();
void USBTHING_close();

int USBTHING_list_devices(uint16_t vid_filter, uint16_t pid_filter);

int USBTHING_connect(struct usbthing_s *usbthing, uint16_t vid_filter, uint16_t pid_filter);

int USBTHING_disconnect(struct usbthing_s *usbthing);

#endif
