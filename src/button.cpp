#ifdef HAS_BUTTON

#include "globals.h"
#include "button.h"

OneButton button(0, true, true);  // GPIO0 is the built-in BOOT button
TaskHandle_t buttonLoopTask;

void IRAM_ATTR readButton(void) { button.tick(); }

void singleClick(void) {
#ifdef HAS_DISPLAY
  dp_refresh(true); // switch to next display page
#endif
#ifdef HAS_MATRIX_DISPLAY
  refreshTheMatrixDisplay(true); // switch to next display page
#endif
}

void buttonLoop(void *parameter) {
  while (1) {
    doIRQ(BUTTON_IRQ);
    delay(50);
  }
}

void button_init(void) {
  ESP_LOGI(TAG, "Starting Button Controller...");

  xTaskCreatePinnedToCore(buttonLoop,      // task function
                         "buttonloop",     // name of task
                         2048,             // stack size of task
                         (void *)1,        // parameter of the task
                         2,                // priority of the task
                         &buttonLoopTask,  // task handle
                         1);               // CPU core

  button.attachClick(singleClick);
  attachInterrupt(digitalPinToInterrupt(0), readButton, CHANGE);
}

#endif
