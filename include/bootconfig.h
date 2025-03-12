#ifndef _BOOTCONFIG_H
#define _BOOTCONFIG_H

#include "driver/rtc_io.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_wdt.h"
#include "soc/gpio_periph.h"

// Function to disable boot button reset functionality
inline void disableBootReset() {
    // Disable all reset sources
    REG_WRITE(RTC_CNTL_BROWN_OUT_REG, 0);  // Disable brownout detector
    REG_WRITE(RTC_CNTL_WDTCONFIG0_REG, 0); // Disable hardware watchdog
    REG_WRITE(RTC_CNTL_WDTWPROTECT_REG, 0); // Disable watchdog write protection
    
    // Disable GPIO0 reset functionality
    rtc_gpio_deinit(GPIO_NUM_0);
    PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[0]);
    gpio_reset_pin(GPIO_NUM_0);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);
    gpio_deep_sleep_hold_en();
    
    // Disable RTC watchdog timer
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    rtc_wdt_protect_on();
}

#endif // _BOOTCONFIG_H
