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
* Change references to Air Quality Throughout file (top text especially)
* set building and room number to 'debug' when value is not given
* Create label in recording screen to display error statuses
* check if the plot can just be refreshed without loading new buffer (with flag for refresh without time scale change)

#### CAD
Modified [ghfisanotti's CYD Case on Thingiverse](https://www.thingiverse.com/thing:7047135), licensed under CC BY-SA 3.0. See CAD folder in this repo for the STL and STEP files.

#### Required Parts
* 1x ESP-32 CYD
* 1x MH-Z19C CO2 Sensor
* 1x 3.7 V Lipo w/ JST 1.25mm connector *optional
* 2x 1.25mm 4-pin JST to Dupont connectors
* 1x CYD_LID (3D printed)
* 1x CYD_BASE (3D printed)
* 4x M3 heat-set inserts
* 4x M3x8 bolts
* 1x [push button](https://www.amazon.com/Waterproof-Momentary-Button-Switch-Colors/dp/B07F24Y1TB) *optional, for zeroing the sensor
* 1x 5-pin female pin header *optional, for securing the sensor to the base

The 5-pin female pin header is secured by melting the plastic around it with a soldering iron

#### Stretch Goals
Eventually I want to take this project and use it as a template to be a smart power meter instead.
* measure current using a clamp, get input for voltage, calculate power, energy, test using a 3d printer (good plotting since motors will spike and drop current while active)
[AC Line Splitter](https://www.digikey.com/en/products/detail/klein-tools-inc/69409/6597020)
[AC Current Sensor](https://www.digikey.com/en/products/detail/dfrobot/SEN0211/6588615)