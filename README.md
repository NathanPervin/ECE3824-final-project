# Air Quality Monitor

### Sensing Parameters
1. Temperature
2. Humidity
3. Barometric Pressure
4. Air Quality (particulates)
5. CO2

### Components
BME680 Sensor (Temperature/Humidity/Barometric Pressure/VOC Gas Detection) - SPI + 3.3/5 V\
[by waveshare](https://www.amazon.com/Environmental-Temperature-Barometric-Detection-Raspberry/dp/B0BZ4W6J49)

MH-Z19C CO2 Sensor - TX/RX\
[by EC Buying](https://www.amazon.com/EC-Buying-Monitoring-Concentration-Detection/dp/B0CRKH5XVX)

ESP32 Cheap Yellow Display ([more info](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display))\
[by Hosyond](https://www.amazon.com/gp/product/B0D92C9MMH)

3.7V LiPo Battery\
[find here](https://www.amazon.com/gp/product/B07BTWK13N)

#### Ideas
* CYD has a microsd card slot -> maybe create an offline data collection mode when a wifi network is unavailible, then have an on-screen option to upload data to the cloud

#### Info
* CYD has battery port which also charges the battery when the USB-C port is powered.

#### Technologies
* Django for web app