#include <Arduino.h>
#include <Wire.h>

#define SLAVE_ADDR ((uint8_t)(0x09u))

typedef enum {
  PULSE_STATE,
  READING_LOW_STATE,
  READING_HIGH_STATE,
  END_STATE,
  TIMEOUT_STATE
} ReadingState;

class Sonar {
private:
  ReadingState state;
  uint32_t startTime;
  uint32_t endTime;
  uint32_t timeoutTime;
  int16_t distanceMm;
  uint8_t trigPin;
  uint8_t echoPin;
  boolean enabled;

public:
  Sonar(void) {
    this->state = END_STATE;
    this->startTime = 0u;
    this->endTime = 0u;
    this->timeoutTime = 0u;
    this->distanceMm = -1;
    this->trigPin = 0xFFu;
    this->echoPin = 0xFFu;
    this->enabled = false;
  }

  void setState(const ReadingState state) {
    this->state = state;
  }
  ReadingState getState(void) const {
    return this->state;
  }

  void setStartTime(const uint32_t startTime) {
    this->startTime = startTime;
  }
  uint32_t getStartTime(void) const {
    return this->startTime;
  }

  void setEndTime(const uint32_t endTime) {
    this->endTime = endTime;
  }
  uint32_t getEndTime(void) const {
    return this->endTime;
  }

  void setTimeoutTime(const uint32_t timeoutTime) {
    this->timeoutTime = timeoutTime;
  }
  uint32_t getTimeoutTime(void) const {
    return this->timeoutTime;
  }

  void setDistanceMm(const int16_t distanceMm) {
    this->distanceMm = distanceMm;
  }
  int16_t getDistanceMm(void) const {
    return this->distanceMm;
  }

  void setTrigPin(const uint8_t trigPin) {
    this->trigPin = trigPin;
  }
  uint8_t getTrigPin(void) const {

    return this->trigPin;
  }

  void setEchoPin(const uint8_t echoPin) {
    this->echoPin = echoPin;
  }
  uint8_t getEchoPin(void) const {
    return this->echoPin;
  }

  void enable(const boolean enabled) {
    this->enabled = enabled;
  }
  boolean isEnabled(void) const {
    return this->enabled;
  }

  virtual ~Sonar() {}
};

#define SONARS ((uint8_t)(6u))

#define WHO_AM_I_CMD ((uint8_t)(0x00u))
#define RESET_CMD ((uint8_t)(0x01u))
#define CONFIG_CMD ((uint8_t)(0x02u))
#define STATUS_CMD ((uint8_t)(0x03u))
#define DATA_0_H_CMD ((uint8_t)(0x04u))
#define DATA_0_L_CMD ((uint8_t)(0x05u))
#define DATA_1_H_CMD ((uint8_t)(0x06u))
#define DATA_1_L_CMD ((uint8_t)(0x07u))
#define DATA_2_H_CMD ((uint8_t)(0x08u))
#define DATA_2_L_CMD ((uint8_t)(0x09u))
#define DATA_3_H_CMD ((uint8_t)(0x0Au))
#define DATA_3_L_CMD ((uint8_t)(0x0Bu))
#define DATA_4_H_CMD ((uint8_t)(0x0Cu))
#define DATA_4_L_CMD ((uint8_t)(0x0Du))
#define DATA_5_H_CMD ((uint8_t)(0x0Eu))
#define DATA_5_L_CMD ((uint8_t)(0x0Fu))
#define EMPTY_CMD ((uint8_t)(0xFFu))

#define READY_DATA_BIT ((uint8_t)(0x07u))
#define TX_BUFFER_SIZE ((uint8_t)(DATA_5_L_CMD + 1))

static Sonar sonars[SONARS];
static uint8_t txBuffer[TX_BUFFER_SIZE];
static uint8_t command;

uint8_t counter;
uint32_t currentTime;

void resetSonars(void) {
  txBuffer[WHO_AM_I_CMD] = SLAVE_ADDR;
  txBuffer[RESET_CMD] = 0u;
  txBuffer[CONFIG_CMD] = 0u;
  txBuffer[STATUS_CMD] = 0b00111111u;

  for (size_t i = 0; i < SONARS; i++) {
    sonars[i].enable(false);
  }

  sonars[0].setTrigPin(2);
  sonars[0].setEchoPin(3);
  sonars[0].enable(true);

  sonars[1].setTrigPin(4);
  sonars[1].setEchoPin(5);
  sonars[1].enable(true);

  sonars[2].setTrigPin(6);
  sonars[2].setEchoPin(7);
  sonars[2].enable(true);

  sonars[3].setTrigPin(8);
  sonars[3].setEchoPin(9);
  sonars[3].enable(true);

  sonars[4].setTrigPin(10);
  sonars[4].setEchoPin(11);
  sonars[4].enable(true);

  sonars[5].setTrigPin(12);
  sonars[5].setEchoPin(13);
  sonars[5].enable(true);

  for (size_t i = 0; i < SONARS; i++) {
    if (sonars[i].isEnabled() == true) {
      pinMode(sonars[i].getTrigPin(), OUTPUT);
      pinMode(sonars[i].getEchoPin(), INPUT);
    }
  }

  counter = 0u;
}

void receiveEvent(int howMany) {
  int commandAttribute;

  if (Wire.available() > 0) {
    command = Wire.read();
    Serial.println(command, HEX);

    switch (command) {
      case WHO_AM_I_CMD:
        break;
      case RESET_CMD:
        /* Reset Device */
        resetSonars();
        break;
      case CONFIG_CMD:
        commandAttribute = Wire.read();
        if (commandAttribute >= 0) {
          for (size_t i = 0; i < SONARS; i++) {
            sonars[i].enable((((uint8_t)commandAttribute) >> i & 0x01u) == true);
          }
          txBuffer[CONFIG_CMD] = ((uint8_t)commandAttribute);
        }
        break;
      case STATUS_CMD:
      case DATA_0_H_CMD:
      case DATA_0_L_CMD:
      case DATA_1_H_CMD:
      case DATA_1_L_CMD:
      case DATA_2_H_CMD:
      case DATA_2_L_CMD:
      case DATA_3_H_CMD:
      case DATA_3_L_CMD:
      case DATA_4_H_CMD:
      case DATA_4_L_CMD:
      case DATA_5_H_CMD:
      case DATA_5_L_CMD:
      case EMPTY_CMD:
        break;

      default:
        break;
    }
  }
}

void requestEvent(void) {
  if (command < TX_BUFFER_SIZE) {
    Wire.write(&txBuffer[command], (TX_BUFFER_SIZE - command));
  } else {
    Wire.write(0);
  }
}

void setup() {
  resetSonars();

  command = WHO_AM_I_CMD;

  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Serial.begin(115200);
}

void loop() {
  if (counter >= SONARS) {
    /* Set READY_DATA_BIT */
    txBuffer[STATUS_CMD] |= (1 << READY_DATA_BIT);
    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].isEnabled() == true) {
        if (sonars[i].getDistanceMm() >= 0) {
          // Serial.print("Distance[");
          // Serial.print(i);
          // Serial.print("] = ");
          // Serial.print(sonars[i].getDistanceMm());
          // Serial.println(" mm");
        } else {
          // Serial.print("TIMEOUT [");
          // Serial.print(i);
          // Serial.print("] = ");
          // Serial.println(sonars[i].getDistanceMm());
        }
      } else {
        sonars[i].setDistanceMm(-2);
      }
      txBuffer[DATA_0_H_CMD + (i * 2)] = (uint8_t)(sonars[i].getDistanceMm() >> 8u);
      txBuffer[DATA_0_L_CMD + (i * 2)] = (uint8_t)(sonars[i].getDistanceMm());
    }
    // Serial.println("-------------------");
    delay(50);

    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].isEnabled() == true) {
        digitalWrite(sonars[i].getTrigPin(), LOW);
      }
    }
    delayMicroseconds(2);
    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].isEnabled() == true) {
        digitalWrite(sonars[i].getTrigPin(), HIGH);
      }
    }
    delayMicroseconds(10);
    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].isEnabled() == true) {
        digitalWrite(sonars[i].getTrigPin(), LOW);
      }
    }

    /* Clear READY_DATA_BIT */
    txBuffer[STATUS_CMD] &= ~(1 << READY_DATA_BIT);
    counter = 0;

    currentTime = micros();
    for (size_t i = 0; i < SONARS; i++) {
      /* Reset timeout */
      if (sonars[i].isEnabled() == true) {
        sonars[i].setTimeoutTime(currentTime);
        sonars[i].setState(READING_LOW_STATE);
      }
    }
  } else {
    counter = 0;
    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].isEnabled() == true) {
        currentTime = micros();
        switch (sonars[i].getState()) {
          case PULSE_STATE:
            counter++;
            break;

          case READING_LOW_STATE:
            if (digitalRead(sonars[i].getEchoPin()) == HIGH) {
              sonars[i].setStartTime(currentTime);
              sonars[i].setTimeoutTime(currentTime);
              sonars[i].setState(READING_HIGH_STATE);
            } else {
              if ((currentTime - sonars[i].getTimeoutTime()) > 1000) {
                sonars[i].setState(TIMEOUT_STATE);
              }
            }
            break;

          case READING_HIGH_STATE:
            if (digitalRead(sonars[i].getEchoPin()) == LOW) {
              sonars[i].setEndTime(currentTime);
              sonars[i].setState(END_STATE);
            } else {
              if ((currentTime - sonars[i].getTimeoutTime()) > 20000) {
                sonars[i].setState(TIMEOUT_STATE);
              }
            }
            break;

          case END_STATE:
            sonars[i].setDistanceMm((int16_t)(((sonars[i].getEndTime() - sonars[i].getStartTime()) * 5) / 29));
            sonars[i].setState(PULSE_STATE);
            break;

          case TIMEOUT_STATE:
            sonars[i].setDistanceMm((int16_t)(-1));
            sonars[i].setState(PULSE_STATE);
            break;

          default:
            break;
        }
      } else {
        counter++;
      }
    }
  }
}