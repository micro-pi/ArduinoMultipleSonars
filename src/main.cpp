#include <Arduino.h>

typedef enum {
  PULSE_STATE,
  READING_LOW_STATE,
  READING_HIGH_STATE,
  END_STATE,
  TIMEOUT_STATE
} ReadingState;

typedef struct {
  const uint8_t trigPin;
  const uint8_t echoPin;
  ReadingState state;
  uint32_t startTime;
  uint32_t endTime;
  uint32_t timeoutTime;
  int16_t distanceMm;
} Sonar;

static Sonar sonars[] = {
    {.trigPin = 10,
     .echoPin = 11},
    {.trigPin = 12,
     .echoPin = 13},
};

#define SONARS ((uint16_t)(sizeof(sonars) / sizeof(Sonar)))

uint8_t counter;
uint32_t currentTime;

void setup() {
  for (size_t i = 0; i < SONARS; i++) {
    sonars[i].state = END_STATE;
    sonars[i].startTime = 0;
    sonars[i].endTime = 0;
    sonars[i].timeoutTime = 0;
    sonars[i].distanceMm = 0;
    pinMode(sonars[i].trigPin, OUTPUT);
    pinMode(sonars[i].echoPin, INPUT);
  }

  counter = 0u;
  Serial.begin(115200);
}

void loop() {
  if (counter >= SONARS) {
    delay(100);
    for (size_t i = 0; i < SONARS; i++) {
      if (sonars[i].distanceMm >= 0) {
        Serial.print("Distance[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.print(sonars[i].distanceMm);
        Serial.println(" mm");
      } else {
        Serial.print("TIMEOUT [");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(sonars[i].timeoutTime);
      }
    }
    Serial.println("-------------------");

    for (size_t i = 0; i < SONARS; i++) {
      digitalWrite(sonars[i].trigPin, LOW);
    }
    delayMicroseconds(2);
    for (size_t i = 0; i < SONARS; i++) {
      digitalWrite(sonars[i].trigPin, HIGH);
    }
    delayMicroseconds(10);
    for (size_t i = 0; i < SONARS; i++) {
      digitalWrite(sonars[i].trigPin, LOW);
    }

    currentTime = micros();
    for (size_t i = 0; i < SONARS; i++) {
      /* Reset timeout */
      sonars[i].timeoutTime = currentTime;
      sonars[i].state = READING_LOW_STATE;
    }
    counter = 0;
  } else {
    counter = 0;
    for (size_t i = 0; i < SONARS; i++) {
      currentTime = micros();
      switch (sonars[i].state) {
        case PULSE_STATE:
          counter++;
          break;

        case READING_LOW_STATE:
          if (digitalRead(sonars[i].echoPin) == HIGH) {
            sonars[i].startTime = currentTime;
            sonars[i].timeoutTime = currentTime;
            sonars[i].state = READING_HIGH_STATE;
          } else {
            if ((currentTime - sonars[i].timeoutTime) > 1000) {
              sonars[i].state = TIMEOUT_STATE;
            }
          }
          break;

        case READING_HIGH_STATE:
          if (digitalRead(sonars[i].echoPin) == LOW) {
            sonars[i].endTime = currentTime;
            sonars[i].state = END_STATE;
          } else {
            if ((currentTime - sonars[i].timeoutTime) > 20000) {
              sonars[i].state = TIMEOUT_STATE;
            }
          }
          break;

        case END_STATE:
          sonars[i].distanceMm = (int16_t)(((sonars[i].endTime - sonars[i].startTime) * 5) / 29);
          sonars[i].state = PULSE_STATE;
          break;

        case TIMEOUT_STATE:
          sonars[i].distanceMm = (int16_t)(-1);
          sonars[i].state = PULSE_STATE;
          break;

        default:
          break;
      }
    }
  }
