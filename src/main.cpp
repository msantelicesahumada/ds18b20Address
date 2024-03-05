#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

OneWire *oneWire;
DallasTemperature *sensors;
DeviceAddress *foundAddresses;
uint8_t expectedDeviceCount;
uint8_t maxAttempts;

void printAddress(const uint8_t address[8]) {
  for (uint8_t i = 0; i < 8; i++) {
    if (address[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(address[i], HEX);
  }
}

float readTemperatureWithAttempts(const DeviceAddress& address) {
  float tempC;
  uint8_t attempts = 0;

  while (attempts < maxAttempts) {
    sensors->requestTemperaturesByAddress(address);
    tempC = sensors->getTempC(address);

    if (tempC != DEVICE_DISCONNECTED_C) {
      break;
    }

    attempts++;
    delay(1000); // Espera antes de intentar leer la temperatura nuevamente
  }

  return tempC;
}

void searchSensors() {
  uint8_t address[8];
  uint8_t deviceCount = 0;

  while (true) {
    Serial.println("Buscando dispositivos OneWire...");
    deviceCount = 0;

    while (oneWire->search(address)) {
      if (deviceCount < expectedDeviceCount) {
        memcpy(foundAddresses[deviceCount], address, 8);
      }
      deviceCount++;
    }

    Serial.print("Se encontraron ");
    Serial.print(deviceCount);
    Serial.println(" dispositivo(s) OneWire.");

    if (deviceCount == expectedDeviceCount) {
      for (uint8_t i = 0; i < deviceCount; i++) {
  Serial.print("Dispositivo: ");
  printAddress(foundAddresses[i]);
  Serial.print(" | ");

  // Realizar una lectura de prueba de temperatura para el sensor actual
  float tempC = readTemperatureWithAttempts(foundAddresses[i]);
  Serial.print("Temperatura: ");
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Desconectado");
  } else {
    Serial.print(tempC);
    Serial.println(" °C");
  }
}

      break;
    } else {
      Serial.println("No se encontró la cantidad esperada de dispositivos OneWire. Reintentando...");
      oneWire->reset_search();
      delay(1000); // Espera antes de volver a buscar dispositivos
    }
  }
}

int readIntWithConfirmation() {
  while (Serial.available() == 0) {
    delay(100);
  }
  int value = Serial.parseInt();
  while (Serial.read() != '\n') {
    delay(10);
  }
  return value;
}

void setup() {
  Serial.begin(115200);

  Serial.println("Ingrese el pin OneWire y presione ENTER:");
  int oneWirePin = readIntWithConfirmation();
  Serial.print("Pin ingresado: ");
  Serial.println(oneWirePin);

  Serial.println("Ingrese la cantidad de sensores esperados y presione ENTER:");
  expectedDeviceCount = readIntWithConfirmation();
  Serial.print("Cantidad de sensores esperados: ");
  Serial.println(expectedDeviceCount);

  Serial.println("Ingrese la cantidad máxima de intentos de lectura de temperatura y presione ENTER:");
  maxAttempts = readIntWithConfirmation();
  Serial.print("Cantidad máxima de intentos: ");
  Serial.println(maxAttempts);

  oneWire = new OneWire(oneWirePin);
  sensors = new DallasTemperature(oneWire);
  sensors->begin();
  foundAddresses = new DeviceAddress[expectedDeviceCount];

  searchSensors();
}

void loop() {
  /* if (Serial.available() > 0 && Serial.read() == '\n') {
  } */
    searchSensors();
    delay(1000);
}