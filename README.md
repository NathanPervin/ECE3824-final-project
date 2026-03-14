# Air Quality Monitor

### Description
ESP-32 device that monitors the air quality and sends the infomation to a database in the cloud. A website will display the data.

### Impact
[Studies](https://www.sciencedirect.com/science/article/pii/S036013232300358X) have shown that short-term exposure to high levels of CO2 can reduce cognitive performance and negatively impact learning. Therefore, it is important to be ensure that CO2 levels remain low in an academic setting.

### Sensing Parameters
1. Temperature
2. Humidity
3. Barometric Pressure
4. Air Quality (particulates)
5. CO2

### Components
BME680 Sensor (Temperature/Humidity/Barometric Pressure/VOC Gas Detection) - SPI + 3.3/5 V\
[by waveshare](https://www.amazon.com/Environmental-Temperature-Barometric-Detection-Raspberry/dp/B0BZ4W6J49)

MH-Z19C CO2 Sensor - TX/RX ([more info](https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19c-pins-type-co2-manual-ver1_0.pdf)) \
[by EC Buying](https://www.amazon.com/EC-Buying-Monitoring-Concentration-Detection/dp/B0CRKH5XVX)

ESP32 Cheap Yellow Display ([more info](https://www.lcdwiki.com/2.8inch_ESP32-32E_Display))\
[by Hosyond](https://www.amazon.com/gp/product/B0D92C9MMH)

3.7V LiPo Battery\
[by MakerHawk](https://www.amazon.com/gp/aw/d/B0D7MC714N)

#### Ideas
* CYD has a microsd card slot -> maybe create an offline data collection mode when a wifi network is unavailible, then have an on-screen option to upload data to the cloud
* Use CYD touch screen to set modes of data collection - 24-hour ambient, single session with start and stop button
* CYD touch screen numpad to enter a room number for COE
* Screen should display the current values and give a air quality score.
* variable data upload rate to save power
* calibrate CO2 sensor by pulling IO35 low (requires hardware switching of the wire)

#### Features
* Screen automatically sleeps after 60 seconds of inactivity

#### Info
* CYD has battery port which also charges the battery when the USB-C port is powered.

#### Technologies
* Django for web app

#### Web Hosting
* AWS

#### Data Collection and Upload
use the CYD's microSD card slot to store text files with each line containing JSON Lines. The timestamp will be updated after each data POST request to keep the timestamps accurate over long periods of time. Data will be in the following format:
```JSON
{"mode":"ambient","building":"COE","room_number":306,"unix_timestamp":1678886400,"temperature":22.5, ...}
```

#### Pinout

| Pin | Use |
|-----|-----|
| IO35 | PWM for CO2 Sensor |
| 5V | Vin for CO2 Sensor |
| GND | GND for CO2 Sensor |

microSD Card should be formatted as FAT32.

#### Tutorials 
https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r

SD card example:
https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/Examples/Basics/3-SDCardTest/3-SDCardTest.ino

[LVGL Widgets Docs](https://docs.lvgl.io/master/widgets/)

#### TODO
* display recording duration if session mode was selected
* display the 4 other vars using labels
* create stop button (logs whatever data is cached and goes back to start screen)