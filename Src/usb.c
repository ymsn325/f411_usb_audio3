#include "usb.h"
#include "log.h"
#include "usart.h"
#include "usb_audio.h"
#include "usb_desc.h"
#include <stddef.h>
#include <stdint.h>
#include <stm32f411xe.h>

#define USB_DEVICE                                                             \
  ((USB_OTG_DeviceTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE +                \
                             USB_OTG_DEVICE_BASE))
#define USB_INEP                                                               \
  ((USB_OTG_INEndpointTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE +            \
                                 USB_OTG_IN_ENDPOINT_BASE))
#define USB_OUTEP                                                              \
  ((USB_OTG_OUTEndpointTypeDef *)((uint32_t)USB_OTG_FS_PERIPH_BASE +           \
                                  USB_OTG_OUT_ENDPOINT_BASE))
#define USB_FIFO(ep)                                                           \
  ((uint32_t *)(USB_OTG_FS_PERIPH_BASE + USB_OTG_FIFO_BASE +                   \
                ((ep) * USB_OTG_FIFO_SIZE)))
// GRXSTSP の PKTSTS フィールド値
#define PKTSTS_OUT_NAK 0x01           // OUT NAK (ホストがNAKを受信)
#define PKTSTS_OUT_DATA_RECEIVED 0x02 // OUT データパケット受信
#define PKTSTS_OUT_COMPLETE 0x03      // OUT転送完了
#define PKTSTS_SETUP_COMPLETE 0x04    // SETUP転送完了
#define PKTSTS_OUT_HALT 0x05          // OUT転送でSTALL受信
#define PKTSTS_SETUP_RECEIVED 0x06    // SETUPデータパケット受信

extern const USB_DeviceDescriptor device_descriptor;
extern const UAC2_ConfigurationDescriptor configuration_descriptor;
extern const USB_DeviceQualifierDescriptor device_qualifier_descriptor;
extern const uint8_t string_lang_descriptor[];
extern const uint8_t string_manufacturer_descriptor[];
extern const uint8_t string_product_descriptor[];

// 関数宣言（変更なし）
static void usb_read_packet(uint8_t *dest, uint32_t bcnt);
static void usb_handle_setup(uint32_t epnum);
static void usb_handle_rxflvl(void);
static void usb_process_setup(USB_SetupPacket *setup);
static void usb_send_contorl_packet(void);
static void usb_write_packet(uint8_t epnum, uint8_t *src, uint16_t len);
static void usb_get_device_descriptor(uint8_t **desc_data,
                                      uint16_t *desc_length);
static void usb_get_configuration_descriptor(uint8_t **desc_data,
                                             uint16_t *desc_length);
static void usb_process_get_descriptor(USB_SetupPacket *setup);
static void usb_process_standard_request(USB_SetupPacket *setup);
static void usb_process_set_address(USB_SetupPacket *setup);
static void usb_prepare_ep0_out_status(void);
static void usb_process_audio_request(USB_SetupPacket *setup);
static void usb_cofig_audio_endpoint(void);
static void debug_descriptor_content(void);
static void debug_raw_descriptors(void);

static void usb_core_reset(void) {
  USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
  while (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST)
    ;
}

void usb_init(void) {
  // Configuration Descriptorの後半部分をダンプ
  LOG_INFO("=== Configuration Descriptor Last 20 bytes ===\r\n");
  uint8_t *cfg = (uint8_t *)&configuration_descriptor;
  for (int i = 107; i < 127; i++) {
    LOG_INFO("cfg[%02d]: 0x%02X\r\n", i, cfg[i]);
  }
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
  usb_core_reset();
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN | USB_OTG_GCCFG_VBUSBSEN;
  USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

  USB_OTG_FS->GRXFSIZ = 256;
  USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (64 << USB_OTG_HPTXFSIZ_PTXFD_Pos) | 256;
  // デバイススピード設定
  USB_DEVICE->DCFG |= USB_OTG_DCFG_DSPD; // Full Speed (11)

  // ターンアラウンドタイム設定
  USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_TRDT;
  USB_OTG_FS->GUSBCFG |= (6 << USB_OTG_GUSBCFG_TRDT_Pos); // Full Speed用

  usb_cofig_audio_endpoint();

  USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_RXFLVLM |
                         USB_OTG_GINTMSK_IEPINT | USB_OTG_GINTMSK_OEPINT;
  USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
  USB_DEVICE->DIEPMSK |= USB_OTG_DIEPMSK_XFRCM;
  USB_DEVICE->DOEPMSK |= USB_OTG_DOEPMSK_XFRCM;
  NVIC_SetPriority(OTG_FS_IRQn, 0);
  NVIC_EnableIRQ(OTG_FS_IRQn);

  USB_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;

  uac2_init();
}

void debug_raw_descriptors(void) {
  LOG_INFO("=== Device Descriptor Raw ===\r\n");
  uint8_t *dev = (uint8_t *)&device_descriptor;
  for (int i = 0; i < 18; i++) {
    LOG_INFO("dev[%02d]: 0x%02X\r\n", i, dev[i]);
  }

  LOG_INFO("=== Configuration Descriptor First 40 bytes ===\r\n");
  uint8_t *cfg = (uint8_t *)&configuration_descriptor;
  for (int i = 0; i < 40; i++) {
    LOG_INFO("cfg[%02d]: 0x%02X\r\n", i, cfg[i]);
  }

  LOG_INFO("=== Expected values check ===\r\n");
  LOG_INFO("Device Class (should be 0xEF): 0x%02X\r\n", dev[4]);
  LOG_INFO("Device SubClass (should be 0x02): 0x%02X\r\n", dev[5]);
  LOG_INFO("Device Protocol (should be 0x01): 0x%02X\r\n", dev[6]);
}

void debug_descriptor_content(void) {
  LOG_DEBUG("=== Configuration Descriptor Analysis ===\r\n");
  uint8_t *desc = (uint8_t *)&configuration_descriptor;

  for (int i = 0; i < 127;) {
    uint8_t length = desc[i];
    uint8_t type = desc[i + 1];

    LOG_DEBUG("Desc[%d]: Length=%d, Type=0x%02X\r\n", i, length, type);

    if (type == 0x0B)
      LOG_DEBUG("  -> Interface Association\r\n");
    if (type == 0x04)
      LOG_DEBUG("  -> Interface\r\n");
    if (type == 0x24)
      LOG_DEBUG("  -> Class-Specific Interface\r\n");
    if (type == 0x05)
      LOG_DEBUG("  -> Endpoint\r\n");
    if (type == 0x25)
      LOG_DEBUG("  -> Class-Specific Endpoint\r\n");

    i += length;
  }
}

USB_ControlState usb_control_state = {
    .state = USB_CTRL_STATE_IDLE,
    .setup = {0},
    .data_buffer = NULL,
    .data_length = 0,
    .data_sent = 0,
    .zlp_required = 0,
};

static void usb_cofig_audio_endpoint(void) {
  USB_OUTEP[1].DOEPCTL = USB_OTG_DOEPCTL_USBAEP | USB_OTG_DOEPCTL_EPTYP_0 |
                         (196 << USB_OTG_DOEPCTL_MPSIZ_Pos);
  USB_OTG_FS->DIEPTXF[0] = (196 << USB_OTG_DTXFSTS_INEPTFSAV_Pos) | 256;
}

static void usb_get_device_descriptor(uint8_t **desc_data,
                                      uint16_t *desc_length) {
  if (desc_data == NULL || desc_length == NULL) {
    LOG_ERROR("NULL pointer in usb_get_device_descriptor\r\n");
    return;
  }

  *desc_data = (uint8_t *)&device_descriptor;
  *desc_length = sizeof(USB_DeviceDescriptor); // 固定サイズを使用

  LOG_INFO("Device descriptor: addr=0x%08X, size=%d\r\n", (uint32_t)*desc_data,
           *desc_length);
}

static void usb_get_configuration_descriptor(uint8_t **desc_data,
                                             uint16_t *desc_length) {
  if (desc_data == NULL || desc_length == NULL) {
    LOG_ERROR("NULL pointer in usb_get_configuration_descriptor\r\n");
    return;
  }

  *desc_data = (uint8_t *)&configuration_descriptor;
  *desc_length = sizeof(UAC2_ConfigurationDescriptor);

  LOG_INFO("Config descriptor: addr=0x%08X, size=%d\r\n", (uint32_t)*desc_data,
           *desc_length);
}

static void usb_get_device_qualifier_descriptor(uint8_t **desc_data,
                                                uint16_t *desc_length) {
  if (desc_data == NULL || desc_length == NULL) {
    LOG_ERROR("NULL pointer in usb_get_device_qualifier_descriptor\r\n");
    return;
  }

  *desc_data = (uint8_t *)&device_qualifier_descriptor;
  *desc_length = sizeof(USB_DeviceQualifierDescriptor); // 固定サイズを使用
}

static void usb_get_string_descriptor(uint8_t **desc_data,
                                      uint16_t *desc_length,
                                      const uint8_t *string_descriptor) {
  *desc_data = (uint8_t *)string_descriptor;
  *desc_length = sizeof(string_descriptor);
}

static void usb_process_get_descriptor(USB_SetupPacket *setup) {
  LOG_INFO("GET_DESCRIPTOR: Type=0x%02X, Index=0x%02X, Length=%d\r\n",
           (setup->wValue >> 8) & 0xFF, setup->wValue & 0xFF, setup->wLength);

  uint8_t desc_type = (setup->wValue >> 8) & 0xff;
  uint8_t desc_index = setup->wValue & 0xff;
  uint16_t desc_length = 0; // 実際のディスクリプタサイズ
  uint8_t *desc_data = NULL;

  switch (desc_type) {
  case 0x01:
    // DEVICE_DESCRIPTOR
    LOG_INFO("Requesting device descriptor\r\n");
    usb_get_device_descriptor(&desc_data, &desc_length);
    break;

  case 0x02:
    // CONFIGURATION_DESCRIPTOR
    LOG_INFO("Requesting configuration descriptor\r\n");
    usb_get_configuration_descriptor(&desc_data, &desc_length);
    break;

  case 0x03: // STRING_DESCRIPTOR
    LOG_INFO("Requesting string descriptor %d\r\n", desc_index);
    switch (desc_index) {
    case 0:
      desc_data = (uint8_t *)string_lang_descriptor;
      desc_length = string_lang_descriptor[0];
      LOG_INFO("String 0: bLength=%d\r\n", string_lang_descriptor[0]);
      break;

    case 1:
      desc_data = (uint8_t *)string_manufacturer_descriptor;
      desc_length = string_manufacturer_descriptor[0];
      LOG_INFO("String 1: bLength=%d\r\n", string_manufacturer_descriptor[0]);
      break;

    case 2:
      desc_data = (uint8_t *)string_product_descriptor;
      desc_length = string_product_descriptor[0]; // bLengthフィールドから取得

      // デバッグ：実際の長さを確認
      LOG_INFO("String 2: descriptor says length=%d, setup requests=%d\r\n",
               desc_length, setup->wLength);

      // 要求されたサイズが記述子サイズより大きい場合は記述子サイズに制限
      if (setup->wLength > desc_length) {
        LOG_WARN("Requested size (%d) > descriptor size (%d), limiting\r\n",
                 setup->wLength, desc_length);
      }
      break;

    default:
      LOG_WARN("Invalid string descriptor index: %d\r\n", desc_index);
      usb_control_stall();
      return;
    }
    break;

  case 0x06:
    // DEVICE_QUALIFIER_DESCRIPTOR
    LOG_INFO("Requesting device qualifier descriptor\r\n");
    usb_get_device_qualifier_descriptor(&desc_data, &desc_length);
    break;

  default:
    LOG_WARN("UNKNOWN DESCRIPTOR TYPE: 0x%02X\r\n", desc_type);
    usb_control_stall();
    return;
  }

  if (desc_data != NULL && desc_length > 0) {
    // 要求されたサイズと実際のサイズの小さい方を送信
    uint16_t send_length =
        (setup->wLength < desc_length) ? setup->wLength : desc_length;
    LOG_INFO("Sending %d bytes of descriptor (requested: %d, actual: %d)\r\n",
             send_length, setup->wLength, desc_length);
    usb_control_send_data(desc_data, send_length);
  } else {
    LOG_ERROR("ERROR: No descriptor data to send\r\n");
    usb_control_stall();
  }
}

void usb_control_stall(void) {
  LOG_INFO("Control STALL\r\n");
  USB_INEP[0].DIEPCTL |= USB_OTG_DIEPCTL_STALL;
  USB_OUTEP[0].DOEPCTL |= USB_OTG_DOEPCTL_STALL;
  usb_control_state.state = USB_CTRL_STATE_IDLE;
}

void usb_control_send_data(uint8_t *data, uint16_t length) {
  usb_control_state.data_buffer = data;
  usb_control_state.data_length = length;
  usb_control_state.data_sent = 0;
  usb_control_state.state = USB_CTRL_STATE_DATA_IN;
  usb_control_state.zlp_required = false;

  if (length == 0) {
    // ZLP送信の場合（SET_ADDRESSなど）
    LOG_INFO("Sending ZLP (Zero Length Packet)\r\n");
    usb_write_packet(0, NULL, 0);
  } else if (data == NULL) {
    // データがあるのにポインタがNULLの場合はエラー
    LOG_ERROR("ERROR: NULL data pointer with non-zero length\r\n");
    usb_control_stall();
    return;
  } else {
    LOG_INFO("Control send data: length=%d, buffer=0x%08X\r\n", length,
             (uint32_t)data);
    usb_send_contorl_packet();
  }
}

static void usb_send_contorl_packet(void) {
  uint16_t packet_size;
  uint16_t remaining =
      usb_control_state.data_length - usb_control_state.data_sent;

  LOG_DEBUG("Send control packet: remaining=%d, sent=%d, total=%d\r\n",
            remaining, usb_control_state.data_sent,
            usb_control_state.data_length);

  if (remaining > 0) {
    packet_size = (remaining > 64) ? 64 : remaining;

    LOG_DEBUG("Sending packet: size=%d, sent=%d, total=%d\r\n", packet_size,
              usb_control_state.data_sent, usb_control_state.data_length);

    usb_write_packet(
        0, usb_control_state.data_buffer + usb_control_state.data_sent,
        packet_size);
    usb_control_state.data_sent += packet_size;

    if (packet_size == 64 &&
        usb_control_state.data_sent == usb_control_state.data_length) {
      usb_control_state.zlp_required = true;
      LOG_DEBUG("ZLP will be required\r\n");
    }
  } else if (usb_control_state.zlp_required) {
    LOG_DEBUG("Sending ZLP\r\n");
    usb_write_packet(0, NULL, 0);
    usb_control_state.zlp_required = false;
  } else {
    LOG_DEBUG("Data transfer complete - preparing for STATUS OUT\r\n");
    usb_control_state.state = USB_CTRL_STATE_STATUS_OUT;
    usb_prepare_ep0_out_status();
  }
}
static uint8_t current_configuration = 0;
static void usb_process_set_configuration(USB_SetupPacket *setup) {
  uint8_t configuration_value = setup->wValue & 0xff;

  LOG_INFO("SET_CONFIGURATION: configuration_value=0x%02X\r\n",
           configuration_value);

  if (configuration_value == 1) {
    USB_OUTEP[1].DOEPCTL =
        USB_OTG_DOEPCTL_EPTYP_0 | (196 << USB_OTG_DOEPCTL_MPSIZ_Pos);
    LOG_INFO("Audio streaming endpoint enabled\r\n");
    current_configuration = 1;
    usb_control_send_data(NULL, 0);
  } else if (configuration_value == 0) {
    LOG_INFO("Configuration 0 set (unconfigured)\r\n");
    current_configuration = 0;
    usb_control_send_data(NULL, 0);
  } else {
    LOG_ERROR("Invalid configuration value: 0x%02X\r\n", configuration_value);
    usb_control_stall();
  }
}

static void usb_process_set_interface(USB_SetupPacket *setup) {
  uint8_t interface_num = setup->wIndex & 0xff;
  uint8_t alternate_setting = setup->wValue & 0xff;

  LOG_INFO("SET_INTERFACE: interface_num=0x%02X, alternate_setting=0x%02X\r\n",
           interface_num, alternate_setting);

  if (interface_num == 0) {
    // Audio Control Interface - 常にalternate setting 0のみ
    if (alternate_setting == 0) {
      LOG_INFO("Setting interface 0, alternate setting 0\r\n");
      usb_control_send_data(NULL, 0);
    } else {
      LOG_ERROR("Invalid alternate setting for interface 0: 0x%02X\r\n",
                alternate_setting);
      usb_control_stall();
    }
  } else if (interface_num == 1) {
    // Audio Streaming Interface - ここでエンドポイント制御
    if (alternate_setting == 0) {
      // Alt 0: ゼロ帯域幅（エンドポイント無効化）
      USB_OUTEP[1].DOEPCTL &= ~USB_OTG_DOEPCTL_EPENA;
      USB_OUTEP[1].DOEPCTL &= ~USB_OTG_DOEPCTL_USBAEP;
      LOG_INFO("Interface 1 Alt 0: Zero bandwidth - endpoint disabled\r\n");
      usb_control_send_data(NULL, 0);
    } else if (alternate_setting == 1) {
      // Alt 1: 動作モード（エンドポイント有効化）
      USB_OUTEP[1].DOEPCTL |= USB_OTG_DOEPCTL_USBAEP;
      USB_OUTEP[1].DOEPCTL |= USB_OTG_DOEPCTL_EPTYP_0; // Isochronous
      USB_OUTEP[1].DOEPCTL |= (196 << USB_OTG_DOEPCTL_MPSIZ_Pos);
      USB_OUTEP[1].DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;

      // 受信準備
      USB_OUTEP[1].DOEPTSIZ = (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | 196;

      LOG_INFO("Interface 1 Alt 1: Operational - endpoint enabled\r\n");
      usb_control_send_data(NULL, 0);
    } else {
      LOG_ERROR("Invalid alternate setting for interface 1: 0x%02X\r\n",
                alternate_setting);
      usb_control_stall();
    }
  } else {
    LOG_ERROR("Invalid interface number: 0x%02X\r\n", interface_num);
    usb_control_stall();
  }
}

static void usb_process_get_status(USB_SetupPacket *setup) {
  uint8_t status[2] = {0x00, 0x00};
  LOG_INFO("GET_STATUS\r\n");
  usb_control_send_data(status, sizeof(status));
}

static void usb_process_standard_request(USB_SetupPacket *setup) {
  LOG_INFO("Standard request: bRequest=0x%02X\r\n", setup->bRequest);

  switch (setup->bRequest) {
  case 0x00:
    // GET_STATUS
    usb_process_get_status(setup);
    break;

  case 0x05:
    // SET_ADDRESS
    usb_process_set_address(setup);
    break;

  case 0x06:
    // GET_DESCRIPTOR
    usb_process_get_descriptor(setup);
    break;

  case 0x08: // GET_CONFIGURATION
    LOG_INFO("GET_CONFIGURATION: returning %d\r\n", current_configuration);
    static uint8_t config_response;
    config_response = current_configuration;
    usb_control_send_data(&config_response, 1);
    break;

  case 0x09:
    // SET_CONFIGURATION
    usb_process_set_configuration(setup);
    break;

  case 0x0b:
    // SET_INTERFACE
    usb_process_set_interface(setup);
    break;

  default:
    LOG_WARN("Unsupported standard request: 0x%02X\r\n", setup->bRequest);
    usb_control_stall();
    break;
  }
}

static void usb_process_audio_request(USB_SetupPacket *setup) {
  LOG_INFO("Audio request: bRequest=0x%02X\r\n", setup->bRequest);
  switch (setup->bRequest) {
  case 0x01:
    // SET_CUR
    LOG_INFO("SET_CUR\r\n");
    usb_control_send_data(NULL, 0);
    break;

  case 0x81:
    // GET_CUR
    static uint8_t dummy_data[4] = {0x00, 0x00, 0x00, 0x00};
    LOG_INFO("GET_CUR\r\n");
    usb_control_send_data(dummy_data, setup->wLength > 4 ? 4 : setup->wLength);
    break;

  default:
    LOG_WARN("Unsupported audio request: 0x%02X\r\n", setup->bRequest);
    usb_control_stall();
    break;
  }
}

static void usb_process_vendor_request(USB_SetupPacket *setup) {
  static uint8_t keep_alive_data[4] = {0xaa, 0xbb, 0xcc, 0xdd};

  switch (setup->bRequest) {
  case 0x01:
    // keep alive request
    LOG_INFO("keep alive request\r\n");
    usb_control_send_data(keep_alive_data, 4);
    break;

  default:
    LOG_WARN("Unsupported vendor request: 0x%02X\r\n", setup->bRequest);
    usb_control_stall();
    break;
  }
}

static void usb_process_setup(USB_SetupPacket *setup) {
  USB_SETUP("bmRequestType=0x%02X, bRequest=0x%02X\r\n", setup->bmRequestType,
            setup->bRequest);
  USB_SETUP("Recipient: %s, Type: %s\r\n",
            (setup->bmRequestType & 0x1F) == 0   ? "Device"
            : (setup->bmRequestType & 0x1F) == 1 ? "Interface"
            : (setup->bmRequestType & 0x1F) == 2 ? "Endpoint"
                                                 : "Other",
            (setup->bmRequestType & 0x60) == 0x00   ? "Standard"
            : (setup->bmRequestType & 0x60) == 0x20 ? "Class"
                                                    : "Other");

  usb_control_state.state = USB_CTRL_STATE_SETUP;
  usb_control_state.data_length = setup->wLength;
  usb_control_state.data_sent = 0;

  switch (setup->bmRequestType & 0x60) {
  case 0x00:
    // standard request
    usb_process_standard_request(setup);
    break;

  case 0x20:
    // class request
    LOG_INFO("Audio class request\r\n");
    uac2_process_audio_request(setup);
    break;

  case 0x40:
    // vendor request
    LOG_INFO("Vendor class request\r\n");
    usb_process_vendor_request(setup);
    break;

  default:
    LOG_WARN("Unsupported request type: 0x%02X\r\n",
             setup->bmRequestType & 0x60);
    usb_control_stall();
    break;
  }
}

static void usb_process_set_address(USB_SetupPacket *setup) {
  uint8_t device_addr = setup->wValue & 0x7f;

  LOG_INFO("SET_ADDRESS: device_addr=0x%02X\r\n", device_addr);

  if (device_addr <= 0x7f && setup->wIndex == 0 && setup->wLength == 0) {
    USB_DEVICE->DCFG = (USB_DEVICE->DCFG & ~USB_OTG_DCFG_DAD) |
                       ((uint32_t)device_addr << USB_OTG_DCFG_DAD_Pos);

    LOG_INFO("Address immediately set to 0x%02X, DCFG=0x%08X\r\n", device_addr,
             USB_DEVICE->DCFG);

    usb_control_send_data(NULL, 0);
  } else {
    LOG_WARN("Invalid SET_ADDRESS request\r\n");
    usb_control_stall();
  }
}

// 残りの関数は変更なし
static void usb_handle_setup(uint32_t epnum) {
  USB_SetupPacket setup;

  usb_read_packet((uint8_t *)&setup, 8);

  if (epnum == 0) {
    usb_process_setup(&setup);
  }
}

static void usb_read_packet(uint8_t *dest, uint32_t bcnt) {
  uint32_t nwords = (bcnt + 3) / 4;
  uint32_t data;
  uint32_t *fifo = USB_FIFO(0);

  for (uint32_t i = 0; i < nwords; i++) {
    data = *fifo;
    if (dest != NULL) {
      for (uint8_t j = 0; j < 4 && (i * 4 + j) < bcnt;
           j++) { // 境界チェック追加
        dest[i * 4 + j] = (uint8_t)(data >> (j * 8));
      }
    }
  }
}

static void usb_write_packet(uint8_t epnum, uint8_t *src, uint16_t len) {
  uint32_t nwords = (len + 3) / 4;
  uint32_t *fifo = USB_FIFO(epnum);
  uint32_t data;

  LOG_DEBUG("Write packet: epnum=%d, len=%d\r\n", epnum, len);

  // DIEPTSIZ設定
  USB_INEP[epnum].DIEPTSIZ = (1 << USB_OTG_DIEPTSIZ_PKTCNT_Pos) | len;

  // 送信開始
  USB_INEP[epnum].DIEPCTL |= USB_OTG_DIEPCTL_EPENA | USB_OTG_DIEPCTL_CNAK;

  // データをFIFOに書き込み
  if (len > 0 && src != NULL) {
    LOG_INFO("Writing data to FIFO:\r\n");
    for (uint32_t i = 0; i < nwords; i++) {
      data = 0;
      for (uint8_t j = 0; j < 4; j++) {
        if (i * 4 + j < len) {
          data |= (uint32_t)(src[i * 4 + j]) << (j * 8);
        }
      }
      *fifo = data;
    }
  }
}

static void usb_handle_rxflvl(void) {
  uint32_t grxstsp = USB_OTG_FS->GRXSTSP;
  uint32_t pktsts =
      (grxstsp & USB_OTG_GRXSTSP_PKTSTS) >> USB_OTG_GRXSTSP_PKTSTS_Pos;
  uint32_t bcnt = (grxstsp & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;
  uint32_t epnum = grxstsp & USB_OTG_GRXSTSP_EPNUM;

  LOG_DEBUG("RXFLVL: EP%d, PKTSTS=0x%X, BCNT=%d\r\n", epnum, pktsts, bcnt);

  switch (pktsts) {
  case PKTSTS_SETUP_RECEIVED: // SETUP受信
    usb_handle_setup(epnum);
    break;

  case PKTSTS_OUT_DATA_RECEIVED: // OUT DATA受信
    USB_DATA("OUT DATA received\r\n");
    if (bcnt > 0) {
      usb_read_packet(NULL, bcnt);
    }
    break;

  case PKTSTS_OUT_COMPLETE: // OUT転送完了
    LOG_DEBUG("OUT transfer completed\r\n");
    usb_prepare_ep0_out_status();
    break;

  case PKTSTS_SETUP_COMPLETE: // SETUP転送完了
    LOG_DEBUG("SETUP transfer completed\r\n");
    break;

  default:
    LOG_WARN("Unknown packet status: 0x%02X\r\n", pktsts);
    if (bcnt > 0) {
      usb_read_packet(NULL, bcnt);
    }
    break;
  }
}

static void usb_handle_ep0_in_complete(void) {
  LOG_DEBUG("EP0 IN complete - State: %d, Sent: %d, Total: %d\r\n",
            usb_control_state.state, usb_control_state.data_sent,
            usb_control_state.data_length);

  if (usb_control_state.state == USB_CTRL_STATE_DATA_IN) {
    usb_send_contorl_packet();
  }
}

static void usb_handle_ep0_out_complete(void) {
  if (usb_control_state.state == USB_CTRL_STATE_STATUS_OUT) {
    usb_control_state.state = USB_CTRL_STATE_IDLE;
    LOG_DEBUG("EP0 out status complete - Control transfer done\r\n");
  }
}

void usb_prepare_ep0_out_status(void) {
  USB_OUTEP[0].DOEPTSIZ = (1 << USB_OTG_DOEPTSIZ_PKTCNT_Pos) | 0; // ZLP受信用
  USB_OUTEP[0].DOEPCTL |= USB_OTG_DOEPCTL_EPENA | USB_OTG_DOEPCTL_CNAK;
  LOG_INFO("EP0 out status prepared\r\n");
}

static void usb_handle_iepint(void) {
  uint32_t daint = USB_DEVICE->DAINT & USB_OTG_DAINT_IEPINT;

  for (uint8_t ep = 0; ep < 4; ep++) {
    if (daint & (1 << ep)) {
      uint32_t diepint = USB_INEP[ep].DIEPINT;

      if (diepint & USB_OTG_DIEPINT_XFRC) {
        USB_INEP[ep].DIEPINT = USB_OTG_DIEPINT_XFRC;
        LOG_DEBUG("EP%d IN transfer completed\r\n", ep);

        if (ep == 0) {
          usb_handle_ep0_in_complete();
        }
      }
    }
  }
}

static void usb_handle_oepint(void) {
  uint32_t daint =
      (USB_DEVICE->DAINT & USB_OTG_DAINT_OEPINT) >> USB_OTG_DAINT_OEPINT_Pos;

  for (uint8_t ep = 0; ep < 4; ep++) {
    if (daint & (1 << ep)) {
      uint32_t doepint = USB_OUTEP[ep].DOEPINT;

      if (doepint & USB_OTG_DOEPINT_XFRC) {
        USB_OUTEP[ep].DOEPINT = USB_OTG_DOEPINT_XFRC;
        LOG_DEBUG("EP%d out transfer completed\r\n", ep);

        if (ep == 0) {
          usb_handle_ep0_out_complete();
        }
      }

      if (doepint & USB_OTG_DOEPINT_STUP) {
        USB_OUTEP[ep].DOEPINT = USB_OTG_DOEPINT_STUP;
        LOG_INFO("EP%d out setup phase done\r\n", ep);
      }
    }
  }
}

void OTG_FS_IRQHandler(void) {
  uint32_t gintsts = USB_OTG_FS->GINTSTS & USB_OTG_FS->GINTMSK;

  if (gintsts & USB_OTG_GINTSTS_USBRST) {
    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_USBRST;
    LOG_INFO("USB reset\r\n");
    USB_DEVICE->DAINTMSK =
        (1 << USB_OTG_DAINTMSK_IEPM_Pos) | (1 << USB_OTG_DAINTMSK_OEPM_Pos);
    USB_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;
    USB_INEP[0].DIEPCTL |= USB_OTG_DIEPCTL_USBAEP;
    USB_OUTEP[0].DOEPCTL |= USB_OTG_DOEPCTL_USBAEP;

    usb_control_state.state = USB_CTRL_STATE_IDLE;
  }

  if (gintsts & USB_OTG_GINTSTS_RXFLVL) {
    usb_handle_rxflvl();
  }

  if (gintsts & USB_OTG_GINTSTS_IEPINT) {
    usb_handle_iepint();
  }

  if (gintsts & USB_OTG_GINTSTS_OEPINT) {
    usb_handle_oepint();
  }
}
