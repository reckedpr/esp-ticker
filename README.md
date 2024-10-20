# ESP32 CYD Ticker Display 📈
An ESP-32 based stock ticker display, inspired by [TickrMeter](https://tickrmeter.com/). Since I had one spare I decided to use the [CYD ESP-32 board](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display). However in the future I may switch to an e-Ink display instead. Currently uses the `finnhub.io` market api since its free plan allows for 60 api calls per minute. There is still ***A LOT*** of polishing and features that need to be added. However it is functional *despite the unoptimised prototype code.*

<img src="https://github.com/reckedpr/esp-ticker/blob/main/pictures/v0.2.0.JPEG?raw=true" alt="esp32 cyd" width="600">

## ⚠ Important
#### The `User_Setup.h` file for `TFT_eSPI` needs to be configured for the correct TFT display! 

👇 A working configuration for the cyd can be found here:

[RuiSantosdotme/ESP32-TFT-Touchscreen/configs/User_Setup.h](https://github.com/RuiSantosdotme/ESP32-TFT-Touchscreen/blob/main/configs/User_Setup.h)

- Copy the contents or download the file
- Navigate to `<project folder>/.pio/libdeps/TFT_eSPI`
- Replace the contents of, or the whole file, `User_Setup.h`

#### Then Ensure that you add these lines to `User_Setup.h`
```c
#define TFT_RGB_ORDER TFT_BGR
#define TFT_INVERSION_ON
```


*If you are using arduino IDE, then you need to head to* `<*>/Arduino/libraries/TFT_eSPI`

\* Arduino Sketchbook folder location. This can be found at  `File > Preferences > Sketchbook location`

## 📜 Features
- [X] Hardware
  - [X] ESP32 CYD Module
  - [ ] Design 3D Printed Enclosure
  - [ ] Add Li-Po Battery
- [x] Display
  - [X] Draw Stock Ticker Symbol
  - [X] Draw Current Stock Price
  - [X] Price % relative to timeframe
  - [X] Colour Indicator
  - [ ] Add date
  - [ ] Introduce alternate layouts
- [x] Backend
  - [X] Local Webserver
    - [X] Form Input
    - [ ] Config Options
    - [ ] Improve UI
  - [X] API Handling
  - [ ] Thorough error handling
  - [ ] Use AsyncHTTPClient instead of HTTPClient

## 🌌 Future Plans
- [ ] Use micro SD for config/themes?
- [ ] Add portrait option?
- [ ] Use E-ink display
- [ ] ~~Switch to raspberyy Pi Zero 2W~~
