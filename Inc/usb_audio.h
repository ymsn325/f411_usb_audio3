#pragma once

#include "usb.h"
#include <stdbool.h>
#include <stdint.h>

#define CALLBACK_TIME_HISTORY_SIZE 128

// UAC2.0 Request Codes
#define UAC2_REQUEST_CUR 0x01
#define UAC2_REQUEST_RANGE 0x02
#define UAC2_REQUEST_MEM 0x03

// UAC2.0 Control Selector Codes
#define UAC2_CS_CONTROL_UNDEFINED 0x00
#define UAC2_CS_SAM_FREQ_CONTROL 0x01
#define UAC2_CS_CLOCK_VALID_CONTROL 0x02
#define UAC2_TE_COPY_PROTECT_CONTROL 0x01
#define UAC2_TE_CONNECTOR_CONTROL 0x02
#define UAC2_TE_OVERLOAD_CONTROL 0x03
#define UAC2_TE_CLUSTER_CONTROL 0x04
#define UAC2_TE_UNDERFLOW_CONTROL 0x05
#define UAC2_TE_OVERFLOW_CONTROL 0x06
#define UAC2_TE_LATENCY_CONTROL 0x07

// UAC2.0 Clock Source Controls
#define UAC2_CS_SAM_FREQ_CONTROL 0x01
#define UAC2_CS_CLOCK_VALID_CONTROL 0x02

// UAC2.0 Entity IDs (Configuration Descriptorと一致させる)
#define UAC2_ENTITY_ID_CLOCK_SOURCE 0x10
#define UAC2_ENTITY_ID_CLOCK_SELECTOR 0x11
#define UAC2_ENTITY_ID_INPUT_TERMINAL 0x12
#define UAC2_ENTITY_ID_OUTPUT_TERMINAL 0x13

// Audio Interface Numbers
#define UAC2_INTERFACE_CONTROL 0x00
#define UAC2_INTERFACE_STREAMING 0x01

// Sample Rate related
#define UAC2_SAMPLE_RATE_48000 48000
#define UAC2_SAMPLE_RATE_44100 44100

#define AUDIO_BUFFER_SIZE 192

// UAC2.0 Clock Source State
typedef struct {
  uint32_t sample_rate; // Current sample rate
  bool clock_valid;     // Clock validity
  bool clock_locked;    // Clock lock status
} UAC2_ClockSourceState;

// Global state
extern UAC2_ClockSourceState uac2_clock_source_state;

// Function declarations
void uac2_init(void);
void uac2_process_audio_request(USB_SetupPacket *setup);
void uac2_handle_clock_source_request(USB_SetupPacket *setup,
                                      uint8_t control_selector);
void uac2_handle_clock_selector_request(USB_SetupPacket *setup,
                                        uint8_t control_selector);
void uac2_prepare_next_reception(void);
void uac2_handle_audio_data_received(void);
void uac2_read_audio_from_fifo(uint32_t byte_count);
