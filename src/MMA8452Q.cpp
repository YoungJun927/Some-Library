/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 CoXlab Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @author Jongsoo Jeong (CoXlab)
 * @date 2016. 6. 13.
 */

#include "MMA8452Q.hpp"

void MMA8452Q::begin(TwoWire &w, uint8_t addr, int8_t pinInt1, int8_t pinInt2) {
  this->wire = &w;
  this->addr = addr;

  if (pinInt1 >= 0) {
    pinMode(pinInt1, INPUT_PULLUP);
    attachInterrupt(pinInt1, EventInterrupt1, this, FALLING);
  }

  if (pinInt2 >= 0) {
    pinMode(pinInt2, INPUT_PULLUP);
    attachInterrupt(pinInt2, EventInterrupt2, this, FALLING);
  }
}

uint8_t MMA8452Q::readSensorId() {
  return read(REG_WHO_AM_I);
}

void MMA8452Q::readXYZ(int16_t *x, int16_t *y, int16_t *z) {
  wire->beginTransmission(addr);
  wire->write(REG_OUT_X_MSB);
  wire->endTransmission(false);

  wire->requestFrom(addr, 6);

  uint16_t valX = (wire->read() << 4);
  valX |= wire->read();
  uint16_t valY = (wire->read() << 4);
  valY |= wire->read();
  uint16_t valZ = (wire->read() << 4);
  valZ |= wire->read();

  if (x) {
    if (bitRead(valX, 11) == 1) {
      valX |= 0xF000;
    }
    *x = (int16_t) valX;
  }

  if (y) {
    if (bitRead(valY, 11) == 1) {
      valY |= 0xF000;
    }
    *y = (int16_t) valY;
  }
  if (z) {
    if (bitRead(valZ, 11) == 1) {
      valZ |= 0xF000;
    }
    *z = (int16_t) valZ;
  }
}

bool MMA8452Q::isActive() {
  uint8_t ctrl1 = read(REG_CTRL1);

  return bitRead(ctrl1, 0);
}

void MMA8452Q::setActive() {
  uint8_t ctrl1 = read(REG_CTRL1);
  if (bitRead(ctrl1, 0) == 1) {
    return; //already active
  }

  bitSet(ctrl1, 0);
  write(REG_CTRL1, ctrl1);
}

void MMA8452Q::setStandby() {
  uint8_t ctrl1 = read(REG_CTRL1);
  if (bitRead(ctrl1, 0) == 0) {
    return; //already standby
  }

  bitClear(ctrl1, 0);
  write(REG_CTRL1, ctrl1);
}

void MMA8452Q::setMode(Mode_t mode) {
  uint8_t ctrl2 = read(REG_CTRL2) & ~(0x03);
  ctrl2 |= mode;
  write(REG_CTRL2, ctrl2);
}

MMA8452Q::Mode_t MMA8452Q::getMode() {
  uint8_t ctrl2 = read(REG_CTRL2);
  return (Mode_t) (ctrl2 & 0x03);
}

MMA8452Q::ODR_t MMA8452Q::getODR() {
  uint8_t ctrl1 = read(REG_CTRL1);
  return (ODR_t) ((ctrl1 & (bit(5) | bit(4) | bit(3))) >> 3);
}

void MMA8452Q::setODR(ODR_t val) {
  bool active = false;
  uint8_t ctrl1 = read(REG_CTRL1);
  if (bitRead(ctrl1, 0) == 1) {
    bitClear(ctrl1, 0);
    write(REG_CTRL1, ctrl1);
    active = true;
  }

  ctrl1 &= ~(bit(5) | bit(4) | bit(3));
  ctrl1 |= (((uint8_t) val << 3) & (bit(5) | bit(4) | bit(3)));

  if (active) {
    bitSet(ctrl1, 0);
  }
  write(REG_CTRL1, ctrl1);
}

void MMA8452Q::onDetectTransient(void (*func)(MMA8452Q &)) {
  handlerTransientDetection = func;
}

static const uint32_t maxDuration[8][4] = {
  {  319000,   319000, 319000,   319000 }, //ODR: 800 Hz
  {  638000,   638000, 638000,   638000 }, //ODR: 400 Hz
  { 1280000,  1280000, 638000,  1280000 }, //ODR: 200 Hz
  { 2550000,  2550000, 638000,  2550000 }, //ODR: 100 Hz
  { 5100000,  5100000, 638000,  5100000 }, //ODR: 50 Hz
  { 5100000, 20400000, 638000, 20400000 }, //ODR: 12.5 Hz
  { 5100000, 20400000, 638000, 40800000 }, //ODR: 6.25 Hz
  { 5100000, 20400000, 638000, 40800000 }, //ODR: 1.56 Hz
};

static const uint32_t step[8][4] = {
  {  1250,  1250, 1250,   1250 }, //ODR: 800 Hz
  {  2500,  2500, 2500,   2500 }, //ODR: 400 Hz
  {  5000,  5000, 2500,   5000 }, //ODR: 200 Hz
  { 10000, 10000, 2500,  10000 }, //ODR: 100 Hz
  { 20000, 20000, 2500,  20000 }, //ODR: 50 Hz
  { 20000, 80000, 2500,  80000 }, //ODR: 12.5 Hz
  { 20000, 80000, 2500, 160000 }, //ODR: 6.25 Hz
  { 20000, 80000, 2500, 160000 }, //ODR: 1.56 Hz
};

void MMA8452Q::setTransientDetection(bool enableX,
                                     bool enableY,
                                     bool enableZ,
                                     uint16_t thresholdMilliG,
                                     uint32_t durationMicros) {
  bool active = false;
  uint8_t ctrl1 = read(REG_CTRL1);
  if (bitRead(ctrl1, 0) == 1) {
    bitClear(ctrl1, 0);
    write(REG_CTRL1, ctrl1);
    active = true;
  }

  write(REG_TRANSIENT_CFG,
        (1 << 4) | //Event flag latch enabled
        ((enableZ) ? (1 << 3) : 0) |
        ((enableY) ? (1 << 2) : 0) |
        ((enableZ) ? (1 << 1) : 0));

  thresholdMilliG = min(thresholdMilliG, 8000);
  write(REG_TRANSIENT_THS, round(thresholdMilliG / 63));

  uint8_t mode = read(REG_CTRL2) & (bit(1) | bit(0));
  ODR_t odr = getODR();

  durationMicros = min(durationMicros, maxDuration[odr][mode]);
  write(REG_TRANSIENT_COUNT, round(durationMicros / step[odr][mode]));

  // Enable transient detection interrupt and route to INT1.
  write(REG_CTRL4, (1 << 5));
  write(REG_CTRL5, (1 << 5));

  if (active) {
    bitSet(ctrl1, 0);
  }
  write(REG_CTRL1, ctrl1);
}

void MMA8452Q::read(uint8_t reg, uint8_t len, uint8_t *dst) {
  if (dst == NULL)
    return;

  wire->beginTransmission(addr);
  wire->write(reg);
  wire->endTransmission(false);

  wire->requestFrom(addr, len);

  uint8_t i;
  for (i = 0; i < len; i++) {
    dst[i] = wire->read();
  }
}

void MMA8452Q::write(uint8_t reg, uint8_t len, const uint8_t *val) {
  wire->beginTransmission(addr);
  wire->write(reg);
  uint8_t i;
  for (i = 0; i < len; i++) {
    wire->write(val[i]);
  }
  wire->endTransmission(true);
}

void MMA8452Q::EventInterrupt1(void *ctx) {
  MMA8452Q *gyro = (MMA8452Q *) ctx;

  uint8_t intSrc = gyro->read(REG_INT_SOURCE);

  if (intSrc == (1 << 2)) {
    /* TODO: Freefall-motion */
  } else if (intSrc == (1 << 3)) {
    /* TODO: Pulse */
  } else if (intSrc == (1 << 4)) {
    /* TODO: Landscape/portrait orientation */
  } else if (intSrc == (1 << 5)) {
    /* Transient */
    gyro->read(REG_TRANSIENT_SRC); //clear interrupt
    if (gyro->handlerTransientDetection) {
      gyro->handlerTransientDetection(*gyro);
    }
  }
}

void MMA8452Q::EventInterrupt2(void *ctx) {
  MMA8452Q *gyro = (MMA8452Q *) ctx;

  uint8_t intSrc = gyro->read(REG_INT_SOURCE);

  if (intSrc == (1 << 0)) {
    /* TODO: Data ready */
  } else if (intSrc == (1 << 7)) {
    /* TODO: Auto SLEEP/WAKE */
  }
}
