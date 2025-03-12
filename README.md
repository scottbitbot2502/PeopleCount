# ESP32-Paxcounter
![logo](docs/assets/paxcounter_logo_white.png)

**Wifi & Bluetooth driven, LoRaWAN enabled, Paxcounter and multi-sensor appliance, built on cheap ESP32 LoRa IoT boards**

---

**Ready-to-go Hardware**: <a href="https://de.aliexpress.com/item/32915894264.html" target="_blank">LILYGO® Paxcounter LoRa</a>

**Documentation**: <a href="https://cyberman54.github.io/ESP32-Paxcounter" target="_blank">https://cyberman54.github.io/ESP32-Paxcounter</a>

**Source Code**: <a href="https://github.com/cyberman54/ESP32-Paxcounter" target="_blank">https://github.com/cyberman54/ESP32-Paxcounter</a>

---

# Use case

Paxcounter is an [ESP32](https://www.espressif.com/en/products/socs/esp32) MCU based device for metering passenger flows and multi-sensor data in realtime. It counts how many mobile devices are around. This gives an estimation how many people are around. Paxcounter detects Wifi and Bluetooth signals in the air, focusing on mobile devices by evaluating their MAC adresses. In parallel, it reads and stores data from multiple connected environment sensors.

Intention of this project is to do this without intrusion in privacy: You don't need to track people owned devices, if you just want to count them. Therefore, Paxcounter does not persistenly store MAC adresses and does no kind of fingerprinting the scanned devices.

Data can either be stored on a local SD-card, transferred to cloud using LoRaWAN network (e.g. TheThingsNetwork or Helium) or MQTT over TCP/IP, or transmitted to a local host using serial (SPI) interface.

You can build this project battery powered using ESP32 deep sleep mode and reach long uptimes with a single 18650 Li-Ion cell.

# License



   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

NOTICE:
Parts of the source files in this repository are made available under different licenses,
see file <A HREF="https://github.com/cyberman54/ESP32-Paxcounter/blob/master/LICENSE">LICENSE.txt</A> in this repository. Refer to each individual source file for more details.

