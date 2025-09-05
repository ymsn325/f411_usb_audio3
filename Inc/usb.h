#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t bmRequestType;
  uint8_t bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} USB_SetupPacket;

typedef enum {
  USB_CTRL_STATE_IDLE,
  USB_CTRL_STATE_SETUP,
  USB_CTRL_STATE_DATA_IN,
  USB_CTRL_STATE_DATA_OUT,
  USB_CTRL_STATE_STATUS_IN,
  USB_CTRL_STATE_STATUS_OUT
} USB_ControlState_t;

typedef struct {
  USB_ControlState_t state;
  USB_SetupPacket setup;
  uint8_t *data_buffer;
  uint16_t data_length;
  uint16_t data_sent;
  bool zlp_required; // Zero Length Packet必要フラグ
  uint8_t pending_address;
  bool address_pending;
} USB_ControlState;

void usb_init(void);
