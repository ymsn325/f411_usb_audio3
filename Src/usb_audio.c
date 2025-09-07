#include "usb_audio.h"
#include "log.h"
#include "stm32f411xe.h"
#include "usart.h"
#include "usb.h"
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

extern uint64_t global_time_us;

// Global clock source state
UAC2_ClockSourceState uac2_clock_source_state = {.sample_rate =
                                                     UAC2_SAMPLE_RATE_48000,
                                                 .clock_valid = true,
                                                 .clock_locked = true};
static uint8_t audio_rx_buf[AUDIO_BUFFER_SIZE];
static uint32_t audio_buffer_index = 0;
int16_t audio_rx_samples_0[AUDIO_BUFFER_SIZE / 2] = {0};
int16_t audio_rx_samples_1[AUDIO_BUFFER_SIZE / 2];
static uint32_t audio_samples_index = 0;
uint64_t callback_time_history[CALLBACK_TIME_HISTORY_SIZE] = {0};
static uint32_t callback_time_history_index = 0;

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

static void process_audio_sample(uint8_t *data, uint32_t len) {
  callback_time_history[callback_time_history_index] = global_time_us;
  callback_time_history_index =
      (callback_time_history_index + 1) & (CALLBACK_TIME_HISTORY_SIZE - 1);

  uint32_t sample_cnt = len / 4;
  uint8_t buf_select = DMA1_Stream5->CR & DMA_SxCR_CT;

  for (uint32_t i = 0; i < sample_cnt; i++) {
    int16_t x_l = (data[i * 4 + 1] << 8) | data[i * 4];
    int16_t x_r = (data[i * 4 + 3] << 8) | data[i * 4 + 2];
    // LOG_DEBUG("sample l: %d\r\n", x_l);

    if (buf_select) {
      audio_rx_samples_0[audio_samples_index] = x_l;
      audio_rx_samples_0[audio_samples_index + 1] = x_r;
      audio_samples_index = (audio_samples_index + 2) % (AUDIO_BUFFER_SIZE / 2);
    } else {
      audio_rx_samples_1[audio_samples_index] = x_l;
      audio_rx_samples_1[audio_samples_index + 1] = x_r;
      audio_samples_index = (audio_samples_index + 2) % (AUDIO_BUFFER_SIZE / 2);
    }
  }
}

void uac2_prepare_next_reception(void) {
  USB_OUTEP[1].DOEPTSIZ =
      (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | AUDIO_BUFFER_SIZE;
  USB_OUTEP[1].DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;

  // LOG_INFO("Audio reception started\r\n");
}

void uac2_handle_audio_data_received(void) {
  uint32_t received_bytes =
      AUDIO_BUFFER_SIZE - (USB_OUTEP[1].DOEPTSIZ & USB_OTG_DOEPTSIZ_XFRSIZ);

  // LOG_DEBUG("Audio data received: %d bytes\r\n", received_bytes);

  process_audio_sample(audio_rx_buf, received_bytes);

  USB_OUTEP[1].DOEPTSIZ =
      (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | AUDIO_BUFFER_SIZE;
  USB_OUTEP[1].DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;
}

void uac2_read_audio_from_fifo(uint32_t byte_count) {
  uint32_t word_count = (byte_count + 3) / 4;
  uint32_t *fifo = USB_FIFO(0); // RXFIFO is always FIFO(0)

  audio_buffer_index = 0;

  for (uint32_t i = 0; i < word_count; i++) {
    uint32_t data = *fifo;
    if (audio_buffer_index + 3 < sizeof(audio_rx_buf)) {
      audio_rx_buf[audio_buffer_index++] = (uint8_t)(data >> 0) & 0xff;
      audio_rx_buf[audio_buffer_index++] = (uint8_t)(data >> 8) & 0xff;
      audio_rx_buf[audio_buffer_index++] = (uint8_t)(data >> 16) & 0xff;
      audio_rx_buf[audio_buffer_index++] = (uint8_t)(data >> 24) & 0xff;
    }
  }
}
