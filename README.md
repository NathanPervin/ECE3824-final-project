# CO2 Monitor

### Description
ESP-32 device that monitors the CO2 level in the room and sends the infomation to a database in the cloud. A website will display the data.

### Impact
[Studies](https://www.sciencedirect.com/science/article/pii/S036013232300358X) have shown that short-term exposure to high levels of CO2 can reduce cognitive performance and negatively impact learning. Therefore, it is important to be ensure that CO2 levels remain low in an academic setting.

### Components
MH-Z19C CO2 Sensor ([more info](https://www.winsen-sensor.com/d/files/infrared-gas-sensor/mh-z19c-pins-type-co2-manual-ver1_0.pdf)) \
[by EC Buying](https://www.amazon.com/EC-Buying-Monitoring-Concentration-Detection/dp/B0CRKH5XVX)

ESP32 Cheap Yellow Display ([more info](https://www.lcdwiki.com/2.8inch_ESP32-32E_Display))\
[by Hosyond](https://www.amazon.com/gp/product/B0D92C9MMH)

3.7V LiPo Battery\
[by MakerHawk](https://www.amazon.com/gp/aw/d/B0D7MC714N)

#### Ideas
* calibrate CO2 sensor by pulling pin HD on MH-Z19C low via button between ground and the pin. Depress button for 7 seconds to zero the CO2 sensor to 400 ppm (only do this when outdoors)

#### Features
* Screen automatically sleeps after 5 minutes of inactivity
* 1100 mAh battery with charging via the USB-C port (~6hr wireless battery life)

#### Dependencies
* Django for web app
* LVGL for ESP-32 GUI
* TFT_eSPI
* XPT2046_Touchscreen

#### Web Hosting
* AWS

#### Data Collection and Upload
Data will be in the following format:
```JSON
{"mode":"ambient","building":"COE","room_number":306,"unix_timestamp":1678886400,"CO2_ppm":750}
```
The timestamp will be updated after each data POST request to keep the timestamps accurate over long periods of time.

#### Pinout

| Pin | Use |
|-----|-----|
| IO35 | PWM for CO2 Sensor |
| 5V (UART PORT) | Vin for CO2 Sensor |
| GND (UART PORT) | GND for CO2 Sensor |
| GND | GND for button -> CO2 HD Pin |


#### Tutorials 
https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r
[LVGL Widgets Docs](https://docs.lvgl.io/master/widgets/)

#### TODO
* stop button logs whatever data is cached and goes back to start screen
* just display date with time and elapsed time also if in session mode
* also display the current CO2 level under the plot that updates every 1s even for longer logging modes - change color from red to green as well
