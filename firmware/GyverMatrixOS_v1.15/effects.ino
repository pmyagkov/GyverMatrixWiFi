// эффекты

// **************** НАСТРОЙКИ ЭФФЕКТОВ ****************
// эффект "шарики"
#define BALLS_AMOUNT 3    // количество "шариков"
#define CLEAR_PATH 1      // очищать путь
#define BALL_TRACK 1      // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP 70     // длина хвоста шарика (чем больше цифра, тем хвост короче)

// эффект "квадратик"
#define BALL_SIZE 3       // размер квадрата
#define RANDOM_COLOR 1    // случайный цвет при отскоке

// эффект "огонь"
#define SPARKLES 1        // вылетающие угольки вкл выкл
#define HUE_ADD 0         // добавка цвета в огонь (от 0 до 230) - меняет весь цвет пламени

// эффект "кометы"
#define TAIL_STEP 100     // длина хвоста кометы
#define SATURATION 150    // насыщенность кометы (от 0 до 255)
#define STAR_DENSE 60     // количество (шанс появления) комет

// эффект "конфетти"
#define DENSE 3           // плотность конфетти
#define BRIGHT_STEP 70    // шаг уменьшения яркости

// эффект "снег"
#define SNOW_DENSE 10     // плотность снегопада

// эффект "Светляки"
#define LIGHTERS_AM 35    // количество светляков

// --------------------- ДЛЯ РАЗРАБОТЧИКОВ ----------------------

byte hue;

// *********** снегопад 2.0 ***********
void snowRoutine() {
  if (loadingFlag) {
    modeCode = MC_SNOW;
    loadingFlag = false;
    FastLED.clear();  // очистить
  }

  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, SNOW_DENSE) == 0))
      drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1, 0x000000);
  }
}

// ***************************** БЛУДНЫЙ КУБИК *****************************
int coordB[2];
int8_t vectorB[2];
CRGB ballColor;

void ballRoutine() {
  if (loadingFlag) {
    for (byte i = 0; i < 2; i++) {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255, 255);
    }
    modeCode = MC_BALL;
    loadingFlag = false;
  }
  for (byte i = 0; i < 2; i++) {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0) {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255, 255);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0] > (WIDTH - BALL_SIZE) * 10) {
    coordB[0] = (WIDTH - BALL_SIZE) * 10;
    vectorB[0] = -vectorB[0];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255, 255);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1] > (HEIGHT - BALL_SIZE) * 10) {
    coordB[1] = (HEIGHT - BALL_SIZE) * 10;
    vectorB[1] = -vectorB[1];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255, 255);
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (byte i = 0; i < BALL_SIZE; i++)
    for (byte j = 0; j < BALL_SIZE; j++)
      leds[getPixelNumber(coordB[0] / 10 + i, coordB[1] / 10 + j)] = ballColor;
}

// *********** радуга заливка ***********
void rainbowRoutine() {
  if (loadingFlag) {
    modeCode = MC_RAINBOW;
    loadingFlag = false;
    FastLED.clear();  // очистить
  }
  hue += 3;
  for (byte i = 0; i < WIDTH; i++) {
    CRGB thisColor = CHSV((byte)(hue + i * float(255 / WIDTH * 2)), 255, 255);
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, thisColor);   //leds[getPixelNumber(i, j)] = thisColor;
  }
}

// *********** радуга дигональная ***********
void rainbowDiagonalRoutine() {
  if (loadingFlag) {
    modeCode = MC_RAINBOW_DIAG;
    loadingFlag = false;
    FastLED.clear();  // очистить
  }
  hue += 3;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      CRGB thisColor = CHSV((byte)(hue + (float)(WIDTH / HEIGHT * x + y) * (float)(255 / max(WIDTH, HEIGHT))), 255, 255);
      drawPixelXY(x, y, thisColor); //leds[getPixelNumber(i, j)] = thisColor;
    }
  }
}

// ********************** огонь **********************
unsigned char matrixValue[8][16];
unsigned char line[WIDTH];
int pcnt = 0;

//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[8][16] PROGMEM = {
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[8][16] PROGMEM = {
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

void fireRoutine() {
  if (loadingFlag) {
    modeCode = MC_FIRE;
    loadingFlag = false;
    FastLED.clear();
    generateLine();
    memset(matrixValue, 0, sizeof(matrixValue));
  }
  if (pcnt >= 100) {
    shiftUp();
    generateLine();
    pcnt = 0;
  }
  drawFrame(pcnt);
  pcnt += 30;
}


// Randomly generate the next line (matrix row)

void generateLine() {
  for (uint8_t x = 0; x < WIDTH; x++) {
    line[x] = random(64, 255);
  }
}

//shift all values in the matrix up one row

void shiftUp() {
  for (uint8_t y = HEIGHT - 1; y > 0; y--) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x%16;
      if (y > 7) continue;
      matrixValue[y][newX] = matrixValue[y - 1][newX];
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x%16;
    matrixValue[0][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(int pcnt) {
  int nextv;

  //each row interpolates with the one before it
  for (unsigned char y = HEIGHT - 1; y > 0; y--) {
    for (unsigned char x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x%16;
      if (y < 8) {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&(valueMask[y][newX]));

        CRGB color = CHSV(
                       HUE_ADD + pgm_read_byte(&(hueMask[y][newX])), // H
                       255, // S
                       (uint8_t)max(0, nextv) // V
                     );

        leds[getPixelNumber(x, y)] = color;
      } else if (y == 8 && SPARKLES) {
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1) != 0) drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);
      } else if (SPARKLES) {

        // старая версия для яркости
        if (getPixColorXY(x, y - 1) > 0)
          drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);

      }
    }
  }

  //first row interpolates with the "next" line
  for (unsigned char x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x%16;
    CRGB color = CHSV(
                   HUE_ADD + pgm_read_byte(&(hueMask[0][newX])), // H
                   255,           // S
                   (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0) // V
                 );
    //leds[getPixelNumber(newX, 0)] = color; // На форуме пишут что это ошибка - вместо newX должно быть x, иначе
    leds[getPixelNumber(x, 0)] = color;      // на матрицах шире 16 столбцов нижний правый угол неработает
  }
}

// **************** МАТРИЦА *****************
void matrixRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_MATRIX;
    FastLED.clear();
  }
  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor == 0)
      drawPixelXY(x, HEIGHT - 1, 0x00FF00 * (random(0, 10) == 0));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }
}


// ********************************* ШАРИКИ *********************************
int coord[BALLS_AMOUNT][2];
int8_t vector[BALLS_AMOUNT][2];
CRGB ballColors[BALLS_AMOUNT];

void ballsRoutine() {
  if (loadingFlag) {
    modeCode = MC_BALLS;
    loadingFlag = false;
    for (byte j = 0; j < BALLS_AMOUNT; j++) {
      int sign;

      // забиваем случайными данными
      coord[j][0] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0] = random(4, 15) * sign;
      coord[j][1] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1] = random(4, 15) * sign;
      ballColors[j] = CHSV(random(0, 9) * 28, 255, 255);
    }
  }

  if (!BALL_TRACK)    // если режим БЕЗ следов шариков
    FastLED.clear();  // очистить
  else {              // режим со следами
    fader(TRACK_STEP);
  }

  // движение шариков
  for (byte j = 0; j < BALLS_AMOUNT; j++) {

    // движение шариков
    for (byte i = 0; i < 2; i++) {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0) {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0] > (WIDTH - 1) * 10) {
      coord[j][0] = (WIDTH - 1) * 10;
      vector[j][0] = -vector[j][0];
    }
    if (coord[j][1] > (HEIGHT - 1) * 10) {
      coord[j][1] = (HEIGHT - 1) * 10;
      vector[j][1] = -vector[j][1];
    }
    leds[getPixelNumber(coord[j][0] / 10, coord[j][1] / 10)] =  ballColors[j];
  }
}

// функция плавного угасания цвета для всех пикселей
void fader(byte step) {
  for (byte i = 0; i < WIDTH; i++) {
    for (byte j = 0; j < HEIGHT; j++) {
      fadePixel(i, j, step);
    }
  }
}

void fadePixel(byte i, byte j, byte step) {     // новый фейдер
  int pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0) return;

  if (leds[pixelNum].r >= 30 ||
      leds[pixelNum].g >= 30 ||
      leds[pixelNum].b >= 30) {
    leds[pixelNum].fadeToBlackBy(step);
  } else {
    leds[pixelNum] = 0;
  }
}

// ********************* ЗВЕЗДОПАД ******************
void starfallRoutine() {
  if (loadingFlag) {
    modeCode = MC_STARFALL;
    loadingFlag = false;
    FastLED.clear();  // очистить
  }

  // заполняем головами комет левую и верхнюю линию
  for (byte i = HEIGHT / 2; i < HEIGHT; i++) {
    if (getPixColorXY(0, i) == 0
        && (random(0, STAR_DENSE) == 0)
        && getPixColorXY(0, i + 1) == 0
        && getPixColorXY(0, i - 1) == 0)
      leds[getPixelNumber(0, i)] = CHSV(random(0, 200), SATURATION, 255);
  }

  for (byte i = 0; i < WIDTH / 2; i++) {
    if (getPixColorXY(i, HEIGHT - 1) == 0
        && (random(0, STAR_DENSE) == 0)
        && getPixColorXY(i + 1, HEIGHT - 1) == 0
        && getPixColorXY(i - 1, HEIGHT - 1) == 0)
      leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(random(0, 200), SATURATION, 255);
  }

  // сдвигаем по диагонали
  for (byte y = 0; y < HEIGHT - 1; y++) {
    for (byte x = WIDTH - 1; x > 0; x--) {
      drawPixelXY(x, y, getPixColorXY(x - 1, y + 1));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (byte i = HEIGHT / 2; i < HEIGHT; i++) {
    fadePixel(0, i, TAIL_STEP);
  }
  for (byte i = 0; i < WIDTH / 2; i++) {
    fadePixel(i, HEIGHT - 1, TAIL_STEP);
  }
}

// рандомные гаснущие вспышки
void sparklesRoutine() {
  if (loadingFlag) {
    modeCode = MC_SPARKLES;
    loadingFlag = false;
    FastLED.clear();  // очистить
  }
  for (byte i = 0; i < DENSE; i++) {
    byte x = random(0, WIDTH);
    byte y = random(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
      leds[getPixelNumber(x, y)] = CHSV(random(0, 255), 255, 255);
  }
  fader(BRIGHT_STEP);
}

// ********************* БУДИЛЬНИК-РАССВЕТ *********************

int8_t row, col;                   // Для эффекта спирали  - точка "глолвы" змейки, бегающей по спирали (первая змейка для круговой спирали)
int8_t row2, col2;                 // Для эффекта спирали  - точка "глолвы" змейки, бегающей по спирали (вторая змейка для плоской спирали)
int8_t dir, dir2;                  // Для эффекта спирали на плоскости - направление движениия змейки: 0 - вниз; 1 - влево; 2 - вверх; 3 - вправо;
int8_t range[4], range2[4];        // Для эффекта спирали на плоскости - границы разворачивания спирали;
uint16_t tail[8], tail2[8];        // Для эффекта спирали на плоскости - позиции хвоста змейки. HiByte = x, LoByte=y
CHSV tailColor;                    // Цвет последней точки "хвоста" змейки. Этот же цвет используется для предварительной заливки всей матрицы
CHSV tailColor2;                   // Предварительная заливка нужна для корректного отображения часов поверх специальных эффектов будильника
boolean firstRowFlag;              // Флаг начала самого первого ряда первого кадра, чтобы не рисовать "хвост" змейки в предыдущем кадре, которого не было.
byte dawnBrightness;               // Текущая яркость будильника "рассвет"
byte tailBrightnessStep;           // Шаг приращения яркости будильника "рассвет"
byte dawnColorIdx;                 // Индекс в массиве цвета "заливки" матрицы будильника "рассвет" (голова змейки)
byte dawnColorPrevIdx;             // Предыдущий индекс - нужен для корректного цвета отрисовки "хвоста" змейки,
                                   // когда голова начинает новый кадр внизу матрицы, а хвост - вверху от предыдущего кадра
byte step_cnt;                     // Номер шага эффекта, чтобы определить какой длины "хвост" у змейки

// "Рассвет" - от красного к желтому - белому - голубому с плавным увеличением яркости;
// Яркость меняется по таймеру - на каждое срабатывание таймера - +1 к яркости.
// Диапазон изменения яркости - от MIN_DAWN_BRIGHT до MAX_DAWN_BRIGHT (количество шагов)
// Цветовой тон матрицы меняется каждые 16 шагов яркости 255 / 16 -> дает 16 индексов в массиве цветов
// Время таймера увеличения яркости - время рассвета DAWN_NINUTES на количество шагов приращения яркости
byte dawnColorHue[16]  PROGMEM = {0, 16, 28, 36, 44, 52, 57, 62, 64, 66, 66, 64, 62, 60, 128, 128};              // Цвет заполнения - HUE змейки 1
byte dawnColorSat[16]  PROGMEM = {255, 250, 245, 235, 225, 210, 200, 185, 170, 155, 130, 105, 80, 50, 25, 80};   // Цвет заполнения - SAT змейки 1
byte dawnColorHue2[16] PROGMEM = {0, 16, 28, 36, 44, 52, 57, 62, 64, 66, 66, 64, 62, 60, 128, 128};              // Цвет заполнения - HUE змейки 2
byte dawnColorSat2[16] PROGMEM = {255, 250, 245, 235, 225, 210, 200, 185, 170, 155, 130, 105, 80, 50, 25, 80};   // Цвет заполнения - SAT змейки 2

#define MIN_DAWN_BRIGHT   2        // Минимальное значение яркости будильника (с чего начинается)
#define MAX_DAWN_BRIGHT   255      // Максимальное значение яркости будильника (чем заканчивается)
byte DAWN_NINUTES = 20;            // Продолжительность рассыета в минутах

void dawnProcedure() {
  if (loadingFlag) {
    modeCode = MC_DAWN_ALARM;
    dawnBrightness = MIN_DAWN_BRIGHT;

    FastLED.clear();  // очистить
    FastLED.setBrightness(dawnBrightness);

    if (realDawnDuration <= 0 || realDawnDuration > dawnDuration) realDawnDuration = dawnDuration;
    dawnTimer.setInterval(realDawnDuration * 60000L / (MAX_DAWN_BRIGHT - MIN_DAWN_BRIGHT));
  }

  // Пришло время увеличить яркость рассвета?
  if (dawnTimer.isReady() && dawnBrightness < 255) {
    dawnBrightness++;
    FastLED.setBrightness(dawnBrightness);
  }

  byte b_tmp = mapEffectToMode(isAlarming ? alarmEffect : EFFECT_DAWN_ALARM);
  if (b_tmp == 255) b_tmp = DEMO_DAWN_ALARM;
  if (b_tmp == DEMO_DAWN_ALARM) {
    // Если устройство лампа (DEVICE_TYPE == 0) - матрица свернута в "трубу" - рассвет - огонек, бегущий вкруговую по спирали
    // Если устройство плоская матрица в рамке (DEVICE_TYPE == 1) - рассвет - огонек, бегущий по спирали от центра матрицы к краям на плоскости
    b_tmp = DEVICE_TYPE == 0 ? DEMO_DAWN_ALARM_SPIRAL : DEMO_DAWN_ALARM_SQUARE;
  }

  // Если эффект "Лампа" и цвет - черный (остался от "выключено" - цвет лампы - белый
  if (b_tmp == MC_FILL_COLOR && globalColor == 0) {
     globalColor = 0xFFFFFF;
  }

  // Спец.режимы так же как и обычные вызываются в customModes (DEMO_DAWN_ALARM_SPIRAL и DEMO_DAWN_ALARM_SQUARE)
  customModes(b_tmp);

  // Сбрасывать флаг нужно ПОСЛЕ того как инициализированы: И процедура рассвета И применяемый эффект,
  // используемый в качестве рассвета
  loadingFlag = false;
}

// "Рассвет" по спирали, для ламп на круговой матрице (свернутой в трубу)
void dawnLampSpiral() {

  if (loadingFlag) {
    row = 0, col = 0;

    dawnBrightness = MIN_DAWN_BRIGHT;
    tailBrightnessStep = 16;
    firstRowFlag = true;
    dawnColorIdx = 0;
    dawnColorPrevIdx = 0;

    tailColor = CHSV(0, 255, 255 - 8 * tailBrightnessStep);
  }

  boolean flag = true;
  int8_t x=col, y=row;

  if (!firstRowFlag) fillAll(tailColor);

  byte tail_len = min(8, WIDTH - 1);
  for (byte i=0; i<tail_len; i++) {
    x--;
    if (x < 0) { x = WIDTH - 1; y--; }
    if (y < 0) {
      y = HEIGHT - 1;
      flag = false;
      if (firstRowFlag) break;
    }

    byte idx = y > row ? dawnColorPrevIdx : dawnColorIdx;
    byte dawnHue = pgm_read_byte(&(dawnColorHue[idx]));
    byte dawnSat = pgm_read_byte(&(dawnColorSat[idx]));

    tailColor = CHSV(dawnHue, dawnSat, 255 - i * tailBrightnessStep);
    drawPixelXY(x,y, tailColor);
  }

  if (flag) {
    firstRowFlag = false;
    dawnColorPrevIdx = dawnColorIdx;
  }
  if (dawnBrightness == 255 && tailBrightnessStep > 8) tailBrightnessStep -= 2;

  col++;
  if (col >= WIDTH) {
    col = 0; row++;
  }

  if (row >= HEIGHT) row = 0;

  if (col == 0 && row == 0) {
    // Кол-во элементов массива - 16; Шагов яркости - 255; Изменение индекса каждые 16 шагов яркости.
    dawnColorIdx = dawnBrightness >> 4;
  }
}

// "Рассвет" по спирали на плоскости, для плоских матриц
void dawnLampSquare() {

  if (loadingFlag) {
    SetStartPos();

    dawnBrightness = MIN_DAWN_BRIGHT;
    tailBrightnessStep = 16;
    dawnColorIdx = 0;
    step_cnt = 0;

    memset(tail, 0, sizeof(uint16_t) * 8);
    memset(tail2, 0, sizeof(uint16_t) * 8);

    tailColor = CHSV(0, 255, 255 - 8 * tailBrightnessStep);
  }

  int8_t x=col, y=row;
  int8_t x2=col2, y2=row2;

  fillAll(tailColor);

  step_cnt++;

  for (byte i=7; i>0; i--) {
    tail[i]  = tail[i-1];
    tail2[i] = tail2[i-1];
  }
  tail[0]  = (uint)((int)x <<8 | (int)y);
  tail2[0] = (uint)((int)x2<<8 | (int)y2);

  byte dawnHue  = pgm_read_byte(&(dawnColorHue[dawnColorIdx]));
  byte dawnSat  = pgm_read_byte(&(dawnColorSat[dawnColorIdx]));
  byte dawnHue2 = pgm_read_byte(&(dawnColorHue2[dawnColorIdx]));
  byte dawnSat2 = pgm_read_byte(&(dawnColorSat2[dawnColorIdx]));

  for (byte i=0; i < 8; i++) {

    tailColor  = CHSV(dawnHue, dawnSat, 255 - i * tailBrightnessStep);
    tailColor2 = CHSV(dawnHue2, dawnSat2, 255 - i * tailBrightnessStep);

    if (i<=step_cnt) {
      x  = tail[i] >>8; y  = tail[i]  & 0xff;
      x2 = tail2[i]>>8; y2 = tail2[i] & 0xff;
      drawPixelXY(x,  y,  tailColor);
      drawPixelXY(x2, y2, tailColor2);
    }
  }

  if (dawnBrightness == 255 && tailBrightnessStep > 8) tailBrightnessStep -= 2;

  switch(dir) {
    case 0: // вниз;
      row--;
      if (row <= range[dir]) {
        range[dir] = row - 2;
        dir++;
      }
      break;
    case 1: // влево;
      col--;
      if (col <= range[dir]) {
        range[dir] = col - 2;
        dir++;
      }
      break;
    case 2: // вверх;
      row++;
      if (row >= range[dir]) {
        range[dir] = row + 2;
        dir++;
      }
      break;
    case 3: // вправо;
      col++;
      if (col >= range[dir]) {
        range[dir] = col + 2;
        dir = 0;
      }
      break;
  }

  switch(dir2) {
    case 0: // вниз;
      row2--;
      if (row2 <= range2[dir2]) {
        range2[dir2] = row2 - 2;
        dir2++;
      }
      break;
    case 1: // влево;
      col2--;
      if (col2 <= range2[dir2]) {
        range2[dir2] = col2 - 2;
        dir2++;
      }
      break;
    case 2: // вверх;
      row2++;
      if (row2 >= range2[dir2]) {
        range2[dir2] = row2 + 2;
        dir2++;
      }
      break;
    case 3: // вправо;
      col2++;
      if (col2 >= range2[dir2]) {
        range2[dir2] = col2 + 2;
        dir2 = 0;
      }
      break;
  }

  bool out  = (col  < 0 || col  >= WIDTH) && (row  < 0 || row  >= HEIGHT);
  bool out2 = (col2 < 0 || col2 >= WIDTH) && (row2 < 0 || row2 >= HEIGHT);
  if (out && out2) {
    // Кол-во элементов массива - 16; Шагов яркости - 255; Изменение индекса каждые 16 шагов яркости.
    dawnColorIdx = dawnBrightness >> 4;
    SetStartPos();
    step_cnt = 0;
  }
}

void SetStartPos() {
  if (WIDTH % 2 == 1)
  {
    col = WIDTH / 2 + 1;
    col2 = col;
  } else {
    col = WIDTH / 2 - 1;      // 7
    col2 = WIDTH - col - 1 ;  // 8
  }

  if (HEIGHT % 2 == 1)
  {
    row = HEIGHT / 2 + 1;
    row2 = row;
  } else {
    row = HEIGHT / 2 - 1;     // 7
    row2 = HEIGHT - row - 1;  // 8
  }

  dir = 2; dir2 = 0;
  // 0 - вниз; 1 - влево; 2 - вверх; 3 - вправо;
  range[0] = row-2; range[1] = col-2; range[2] = row+2; range[3] = col+2;
  range2[0] = row2-2; range2[1] = col2-2; range2[2] = row2+2; range2[3] = col2+2;
}

// ******************* ЛАМПА ********************

void fillColorProcedure() {
  if (loadingFlag) {
    modeCode = MC_FILL_COLOR;
    loadingFlag = false;
  }

  byte bright =
    isAlarming && !isAlarmStopped
    ? dawnBrightness
    : (specialMode ? specialBrightness : globalBrightness);

  FastLED.setBrightness(bright);

  fillAll(gammaCorrection(globalColor));
}

// ----------------------------- СВЕТЛЯКИ ------------------------------
int lightersPos[2][LIGHTERS_AM];
int8_t lightersSpeed[2][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
byte loopCounter;

int angle[LIGHTERS_AM];
int speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];

void lightersRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_LIGHTERS;
    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);
      lightersSpeed[0][i] = random(-10, 10);
      lightersSpeed[1][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0, 255), 255, 255);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;
  for (byte i = 0; i < map(LIGHTERS_AM,0,255,5,150); i++) {
    if (loopCounter == 0) {     // меняем скорость каждые 255 отрисовок
      lightersSpeed[0][i] += random(-3, 4);
      lightersSpeed[1][i] += random(-3, 4);
      lightersSpeed[0][i] = constrain(lightersSpeed[0][i], -20, 20);
      lightersSpeed[1][i] = constrain(lightersSpeed[1][i], -20, 20);
    }

    lightersPos[0][i] += lightersSpeed[0][i];
    lightersPos[1][i] += lightersSpeed[1][i];

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0) {
      lightersPos[1][i] = 0;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    if (lightersPos[1][i] >= (HEIGHT - 1) * 10) {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
}

// ------------- ПЕЙНТБОЛ -------------

uint8_t USE_SEGMENTS = 1;
uint8_t BorderWidth = 0;
uint8_t dir_mx, seg_num, seg_size, seg_offset;

void lightBallsRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_PAINTBALL;
    FastLED.clear();  // очистить
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами
    BorderWidth = 0;
  }

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(2,64,100));
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  // The color of each point shifts over time, each at a different speed.
  uint32_t ms = millis();
  int16_t idx;

  byte cnt = map(effectSpeed, 0, 255, 1, 4);

  if (USE_SEGMENTS != 0) {
    // Для неквадратных - вычленяем квадратные сегменты, которые равномерно распределяем по ширине / высоте матрицы
    uint8_t  i = beatsin8(  91, 0, seg_size - BorderWidth - 1);
    uint8_t  j = beatsin8( 109, 0, seg_size - BorderWidth - 1);
    uint8_t  k = beatsin8(  73, 0, seg_size - BorderWidth - 1);
    uint8_t  m = beatsin8( 123, 0, seg_size - BorderWidth - 1);

    uint8_t d1 = ms / 29;
    uint8_t d2 = ms / 41;
    uint8_t d3 = ms / 73;
    uint8_t d4 = ms / 97;

    for (uint8_t ii = 0; ii < seg_num; ii++) {
      delay(0); // Для предотвращения ESP8266 Watchdog Timer
      uint8_t cx = dir_mx == 0 ? (seg_offset * (ii + 1) + seg_size * ii) : 0;
      uint8_t cy = dir_mx == 0 ? 0 : (seg_offset * (ii + 1) + seg_size * ii);
      uint8_t color_shift = ii * 50;
      if (cnt <= 1) { idx = XY(i+cx, j+cy); leds[idx] += CHSV( color_shift + d1, 200U, 255U); }
      if (cnt <= 2) { idx = XY(j+cx, k+cy); leds[idx] += CHSV( color_shift + d2, 200U, 255U); }
      if (cnt <= 3) { idx = XY(k+cx, m+cy); leds[idx] += CHSV( color_shift + d3, 200U, 255U); }
      if (cnt <= 4) { idx = XY(m+cx, i+cy); leds[idx] += CHSV( color_shift + d4, 200U, 255U); }

      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      for (byte i2 = cy; i2 < cy + seg_size; i2++) {
        fadePixel(cx + BorderWidth, i2, 15);
        fadePixel(cx + seg_size - BorderWidth - 1, i2, 15);
      }
    }
  }
  else
  {
    uint8_t  i = beatsin8(  91, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  j = beatsin8( 109, BorderWidth, HEIGHT - BorderWidth - 1);
    uint8_t  k = beatsin8(  73, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  m = beatsin8( 123, BorderWidth, HEIGHT - BorderWidth - 1);

    if (cnt <= 1) { idx = XY(i, j); leds[idx] += CHSV( ms / 29, 200U, 255U); }
    if (cnt <= 2) { idx = XY(k, j); leds[idx] += CHSV( ms / 41, 200U, 255U); }
    if (cnt <= 3) { idx = XY(k, m); leds[idx] += CHSV( ms / 73, 200U, 255U); }
    if (cnt <= 4) { idx = XY(i, m); leds[idx] += CHSV( ms / 97, 200U, 255U); }

    if (WIDTH == HEIGHT) {
      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      for (byte i = 0; i < HEIGHT; i++) {
        fadePixel(0, i, 15);
        fadePixel(WIDTH-1, i, 15);
      }
    }
  }
}

// ------------- ВОДОВОРОТ -------------

void swirlRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    modeCode = MC_SWIRL;
    FastLED.clear();  // очистить
    dir_mx = WIDTH > HEIGHT ? 0 : 1;                                 // 0 - квадратные сегменты расположены горизонтально, 1 - вертикально
    seg_num = dir_mx == 0 ? (WIDTH / HEIGHT) : (HEIGHT / WIDTH);     // вычисляем количество сегментов, умещающихся на матрице
    seg_size = dir_mx == 0 ? HEIGHT : WIDTH;                         // Размер квадратного сегмента (высота и ширина равны)
    seg_offset = ((dir_mx == 0 ? WIDTH : HEIGHT) - seg_size * seg_num) / (seg_num + 1); // смещение от края матрицы и между сегментами
    BorderWidth = seg_num == 1 ? 0 : 1;
  }

  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(2,64,100));
  blur2d( leds, WIDTH, HEIGHT, blurAmount);

  uint32_t ms = millis();
  int16_t idx;

  if (USE_SEGMENTS != 0) {
    // Use two out-of-sync sine waves
    uint8_t  i = beatsin8( 41, 0, seg_size - BorderWidth - 1);
    uint8_t  j = beatsin8( 27, 0, seg_size - BorderWidth - 1);

    // Also calculate some reflections
    uint8_t ni = (seg_size-1)-i;
    uint8_t nj = (seg_size-1)-j;

    uint8_t d1 = ms / 11;
    uint8_t d2 = ms / 13;
    uint8_t d3 = ms / 17;
    uint8_t d4 = ms / 29;
    uint8_t d5 = ms / 37;
    uint8_t d6 = ms / 41;

    for (uint8_t ii = 0; ii < seg_num; ii++) {
      delay(0); // Для предотвращения ESP8266 Watchdog Timer
      uint8_t cx = dir_mx == 0 ? (seg_offset * (ii + 1) + seg_size * ii) : 0;
      uint8_t cy = dir_mx == 0 ? 0 : (seg_offset * (ii + 1) + seg_size * ii);
      uint8_t color_shift = ii * 50;

      // The color of each point shifts over time, each at a different speed.
      idx = XY( i+cx, j+cy); leds[idx] += CHSV( color_shift + d1, 200, 192);
      idx = XY(ni+cx,nj+cy); leds[idx] += CHSV( color_shift + d2, 200, 192);
      idx = XY( i+cx,nj+cy); leds[idx] += CHSV( color_shift + d3, 200, 192);
      idx = XY(ni+cx, j+cy); leds[idx] += CHSV( color_shift + d4, 200, 192);
      idx = XY( j+cx, i+cy); leds[idx] += CHSV( color_shift + d5, 200, 192);
      idx = XY(nj+cx,ni+cy); leds[idx] += CHSV( color_shift + d6, 200, 192);

      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      for (byte i2 = cy; i2 < cy + seg_size; i2++) {
        fadePixel(cx, i2, 15);
        fadePixel(cx + BorderWidth, i2, 15);
        fadePixel(cx + seg_size - 1, i2, 15);
        fadePixel(cx + seg_size - BorderWidth - 1, i2, 15);
      }
    }
  }
  else {
    // Use two out-of-sync sine waves
    uint8_t  i = beatsin8( 41, BorderWidth, WIDTH - BorderWidth - 1);
    uint8_t  j = beatsin8( 27, BorderWidth, HEIGHT - BorderWidth - 1);

    // Also calculate some reflections
    uint8_t ni = (WIDTH-1)-i;
    uint8_t nj = (HEIGHT-1)-j;

    // The color of each point shifts over time, each at a different speed.
    idx = XY( i, j); leds[idx] += CHSV( ms / 11, 200, 192);
    idx = XY(ni,nj); leds[idx] += CHSV( ms / 13, 200, 192);
    idx = XY( i,nj); leds[idx] += CHSV( ms / 17, 200, 192);
    idx = XY(ni, j); leds[idx] += CHSV( ms / 29, 200, 192);

    if (HEIGHT == WIDTH) {
      // для квадратных матриц - 6 точек создают более красивую картину
      idx = XY( j, i); leds[idx] += CHSV( ms / 37, 200, 192);
      idx = XY(nj,ni); leds[idx] += CHSV( ms / 41, 200, 192);

      // При соединении матрицы из угла вверх или вниз почему-то слева и справа узора остаются полосы, которые
      // не гаснут обычным blur - гасим полоски левой и правой стороны дополнительно.
      // При соединении из угла влево или вправо или на неквадратных матрицах такого эффекта не наблюдается
      for (byte i = 0; i < HEIGHT; i++) {
        fadePixel(0, i, 15);
        fadePixel(WIDTH-1, i, 15);
      }
    }
  }
}

uint16_t XY(uint8_t x, uint8_t y) {
  return getPixelNumber(x, y);
}
