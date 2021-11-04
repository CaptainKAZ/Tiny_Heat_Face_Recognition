#include <SPI.h>
#include <Seeed_FS.h>
#include <TFT_eSPI.h>  // Include the graphics library for visualization
#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX9064X_I2C_Driver.h"
#include "SD/Seeed_SD.h"

/*Tiny jpeg JPG encoding and flie saving*/
void tje_log(String s) { Serial.println(s); }
#define TJE_IMPLEMENTATION
#include "TinyJPEG/tiny_jpeg.h"
uint8_t bitmap[768 * 3];  // Bitmap image

/*Hardware abstract*/
File myFile;
TFT_eSPI tft = TFT_eSPI();

/*File name*/
uint16_t face0Count = 0;
uint16_t face1Count = 0;
uint16_t envCount = 0;
String face0 = String("face0");//CHANGE LABEL HERE
String face1 = String("face1");//CHANGE LABEL HERE
String env = String("env");//CHANGE LABEL HERE

/*MLX90640 params*/
const byte MLX90640_address =
    0x33;           // Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8  // Default shift for MLX90640 in open air
float MLX90640To[768];
uint16_t MLX90640Frame[834];
paramsMLX90640 MLX90640;

#define debug Serial

/*Visualization*/
uint16_t TheColor;
// start with some initial colors
uint16_t MinTemp = 25;
uint16_t MaxTemp = 40;
// vaiable to toggle the display gridr
int ShowGrid = -1;
// size of a display "pixel"
byte BoxWidth = 6;
byte BoxHeight = 6;
float a, b, c, d;

// Custom fwrite
void myFwrite(void *context, void *data, int size) {
  ((File *)context)->File::write((const uint8_t *)data, size);
}

void setup() {
  Wire.begin();
  Wire.setClock(2000000);  // Increase I2C clock speed to 2M
  debug.begin(115200);     // Fast debug as possible

  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  debug.println("helloworld!");

  if (isConnected() == false) {
    debug.println(
        "MLX90640 not detected at default I2C address. Please check wiring. "
        "Freezing.");
    while (1)
      ;
  }

  // Get device parameters - We only have to do this once
  uint16_t eeMLX90640[832];
  int status;
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0) {
    debug.println("Failed to load system parameters");
    while (1)
      ;
  }

  // Extract device parameters
  status = MLX90640_ExtractParameters(eeMLX90640, &MLX90640);
  if (status != 0) {
    debug.println("Parameter extraction failed");
    while (1)
      ;
  }
  // Once params are extracted, we can release eeMLX90640 array
  MLX90640_SetRefreshRate(MLX90640_address, 0x03);  // Set rate to 4Hz

  // Start SD card
  if (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {
    Serial.println("Fail to init SD card!");
    while (1)
      ;
  }

  // Init screen
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  // get the cutoff points for the color interpolation routines
  // note this function called when the temp scale is changed
  Getabcd();
  // draw a legend with the scale that matches the sensors max and min
  DrawLegend();
}

void loop() {
  for (byte x = 0; x < 2; x++) {
//    while (MLX90640_SynchFrame(0x33) != 0) {
//      delay(1);
//    }
    int status = MLX90640_GetFrameData(MLX90640_address, MLX90640Frame);

    float vdd = MLX90640_GetVdd(MLX90640Frame, &MLX90640);
    float Ta = MLX90640_GetTa(MLX90640Frame, &MLX90640);

    float tr = Ta - TA_SHIFT;  // Reflected temperature based on the sensor
                               // ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(MLX90640Frame, &MLX90640, emissivity, tr, MLX90640To);
  }
//  MLX90640_BadPixelsCorrection((&MLX90640)->brokenPixels, MLX90640To,
//                               (MLX90640Frame[832] & 0x1000) >> 5, &MLX90640);

  DisplayGradient();
  tft.setRotation(3);
  if (digitalRead(WIO_KEY_A) == LOW) {
    // Get file name
    while (
        SD.exists(face0 + String(".") + String(face0Count) + String(".jpg"))) {
      face0Count++;
    }
    myFile = SD.open(face0 + String(".") + String(face0Count) + String(".jpg"),
                     FILE_WRITE);
    tft.drawString((String)face0Count + " " + face0, 15, 15);
    face0Count++;
  } else if (digitalRead(WIO_KEY_B) == LOW) {
    // Get file name
    while (
        SD.exists(face1 + String(".") + String(face1Count) + String(".jpg"))) {
      face1Count++;
    }
    myFile = SD.open(face1 + String(".") + String(face1Count) + String(".jpg"),
                     FILE_WRITE);
    tft.drawString((String)face1Count + " " + face1, 15, 15);
    face1Count++;
  } else if (digitalRead(WIO_KEY_C) == LOW) {
    // Get file name
    while (SD.exists(env + String(".") + String(envCount) + String(".jpg"))) {
      envCount++;
    }
    myFile = SD.open(env + String(".") + String(envCount) + String(".jpg"),
                     FILE_WRITE);
    tft.drawString((String)envCount + " " + env, 15, 15);
    envCount++;
  }
  // Fill the bitmap image
  for (uint16_t i = 0; i < 768; i++) {
    float temp = (MLX90640To[i] - 22) * 255 / 15;
    if (temp > 255) {
      temp = 255;
    } else if (temp < 0) {
      temp = 0;
    }
    bitmap[i * 3 + 0] = (uint8_t)temp;
    bitmap[i * 3 + 1] = (uint8_t)temp;
    bitmap[i * 3 + 2] = (uint8_t)temp;
  }

  // Encode & save file
  tje_encode_with_func(myFwrite, &myFile, 3, 32, 24, 3, bitmap);
  myFile.close();
}

// Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected() {
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    return (false);  // Sensor did not ACK
  }
  return (true);
}

// function to display the results
void DisplayGradient() {
  uint8_t row, col;

  tft.setRotation(4);
  // rip through 70 rows
  for (row = 0; row < 24; row++) {
    // fast way to draw a non-flicker grid--just make every 10 MLX90640To 2x2 as
    // opposed to 3x3 drawing lines after the grid will just flicker too much
    if (ShowGrid < 0) {
      BoxWidth = 6;
    } else {
      if ((row % 10 == 9)) {
        BoxWidth = 7;
      } else {
        BoxWidth = 6;
      }
    }
    // then rip through each 70 cols
    for (col = 0; col < 32; col++) {
      // fast way to draw a non-flicker grid--just make every 10 MLX90640To 2x2
      // as opposed to 3x3
      if (ShowGrid < 0) {
        BoxHeight = 6;
      } else {
        if ((col % 10 == 9)) {
          BoxHeight = 7;
        } else {
          BoxHeight = 6;
        }
      }
      // finally we can draw each the 70 x 70 points, note the call to get
      // interpolated color
      tft.fillRect((row * 6) + 15, (col * 6) + 15, BoxWidth, BoxHeight,
                   GetColor(MLX90640To[row * 32 + col]));
    }
  }
}

// my fast yet effective color interpolation routine
uint16_t GetColor(float val) {
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed
    simple linear equations again a 5-6-5 color display will not need accurate
    temp to R G B color calculation

    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html

  */
  float red, green, blue;
  red = constrain(255.0 / (c - b) * val - ((b * 255.0) / (c - b)), 0, 255);

  if ((val > MinTemp) & (val < a)) {
    green = constrain(
        255.0 / (a - MinTemp) * val - (255.0 * MinTemp) / (a - MinTemp), 0,
        255);
  } else if ((val >= a) & (val <= c)) {
    green = 255;
  } else if (val > c) {
    green = constrain(255.0 / (c - d) * val - (d * 255.0) / (c - d), 0, 255);
  } else if ((val > d) | (val < a)) {
    green = 0;
  }

  if (val <= b) {
    blue = constrain(255.0 / (a - b) * val - (255.0 * b) / (a - b), 0, 255);
  } else if ((val > b) & (val <= d)) {
    blue = 0;
  } else if (val > d) {
    blue = constrain(240.0 / (MaxTemp - d) * val - (d * 240.0) / (MaxTemp - d),
                     0, 240);
  }

  // use the displays color mapping function to get 5-6-5 color palet (R=5 bits,
  // G=6 bits, B-5 bits)
  return tft.color565(red, green, blue);
}

// function to get the cutoff points in the temp vs RGB graph
void Getabcd() {
  a = MinTemp + (MaxTemp - MinTemp) * 0.2121;
  b = MinTemp + (MaxTemp - MinTemp) * 0.3182;
  c = MinTemp + (MaxTemp - MinTemp) * 0.4242;
  d = MinTemp + (MaxTemp - MinTemp) * 0.8182;
}

float get_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y) {
  if (x < 0) {
    x = 0;
  }
  if (y < 0) {
    y = 0;
  }
  if (x >= cols) {
    x = cols - 1;
  }
  if (y >= rows) {
    y = rows - 1;
  }
  return p[y * cols + x];
}

void set_point(float *p, uint8_t rows, uint8_t cols, int8_t x, int8_t y,
               float f) {
  if ((x < 0) || (x >= cols)) {
    return;
  }
  if ((y < 0) || (y >= rows)) {
    return;
  }
  p[y * cols + x] = f;
}
// function to draw a legend
void DrawLegend() {
  int16_t j = 0;
  float ii;
  char buf[20];
  // color legend with max and min text

  float inc = (MaxTemp - MinTemp) / 160.0;

  for (ii = MinTemp; ii < MaxTemp; ii += inc) {
    tft.drawFastHLine(260, 200 - j++, 30, GetColor(ii));
  }

  tft.setTextSize(2);
  tft.setCursor(245, 20);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(buf, "%2d/%2d", MaxTemp, (int)(MaxTemp * 1.12) + 32);
  tft.print(buf);

  tft.setTextSize(2);
  tft.setCursor(245, 210);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(buf, "%2d/%2d", MinTemp, (int)(MinTemp * 1.12) + 32);
  tft.print(buf);
}
