#include "usb_audio.h"
#include "log.h"
#include "usart.h"
#include "usb.h"
#include <stddef.h>

// Global clock source state
UAC2_ClockSourceState uac2_clock_source_state = {.sample_rate =
                                                     UAC2_SAMPLE_RATE_48000,
                                                 .clock_valid = true,
                                                 .clock_locked = true};

void uac2_init(void) {
  LOG_INFO("UAC2.0 Audio Class initialized\r\n");
  uac2_clock_source_state.sample_rate = UAC2_SAMPLE_RATE_48000;
  uac2_clock_source_state.clock_valid = true;
  uac2_clock_source_state.clock_locked = true;
}

void uac2_process_audio_request(USB_SetupPacket *setup) {
  uint8_t recipient = setup->bmRequestType & 0x1F;
  uint8_t entity_id = (setup->wIndex >> 8) & 0xFF;
  uint8_t interface_id = setup->wIndex & 0xFF;
  uint8_t control_selector = (setup->wValue >> 8) & 0xFF;

  LOG_INFO(
      "Audio Request: Recipient=0x%02X, Entity=0x%02X, Interface=0x%02X\r\n",
      recipient, entity_id, interface_id);
  LOG_DEBUG("Control Selector=0x%02X, Request=0x%02X\r\n", control_selector,
            setup->bRequest);

  // Recipient check
  if (recipient == 0x01) { // Interface
    if (interface_id == UAC2_INTERFACE_CONTROL) {
      // Audio Control Interface requests
      switch (entity_id) {
      case UAC2_ENTITY_ID_CLOCK_SOURCE:
        uac2_handle_clock_source_request(setup, control_selector);
        break;
      case UAC2_ENTITY_ID_CLOCK_SELECTOR:
        uac2_handle_clock_selector_request(setup, control_selector);
        break;
      default:
        LOG_WARN("Unsupported entity ID: 0x%02X\r\n", entity_id);
        usb_control_stall();
        break;
      }
    } else if (interface_id == UAC2_INTERFACE_STREAMING) {
      // Audio Streaming Interface requests
      LOG_DEBUG("Audio Streaming Interface request\r\n");
      usb_control_send_data(NULL, 0); // ACK for now
    } else {
      LOG_WARN("Unknown interface ID: 0x%02X\r\n", interface_id);
      usb_control_stall();
    }
  } else if (recipient == 0x02) { // Endpoint
    LOG_DEBUG("Audio Endpoint request\r\n");
    usb_control_send_data(NULL, 0); // ACK for now
  } else {
    LOG_WARN("Unsupported recipient: 0x%02X\r\n", recipient);
    usb_control_stall();
  }
}

void uac2_handle_clock_source_request(USB_SetupPacket *setup,
                                      uint8_t control_selector) {
  static uint8_t response_buffer[8];

  LOG_DEBUG("Clock Source Request: Control=0x%02X, Request=0x%02X\r\n",
            control_selector, setup->bRequest);

  switch (control_selector) {
  case UAC2_CS_SAM_FREQ_CONTROL:
    if (setup->bRequest == UAC2_REQUEST_CUR) {
      // GET_CUR: Return current sample rate
      if (setup->bmRequestType & 0x80) {
        LOG_INFO("GET_CUR Sample Rate: %d Hz\r\n",
                 uac2_clock_source_state.sample_rate);
        response_buffer[0] = (uac2_clock_source_state.sample_rate >> 0) & 0xFF;
        response_buffer[1] = (uac2_clock_source_state.sample_rate >> 8) & 0xFF;
        response_buffer[2] = (uac2_clock_source_state.sample_rate >> 16) & 0xFF;
        response_buffer[3] = (uac2_clock_source_state.sample_rate >> 24) & 0xFF;
        usb_control_send_data(response_buffer, 4);
      } else {
        // SET_CUR: Set sample rate
        LOG_DEBUG("SET_CUR Sample Rate request\r\n");
        usb_control_send_data(NULL, 0); // ACK
      }
    } else if (setup->bRequest == UAC2_REQUEST_RANGE) {
      // GET_RANGE: Return supported sample rate range
      if (setup->bmRequestType & 0x80) {
        LOG_DEBUG("GET_RANGE Sample Rate\r\n");
        static uint8_t sample_rate_range[14] = {
            0x01, 0x00,             // wNumSubranges
            0x80, 0xbb, 0x00, 0x00, // dMIN
            0x80, 0xbb, 0x00, 0x00, // dMAX
            0x01, 0x00, 0x00, 0x00  // dRES
        };
        usb_control_send_data(sample_rate_range, 14);
      } else {
        usb_control_stall();
      }
    } else {
      usb_control_stall();
    }
    break;

  case UAC2_CS_CLOCK_VALID_CONTROL:
    if (setup->bRequest == UAC2_REQUEST_CUR) {
      if (setup->bmRequestType & 0x80) {
        // GET_CUR: Return clock validity
        LOG_INFO("GET_CUR Clock Valid: %s\r\n",
                 uac2_clock_source_state.clock_valid ? "Valid" : "Invalid");
        response_buffer[0] = uac2_clock_source_state.clock_valid ? 1 : 0;
        usb_control_send_data(response_buffer, 1);
      } else {
        // SET_CUR not typically supported for clock validity
        usb_control_stall();
      }
    } else {
      usb_control_stall();
    }
    break;

  default:
    LOG_WARN("Unsupported Clock Source control: 0x%02X\r\n", control_selector);
    usb_control_stall();
    break;
  }
}

void uac2_handle_clock_selector_request(USB_SetupPacket *setup,
                                        uint8_t control_selector) {
  static uint8_t response_buffer[4];

  LOG_DEBUG("Clock Selector Request: Control=0x%02X, Request=0x%02X\r\n",
            control_selector, setup->bRequest);

  // For simple implementation, we only have one clock source
  if (setup->bRequest == UAC2_REQUEST_CUR) {
    if (setup->bmRequestType & 0x80) {
      // GET_CUR: Return selected clock source
      response_buffer[0] = UAC2_ENTITY_ID_CLOCK_SOURCE; // Clock source 1
      usb_control_send_data(response_buffer, 1);
    } else {
      // SET_CUR: Select clock source
      LOG_DEBUG("SET_CUR Clock Selector\r\n");
      usb_control_send_data(NULL, 0); // ACK
    }
  } else {
    usb_control_stall();
  }
}
