#include "usb_desc.h"
#include "usb_audio.h"

const USB_DeviceDescriptor device_descriptor = {
    .bLength = sizeof(USB_DeviceDescriptor),
    .bDescriptorType = 0x01,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xef,
    .bDeviceSubClass = 0x02,
    .bDeviceProtocol = 0x01,
    .bMaxPacketSize0 = 0x40,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = 0x0200,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x00,
    .bNumConfigurations = 0x01,
};

const UAC2_ConfigurationDescriptor configuration_descriptor = {
    // Configuration Descriptor
    .config =
        {
            .bLength = sizeof(USB_ConfigurationDescriptor),
            .bDescriptorType = 0x02, // CONFIGURATION
            .wTotalLength = sizeof(UAC2_ConfigurationDescriptor),
            .bNumInterfaces = 2, // AC Interface + AS Interface
            .bConfigurationValue = 1,
            .iConfiguration = 0,
            .bmAttributes = 0x80, // Bus powered
            .bMaxPower = 50       // 100mA
        },

    // Interface Association Descriptor (Audio Function)
    .iad =
        {
            .bLength = sizeof(USB_InterfaceAssociationDescriptor),
            .bDescriptorType = 0x0B,           // INTERFACE_ASSOCIATION
            .bFirstInterface = 0,              // First interface (AC)
            .bInterfaceCount = 2,              // AC + AS interfaces
            .bFunctionClass = USB_CLASS_AUDIO, // Audio (0x01)
            .bFunctionSubClass =
                UAC2_FUNCTION_SUBCLASS, // Audio Function (0x00)
            .bFunctionProtocol =
                UAC2_AF_VERSION_02_00, // AF_VERSION_02_00 (0x20)
            .iFunction = 0             // No string descriptor
        },

    // Audio Control Interface (Interface 0, Alt 0)
    .ac_interface = {.bLength = sizeof(USB_InterfaceDescriptor),
                     .bDescriptorType = 0x04, // INTERFACE
                     .bInterfaceNumber = 0,
                     .bAlternateSetting = 0,
                     .bNumEndpoints = 0, // No endpoints in AC interface
                     .bInterfaceClass = USB_CLASS_AUDIO, // Audio (0x01)
                     .bInterfaceSubClass =
                         USB_SUBCLASS_AUDIOCONTROL, // Audio Control (0x01)
                     .bInterfaceProtocol =
                         UAC2_AF_VERSION_02_00, // UAC 2.0 (0x20)
                     .iInterface = 0},

    // Audio Control Header Descriptor
    .ac_header =
        {
            .bLength = sizeof(UAC2_ACHeaderDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype = UAC2_HEADER,         // HEADER (0x01)
            .bcdADC = UAC2_BCDADC,                     // UAC 2.0 (0x0200)
            .bCategory = 0x08,                         // Desktop speaker
            .wTotalLength =
                sizeof(UAC2_ACHeaderDescriptor) +
                sizeof(UAC2_ClockSourceDescriptor) +
                sizeof(UAC2_InputTerminalDescriptor) +
                sizeof(UAC2_OutputTerminalDescriptor), // Header + Clock + Input
                                                       // + Output
            .bmControls = 0x00                         // No controls
        },

    // Clock Source Descriptor
    .clock_source =
        {
            .bLength = sizeof(UAC2_ClockSourceDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype = UAC2_CLOCK_SOURCE,   // CLOCK_SOURCE (0x0A)
            .bClockID = UAC2_ENTITY_ID_CLOCK_SOURCE,   // Clock source ID
            .bmAttributes = 0x01,                      // Internal fixed clock
            .bmControls = 0x01,     // Clock frequency control (read-only)
            .bAssocTerminal = 0x00, // No associated terminal
            .iClockSource = 0       // No string descriptor
        },

    // Input Terminal Descriptor (USB Streaming)
    .input_terminal =
        {
            .bLength = sizeof(UAC2_InputTerminalDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype = UAC2_INPUT_TERMINAL, // INPUT_TERMINAL (0x02)
            .bTerminalID = UAC2_ENTITY_ID_INPUT_TERMINAL, // Terminal ID
            .wTerminalType =
                UAC2_TERMINAL_USB_STREAMING, // USB Streaming (0x0101)
            .bAssocTerminal = 0x00,          // No associated terminal
            .bCSourceID =
                UAC2_ENTITY_ID_CLOCK_SOURCE, // Connected to clock source
            .bNrChannels = 2,                // 2 channels (stereo)
            .bmChannelConfig = 0x00000003,   // Left Front + Right Front
            .iChannelNames = 0,              // No string descriptor
            .bmControls = 0x0000,            // No controls
            .iTerminal = 0                   // No string descriptor
        },

    // Output Terminal Descriptor (Speaker)
    .output_terminal =
        {
            .bLength = sizeof(UAC2_OutputTerminalDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype =
                UAC2_OUTPUT_TERMINAL, // OUTPUT_TERMINAL (0x03)
            .bTerminalID = UAC2_ENTITY_ID_OUTPUT_TERMINAL, // Terminal ID
            .wTerminalType = UAC2_TERMINAL_SPEAKER,        // Speaker (0x0301)
            .bAssocTerminal = 0x00, // No associated terminal
            .bSourceID =
                UAC2_ENTITY_ID_INPUT_TERMINAL, // Connected to input terminal
            .bCSourceID =
                UAC2_ENTITY_ID_CLOCK_SOURCE, // Connected to clock source
            .bmControls = 0x0000,            // No controls
            .iTerminal = 0                   // No string descriptor
        },

    // Audio Streaming Interface (Interface 1, Alt 0 - Zero bandwidth)
    .as_interface_alt0 =
        {.bLength = sizeof(USB_InterfaceDescriptor),
         .bDescriptorType = 0x04, // INTERFACE
         .bInterfaceNumber = 1,
         .bAlternateSetting = 0,
         .bNumEndpoints = 0,                 // No endpoints (zero bandwidth)
         .bInterfaceClass = USB_CLASS_AUDIO, // Audio (0x01)
         .bInterfaceSubClass =
             USB_SUBCLASS_AUDIOSTREAMING,             // Audio Streaming (0x02)
         .bInterfaceProtocol = UAC2_AF_VERSION_02_00, // UAC 2.0 (0x20)
         .iInterface = 0},

    // Audio Streaming Interface (Interface 1, Alt 1 - Operational)
    .as_interface_alt1 =
        {.bLength = sizeof(USB_InterfaceDescriptor),
         .bDescriptorType = 0x04, // INTERFACE
         .bInterfaceNumber = 1,
         .bAlternateSetting = 1,
         .bNumEndpoints = 1,                 // One isochronous endpoint
         .bInterfaceClass = USB_CLASS_AUDIO, // Audio (0x01)
         .bInterfaceSubClass =
             USB_SUBCLASS_AUDIOSTREAMING,             // Audio Streaming (0x02)
         .bInterfaceProtocol = UAC2_AF_VERSION_02_00, // UAC 2.0 (0x20)
         .iInterface = 0},

    // Audio Streaming General Descriptor
    .as_general =
        {
            .bLength = sizeof(UAC2_ASGeneralDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype = UAC2_AS_GENERAL,     // AS_GENERAL (0x01)
            .bTerminalLink =
                UAC2_ENTITY_ID_INPUT_TERMINAL, // Connected to input terminal
            .bmControls = 0x00,                // No controls
            .bFormatType = UAC2_FORMAT_TYPE_I, // Format Type I (0x01)
            .bmFormats = UAC2_FORMAT_PCM,      // PCM format (0x00000001)
            .bNrChannels = 2,                  // 2 channels
            .bmChannelConfig = 0x00000003,     // Left Front + Right Front
            .iChannelNames = 0                 // No string descriptor
        },

    // Type I Format Descriptor
    .format_type =
        {
            .bLength = sizeof(UAC2_FormatTypeDescriptor),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE, // CS_INTERFACE (0x24)
            .bDescriptorSubtype = UAC2_FORMAT_TYPE,    // FORMAT_TYPE (0x02)
            .bFormatType = UAC2_FORMAT_TYPE_I,         // Format Type I (0x01)
            .bSubslotSize = 2,    // 2 bytes per sample (16bit)
            .bBitResolution = 16, // 16 bits per sample
        },

    // Audio Streaming Endpoint (Isochronous OUT)
    .as_endpoint =
        {
            .bLength = sizeof(USB_EndpointDescriptor),
            .bDescriptorType = 0x05,  // ENDPOINT
            .bEndpointAddress = 0x01, // EP1 OUT
            .bmAttributes = 0x09,     // Isochronous, Adaptive
            .wMaxPacketSize = 192,    // 48kHz * 2ch * 2bytes + overhead
            .bInterval = 1            // 1ms interval (Full Speed)
        },

    // Audio Streaming Endpoint Descriptor
    .as_ep_desc = {
        .bLength = sizeof(UAC2_ASEndpointDescriptor),
        .bDescriptorType = USB_DTYPE_CS_ENDPOINT, // CS_ENDPOINT (0x25)
        .bDescriptorSubtype = UAC2_EP_GENERAL,    // EP_GENERAL (0x01)
        .bmAttributes = 0x00,                     // No attributes
        .bmControls = 0x00,                       // No controls
        .bLockDelayUnits = 0x02,                  // Decoded PCM samples
        .wLockDelay = 0x0000                      // No lock delay
    }};

const USB_DeviceQualifierDescriptor device_qualifier_descriptor = {
    .bLength = sizeof(USB_DeviceQualifierDescriptor),
    .bDescriptorType = 0x06, // DEVICE_QUALIFIER
    .bcdUSB = 0x0200,        // USB 2.0
    .bDeviceClass = 0xef,    // Interface Defined
    .bDeviceSubClass = 0x02,
    .bDeviceProtocol = 0x01,
    .bMaxPacketSize0 = 64,
    .bNumConfigurations = 1,
    .bReserved = 0};

// Language ID descriptor (English US)
const uint8_t string_lang_descriptor[] = {
    4,         // bLength
    0x03,      // bDescriptorType (STRING)
    0x09, 0x04 // wLANGID (0x0409 = English)
};

// Manufacturer string
const uint8_t string_manufacturer_descriptor[] = {
    18,   // bLength
    0x03, // bDescriptorType (STRING)
    'y',  0, 'a', 0, 'm', 0, 'a', 0,
    's',  0, 'h', 0, 'u', 0, 'n', 0 // "yamashun"
};

// Product string
const uint8_t string_product_descriptor[] = {
    28,   // bLength
    0x03, // bDescriptorType (STRING)
    'y',  0, 'a', 0, 'm', 0, 'a', 0, 's', 0, 'h', 0, 'u', 0,
    'n',  0, 'A', 0, 'u', 0, 'd', 0, 'i', 0, 'o', 0 // "yamashunAudio"
};
