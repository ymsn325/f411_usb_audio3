#pragma once

#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint16_t idVendor;
  uint16_t idProduct;
  uint16_t bcdDevice;
  uint8_t iManufacturer;
  uint8_t iProduct;
  uint8_t iSerialNumber;
  uint8_t bNumConfigurations;
} USB_DeviceDescriptor;

// Standard Configuration Descriptor
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
} USB_ConfigurationDescriptor;

// Standard Interface Descriptor
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubClass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
} USB_InterfaceDescriptor;

// Interface Association Descriptor
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;   // INTERFACE_ASSOCIATION (0x0B)
  uint8_t bFirstInterface;   // First interface number
  uint8_t bInterfaceCount;   // Number of interfaces
  uint8_t bFunctionClass;    // Audio (0x01)
  uint8_t bFunctionSubClass; // Audio Function (0x00)
  uint8_t bFunctionProtocol; // AF_VERSION_02_00 (0x20)
  uint8_t iFunction;         // String descriptor index
} USB_InterfaceAssociationDescriptor;

// Audio Control Interface Header Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_INTERFACE (0x24)
  uint8_t bDescriptorSubtype; // HEADER (0x01)
  uint16_t bcdADC;       // Audio Device Class spec version (0x0200 for UAC 2.0)
  uint8_t bCategory;     // Function category
  uint16_t wTotalLength; // Total size of class-specific AC interface descriptor
  uint8_t bmControls;    // Bitmap of controls
} UAC2_ACHeaderDescriptor;

// Clock Source Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_INTERFACE (0x24)
  uint8_t bDescriptorSubtype; // CLOCK_SOURCE (0x0A)
  uint8_t bClockID;           // Clock source ID
  uint8_t bmAttributes;       // Clock attributes
  uint8_t bmControls;         // Clock controls
  uint8_t bAssocTerminal;     // Associated terminal ID
  uint8_t iClockSource;       // String descriptor index
} UAC2_ClockSourceDescriptor;

// Input Terminal Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_INTERFACE (0x24)
  uint8_t bDescriptorSubtype; // INPUT_TERMINAL (0x02)
  uint8_t bTerminalID;        // Terminal ID
  uint16_t wTerminalType;     // Terminal type
  uint8_t bAssocTerminal;     // Associated output terminal ID
  uint8_t bCSourceID;         // Connected clock source ID
  uint8_t bNrChannels;        // Number of channels
  uint32_t bmChannelConfig;   // Channel configuration bitmap
  uint8_t iChannelNames;      // String descriptor for channel names
  uint16_t bmControls;        // Bitmap of controls
  uint8_t iTerminal;          // String descriptor for terminal
} UAC2_InputTerminalDescriptor;

// Output Terminal Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_INTERFACE (0x24)
  uint8_t bDescriptorSubtype; // OUTPUT_TERMINAL (0x03)
  uint8_t bTerminalID;        // Terminal ID
  uint16_t wTerminalType;     // Terminal type
  uint8_t bAssocTerminal;     // Associated input terminal ID
  uint8_t bSourceID;          // Source unit/terminal ID
  uint8_t bCSourceID;         // Connected clock source ID
  uint16_t bmControls;        // Bitmap of controls
  uint8_t iTerminal;          // String descriptor for terminal
} UAC2_OutputTerminalDescriptor;

// Standard Endpoint Descriptor
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  uint16_t wMaxPacketSize;
  uint8_t bInterval;
} USB_EndpointDescriptor;

// Audio Streaming Interface Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_INTERFACE (0x24)
  uint8_t bDescriptorSubtype; // AS_GENERAL (0x01)
  uint8_t bTerminalLink;      // Terminal ID linked to this interface
  uint8_t bmControls;         // Bitmap of controls
  uint8_t bFormatType;        // Format type (0x01 for Type I)
  uint32_t bmFormats;         // Bitmap of supported formats
  uint8_t bNrChannels;        // Number of channels
  uint32_t bmChannelConfig;   // Channel configuration bitmap
  uint8_t iChannelNames;      // String descriptor for channel names
} UAC2_ASGeneralDescriptor;

// Type I Format Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;            // 6
  uint8_t bDescriptorType;    // 0x24 (CS_INTERFACE)
  uint8_t bDescriptorSubtype; // 0x02 (FORMAT_TYPE)
  uint8_t bFormatType;        // 0x01 (FORMAT_TYPE_I)
  uint8_t bSubslotSize;       // 2 (16bit)
  uint8_t bBitResolution;     // 16
} UAC2_FormatTypeDescriptor;

// Audio Streaming Isochronous Endpoint Descriptor (UAC 2.0)
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;    // CS_ENDPOINT (0x25)
  uint8_t bDescriptorSubtype; // EP_GENERAL (0x01)
  uint8_t bmAttributes;       // Endpoint attributes
  uint8_t bmControls;         // Bitmap of controls
  uint8_t bLockDelayUnits;    // Lock delay units
  uint16_t wLockDelay;        // Lock delay value
} UAC2_ASEndpointDescriptor;

// Complete UAC 2.0 Configuration Descriptor
typedef struct __attribute__((packed)) {
  // Configuration Descriptor
  USB_ConfigurationDescriptor config;

  // Interface Association Descriptor (Audio Function)
  struct {
    uint8_t bLength;
    uint8_t bDescriptorType;   // INTERFACE_ASSOCIATION (0x0B)
    uint8_t bFirstInterface;   // First interface number
    uint8_t bInterfaceCount;   // Number of interfaces
    uint8_t bFunctionClass;    // Audio (0x01)
    uint8_t bFunctionSubClass; // Audio Function (0x00)
    uint8_t bFunctionProtocol; // AF_VERSION_02_00 (0x20)
    uint8_t iFunction;         // String descriptor index
  } iad;

  // Audio Control Interface (Interface 0, Alt 0)
  USB_InterfaceDescriptor ac_interface;
  UAC2_ACHeaderDescriptor ac_header;
  UAC2_ClockSourceDescriptor clock_source;
  UAC2_InputTerminalDescriptor input_terminal;
  UAC2_OutputTerminalDescriptor output_terminal;

  // Audio Streaming Interface (Interface 1, Alt 0 - Zero bandwidth)
  USB_InterfaceDescriptor as_interface_alt0;

  // Audio Streaming Interface (Interface 1, Alt 1 - Operational)
  USB_InterfaceDescriptor as_interface_alt1;
  UAC2_ASGeneralDescriptor as_general;
  UAC2_FormatTypeDescriptor format_type;

  // Audio Streaming Endpoint
  USB_EndpointDescriptor as_endpoint;
  UAC2_ASEndpointDescriptor as_ep_desc;

} UAC2_ConfigurationDescriptor;

// Device Qualifier Descriptor
typedef struct __attribute__((packed)) {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint16_t bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubClass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  uint8_t bNumConfigurations;
  uint8_t bReserved;
} USB_DeviceQualifierDescriptor;
#pragma pack(pop)

// UAC 2.0 Constants
#define UAC2_BCDADC 0x0200          // UAC 2.0
#define UAC2_FUNCTION_SUBCLASS 0x00 // Audio Function
#define UAC2_AF_VERSION_02_00 0x20  // Audio Function Protocol

// Audio Class Codes
#define USB_CLASS_AUDIO 0x01
#define USB_SUBCLASS_AUDIOCONTROL 0x01
#define USB_SUBCLASS_AUDIOSTREAMING 0x02

// Audio Descriptor Types
#define USB_DTYPE_CS_INTERFACE 0x24
#define USB_DTYPE_CS_ENDPOINT 0x25

// Audio Control Interface Descriptor Subtypes (UAC 2.0)
#define UAC2_AC_DESCRIPTOR_UNDEFINED 0x00
#define UAC2_HEADER 0x01
#define UAC2_INPUT_TERMINAL 0x02
#define UAC2_OUTPUT_TERMINAL 0x03
#define UAC2_CLOCK_SOURCE 0x0A

// Audio Streaming Interface Descriptor Subtypes (UAC 2.0)
#define UAC2_AS_DESCRIPTOR_UNDEFINED 0x00
#define UAC2_AS_GENERAL 0x01
#define UAC2_FORMAT_TYPE 0x02

// Audio Streaming Endpoint Descriptor Subtypes (UAC 2.0)
#define UAC2_EP_DESCRIPTOR_UNDEFINED 0x00
#define UAC2_EP_GENERAL 0x01

// Terminal Types
#define UAC2_TERMINAL_USB_STREAMING 0x0101
#define UAC2_TERMINAL_SPEAKER 0x0301
#define UAC2_TERMINAL_MICROPHONE 0x0201

// Format Type
#define UAC2_FORMAT_TYPE_I 0x01

// Audio Data Formats (bmFormats)
#define UAC2_FORMAT_PCM 0x00000001
