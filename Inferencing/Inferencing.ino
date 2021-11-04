/* Edge Impulse Arduino examples
 * Copyright (c) 2021 EdgeImpulse Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Includes ---------------------------------------------------------------- */
#include <TFT_eSPI.h>
#include <Wire.h>
/* INCLUDE YOUR OWN ARDUINO LIB!!!
//#include <heat_inferencing.h>

#include "MLX90640_API.h"
#include "MLX9064X_I2C_Driver.h"

/* Constant defines -------------------------------------------------------- */

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false;  // Set this to true to see e.g. features
                               // generated from the raw signal

const byte MLX90640_address =
    0x33;           // Default 7-bit unshifted address of the MLX90640
#define TA_SHIFT 8  // Default shift for MLX90640 in open air
#define debug Serial
uint16_t eeMLX90640[832];
float MLX90640To[768];
uint16_t MLX90640Frame[834];
paramsMLX90640 MLX90640;
int errorno = 0;
TFT_eSPI tft = TFT_eSPI();
uint16_t MinTemp = 25;
uint16_t MaxTemp = 40;
byte red, green, blue;
// variables for row/column interpolation
byte i, j, k, row, col, incr;
float intPoint, val, a, b, c, d, ii;
byte aLow, aHigh;
byte BoxWidth = 6;
byte BoxHeight = 6;
int x, y;
int ShowGrid = -1;
ei_classifier_smooth_t smooth;

/**
 * @brief      Arduino setup function
 */
void setup() {
  Wire.begin();
  Wire.setClock(2000000);
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Edge Impulse Inferencing Demo");
  if (isConnected() == false) {
    debug.println(
        "MLX90640 not detected at default I2C address. Please check wiring. "
        "Freezing.");
    while (1)
      ;
  }

  int status;
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  errorno = status;  // MLX90640_CheckEEPROMValid(eeMLX90640);//eeMLX90640[10] &
                     // 0x0040;//

  if (status != 0) {
    debug.println("Failed to load system parameters");
    while (1)
      ;
  }

  status = MLX90640_ExtractParameters(eeMLX90640, &MLX90640);
  // errorno = status;
  if (status != 0) {
    debug.println("Parameter extraction failed");
    while (1)
      ;
  }
  MLX90640_SetRefreshRate(MLX90640_address, 0x04);  // Set rate to 16Hz

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // get the cutoff points for the color interpolation routines
  // note this function called when the temp scale is changed
  Getabcd();

  // draw a legend with the scale that matches the sensors max and min
  DrawLegend();
  ei_classifier_smooth_init(&smooth, 3 /* no. of readings */,
                            2 /* min. readings the same */,
                            0.8 /* min. confidence */, 0.3 /* max anomaly */);
}

/**
 * @brief      Printf function uses vsnprintf and output using Arduino Serial
 *
 * @param[in]  format     Variable argument list
 */
void ei_printf(const char *format, ...) {
  static char print_buf[1024] = {0};

  va_list args;
  va_start(args, format);
  int r = vsnprintf(print_buf, sizeof(print_buf), format, args);
  va_end(args);

  if (r > 0) {
    Serial.write(print_buf);
  }
}

/**
 * @brief      Get data and run inferencing
 *
 * @param[in]  debug  Get debug info if true
 */
void loop() {
  ei_printf("Sampling...\n");

  float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {0};

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
  for (uint16_t z = 0; z < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; z++) {
    buffer[z] = (MLX90640To[z] - 22.0) * 255 / 15.0;
    if (buffer[z] > 255) {
      buffer[z] = 255;
    } else if (buffer[z] < 0) {
      buffer[z] = 0;
    }
    buffer[z] = (uint32_t)buffer[z] << 16 | (uint32_t)buffer[z] << 8 |
                (uint32_t)buffer[z];
  }
  DisplayGradient();
  // Turn the raw buffer in a signal which we can the classify
  signal_t signal;
  int err = numpy::signal_from_buffer(
      buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    ei_printf("Failed to create signal from buffer (%d)\n", err);
    return;
  }

  // Run the classifier
  ei_impulse_result_t result = {0};

  err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) {
    ei_printf("ERR: Failed to run classifier (%d)\n", err);
    return;
  }

  // print the predictions
  ei_printf("Predictions ");
  ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification,
            result.timing.anomaly);
  ei_printf(": \n");
  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    ei_printf("    %s: %.5f\n", result.classification[ix].label,
              result.classification[ix].value);
  }
  const char *prediction = ei_classifier_smooth_update(&smooth, &result);
#if EI_CLASSIFIER_HAS_ANOMALY == 1
  ei_printf("    anomaly score: %.3f\n", result.anomaly);
  Serial.println(prediction);
#endif
  tft.setRotation(3);
  tft.drawString(prediction, 15, 15);
}

boolean isConnected() {
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0) {
    return (false);  // Sensor did not ACK
  }
  return (true);
}
// function to display the results
void DisplayGradient() {
  tft.setRotation(4);

  // rip through 70 rows
  for (row = 0; row < 24; row++) {
    // fast way to draw a non-flicker grid--just make every 10 MLX90640To 2x2 as
    // opposed to 3x3 drawing lines after the grid will just flicker too much
    if (ShowGrid < 0) {
      BoxWidth = 6;
    } else {
      if ((row % 10 == 9)) {
        BoxWidth = 4;
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
uint16_t GetColor(float val) {
  /*
    pass in value and figure out R G B
    several published ways to do this I basically graphed R G B and developed
    simple linear equations again a 5-6-5 color display will not need accurate
    temp to R G B color calculation

    equations based on
    http://web-tech.ga-usa.com/2012/05/creating-a-custom-hot-to-cold-temperature-color-gradient-for-use-with-rrdtool/index.html

  */

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

void DrawLegend() {
  // color legend with max and min text
  j = 0;

  float inc = (MaxTemp - MinTemp) / 160.0;

  for (ii = MinTemp; ii < MaxTemp; ii += inc) {
    tft.drawFastHLine(260, 200 - j++, 30, GetColor(ii));
  }

  tft.setTextSize(2);
  tft.setCursor(245, 20);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[20];
  sprintf(buf, "%2d/%2d", MaxTemp, (int)(MaxTemp * 1.12) + 32);
  tft.print(buf);

  tft.setTextSize(2);
  tft.setCursor(245, 210);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  sprintf(buf, "%2d/%2d", MinTemp, (int)(MinTemp * 1.12) + 32);
  tft.print(buf);
}
