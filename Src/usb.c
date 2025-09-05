#include "usb.h"
#include "usart.h"
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

static void usb_read_packet(uint8_t *dest, uint32_t bcnt);
static void usb_handle_setup(uint32_t epnum);
static void usb_handle_rxflvl(void);
static void usb_process_setup(USB_SetupPacket *setup);

static void usb_core_reset(void) {
  USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST;
  while (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST)
    ;
}

void usb_init(void) {
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
  usb_core_reset();
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_PWRDWN | USB_OTG_GCCFG_VBUSBSEN;
  USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD;

  USB_OTG_FS->GRXFSIZ = 128;
  USB_OTG_FS->DIEPTXF0_HNPTXFSIZ = (64 << USB_OTG_HPTXFSIZ_PTXFD_Pos) | 128;

  USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBRST | USB_OTG_GINTMSK_RXFLVLM;
  USB_OTG_FS->GAHBCFG |= USB_OTG_GAHBCFG_GINT;
  NVIC_SetPriority(OTG_FS_IRQn, 0);
  NVIC_EnableIRQ(OTG_FS_IRQn);

  USB_DEVICE->DCTL &= ~USB_OTG_DCTL_SDIS;
}

static void usb_process_setup(USB_SetupPacket *setup) {
  printf_usart2("Setup packet: bmRequestType=%02X, bRequest=%02X, wValue=%04X, "
                "wIndex=%04X, wLength=%04X\r\n",
                setup->bmRequestType, setup->bRequest, setup->wValue,
                setup->wIndex, setup->wLength);
}

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
      for (uint8_t j = 0; j < 4; j++) {
        dest[i * 4 + j] = (uint8_t)(data >> (j * 8));
      }
    }
  }
}

static void usb_handle_rxflvl(void) {
  uint32_t grxstsp = USB_OTG_FS->GRXSTSP;
  uint32_t pktsts =
      (grxstsp & USB_OTG_GRXSTSP_PKTSTS) >> USB_OTG_GRXSTSP_PKTSTS_Pos;
  uint32_t bcnt = (grxstsp & USB_OTG_GRXSTSP_BCNT) >> USB_OTG_GRXSTSP_BCNT_Pos;
  uint32_t epnum = grxstsp & USB_OTG_GRXSTSP_EPNUM;

  switch (pktsts) {
  case 0x06:
    usb_handle_setup(epnum);
    break;

  default:
    if (bcnt > 0) {
      usb_read_packet(NULL, bcnt);
    }
    break;
  }
}

void OTG_FS_IRQHandler(void) {
  if (USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_USBRST) {
    USB_OTG_FS->GINTSTS = USB_OTG_GINTSTS_USBRST;
    printf_usart2("USB reset\r\n");
    USB_DEVICE->DCFG &= ~USB_OTG_DCFG_DAD;
    USB_INEP[0].DIEPCTL |= USB_OTG_DIEPCTL_USBAEP;
    USB_OUTEP[0].DOEPCTL |= USB_OTG_DOEPCTL_USBAEP;
  }

  if (USB_OTG_FS->GINTSTS & USB_OTG_GINTSTS_RXFLVL) {
    usb_handle_rxflvl();
  }
}
