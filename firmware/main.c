

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <time.h>
#include <util/delay.h>

#include "millis.c"
#include "usbdrv/oddebug.h"
#include "usbdrv/usbdrv.h"

const PROGMEM char
    usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x02, // USAGE (Mouse)
        0xa1, 0x01, // COLLECTION (Application)
        0x09, 0x01, //   USAGE (Pointer)
        0xa1, 0x00, //   COLLECTION (Physical)
        0x05, 0x09, //     USAGE_PAGE (Button)
        0x19, 0x01, //     USAGE_MINIMUM (Button 1)
        0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
        0x15, 0x00, //     LOGICAL_MINIMUM (0)
        0x25, 0x01, //     LOGICAL_MAXIMUM (1)
        0x95, 0x03, //     REPORT_COUNT (3)
        0x75, 0x01, //     REPORT_SIZE (1)
        0x81, 0x02, //     INPUT (Data,Var,Abs)
        0x95, 0x01, //     REPORT_COUNT (1)
        0x75, 0x05, //     REPORT_SIZE (5)
        0x81, 0x03, //     INPUT (Cnst,Var,Abs)
        0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
        0x09, 0x30, //     USAGE (X)
        0x09, 0x31, //     USAGE (Y)
        0x15, 0x81, //     LOGICAL_MINIMUM (-127)
        0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
        0x75, 0x08, //     REPORT_SIZE (8)
        0x95, 0x02, //     REPORT_COUNT (2)
        0x81, 0x06, //     INPUT (Data,Var,Rel)
        0xc0,       //   END_COLLECTION
        0xc0        // END_COLLECTION
};

static uchar reportBuffer[3];

uchar usbFunctionSetup(uchar data[8]) {
  usbRequest_t *rq = (void *)data;

  usbMsgPtr = reportBuffer;
  if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {
    if (rq->bRequest == USBRQ_HID_GET_REPORT) {
      return sizeof(reportBuffer);
    }
  } else {
    /* no vendor specific requests implemented */
  }
  return 0;
}

void usbEventResetReady(void) {}

static unsigned long lastMillis = 0;

uint8_t shouldMove() {
  unsigned long nowMillis = millis();

  if (nowMillis - lastMillis >= 1000) {
    lastMillis = nowMillis;
    return 1;
  }

  return 0;
}

int main(void) {
  srand(time(NULL));

  usbDeviceDisconnect();
  _delay_ms(300);
  usbDeviceConnect();

  usbInit();
  init_millis();
  sei();
  for (;;) {
    usbPoll();
    if (usbInterruptIsReady()) {

      reportBuffer[0] = 0;
      reportBuffer[1] = 0;
      reportBuffer[2] = 0;

      if (shouldMove() == 1) {
        int rand2 = rand() % 2;

        if (rand() % 2 == 0) {
          reportBuffer[1] = (char)rand();
          if (rand2 == 0) {
            reportBuffer[1] = -reportBuffer[1];
          }
        } else {
          reportBuffer[2] = (char)rand();
          if (rand2 == 0) {
            reportBuffer[2] = -reportBuffer[2];
          }
        }
      }

      usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
    }
  }
  return 0;
}
