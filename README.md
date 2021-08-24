# Solar Lamp/Alarm Clock

## Parts List
Besides the standard fare of a soldering iron, solder, wire and wire strippers/cutters necessary for assembly, this project also uses:

* [Adafruit Feather M0 Wifi](https://www.adafruit.com/product/3010)
* [Adafruit 128x64 OLED FeatherWing](https://www.adafruit.com/product/4650)
* [Adafriut 4-digit Seven segment Featherwing](https://www.adafruit.com/product/3110)
* [Adafruit Music Maker Featherwing w/ Amp](https://www.adafruit.com/product/3436)
* [Adafruit Featherwing Doubler Protoboard](https://www.adafruit.com/product/2890)
* [Adafruit Stemma Qt Mini GPS](https://www.adafruit.com/product/4415)
* [Adafruit Adafruit AHT20 Temperature & Humidity Sensor Breakout Board](https://www.adafruit.com/product/4566)
* [Adafruit NeoPixel Jewel RGBW (Warm)](https://www.adafruit.com/product/2858)
* [Adafruit 12 NeoPixel Ring RGBW (Warm)](https://www.adafruit.com/product/2851)
* [Adafruit I2C Stemma QT Rotary Encoder Board](https://www.adafruit.com/product/4991)
* [2.1mm jack to screw terminal block](https://www.adafruit.com/product/368)
* [5V DC power supply](https://www.adafruit.com/product/276)
* [USB Micro B Male Plug to 5-pin Terminal Block](https://www.adafruit.com/product/3972)
* [TI 74AHCT125](https://www.adafruit.com/product/1787)
* [STEMMA QT / Qwiic 50mm Cable](https://www.adafruit.com/product/4399) x2
* [STEMMA QT / Qwiic 100mm Cable](https://www.adafruit.com/product/4210)
* [40mm 4 Ohm Speaker](https://www.adafruit.com/product/3968) x2
* [Rotary Encoder](https://www.digikey.ca/en/products/detail/bourns-inc/PEC11L-4115F-S0020/4699168)
* [Feather Female Stacking Headers](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2830/5823439) x2
* [Feather Female Headers](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/2886/5823440) x2
* [470 Ohm Resistor](https://www.digikey.ca/en/products/detail/te-connectivity-passive-product/ROX3SJ470R/3477442)
* [1000uF Electrolytic Capacitor](https://www.digikey.ca/en/products/detail/panasonic-electronic-components/EEU-FR0J102/9921020)
* Micro-usb cable
* SD card

## Case
![Clock Case](res/images/ClockRender.png)

## Secrets
The file `src/secrets.h` is used to define the Wifi SSID and password, and is excluded from this repo for obvious reasons. To compile this project, you'll need to make your own, like:
```
#define WLAN_SSID       "Network_SSID"
#define WLAN_PASS       "password"
```