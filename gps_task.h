#ifndef GPS_TASK_H
#define GPS_TASK_H

#include "global.h"
#include <Adafruit_GPS.h>

static HardwareSerial GPS_Serial(GPS_UART_NUM);
static Adafruit_GPS GPS(&Serial);

bool init_gps() {
#ifdef DEBUG_MODE
  ESP_LOGI("GPS", "Init GPS (configuration multiplexeur reportee)");
#endif

  // NE PAS toucher au multiplexeur pendant l'init !
  
  GPS_Serial.begin(GPS_BAUD_RATE, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  vTaskDelay(pdMS_TO_TICKS(100));

  GPS.begin(GPS_BAUD_RATE);
  
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  vTaskDelay(pdMS_TO_TICKS(100));
  
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  vTaskDelay(pdMS_TO_TICKS(100));
  
  GPS.sendCommand(PGCMD_ANTENNA);
  vTaskDelay(pdMS_TO_TICKS(100));

#ifdef DEBUG_MODE
  ESP_LOGI("GPS", "GPS configure");
#endif

  gps_initialized = true;
  gps_data.valid = false;
  return true;
}

void gps_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("GPS", "Demarrage tache GPS");
#endif

  // Configuration multiplexeur vers H3 MAINTENANT
  pinMode(GPS_UART_SEL_PIN, OUTPUT);
  digitalWrite(GPS_UART_SEL_PIN, HIGH);  // LOW = H3 externe
  vTaskDelay(pdMS_TO_TICKS(200));

#ifdef DEBUG_MODE
  ESP_LOGI("GPS", "Multiplexeur route vers H3");
#endif

  TickType_t last_wake_time = xTaskGetTickCount();
  
  while (1) {
    char c = GPS.read();
    
    if (GPS.newNMEAreceived()) {
      
      if (!GPS.parse(GPS.lastNMEA())) {
        continue;
      }
      
      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        gps_data.latitude = GPS.latitudeDegrees;
        gps_data.longitude = GPS.longitudeDegrees;
        gps_data.altitude_gps = GPS.altitude;
        gps_data.speed_knots = GPS.speed;
        gps_data.speed_kmh = GPS.speed * 1.852f;
        gps_data.course = GPS.angle;
        gps_data.satellites = GPS.satellites;
        gps_data.fix = GPS.fix;
        gps_data.fix_quality = GPS.fixquality;
        gps_data.timestamp = millis();
        gps_data.valid = GPS.fix;
        
        xSemaphoreGive(dataMutex);
      }

#ifdef DEBUG_MODE
      static uint32_t last_debug = 0;
      uint32_t now = millis();
      if (now - last_debug > 5000) {
        ESP_LOGI("GPS", "Fix:%d Qual:%d Sat:%d",
                 GPS.fix, GPS.fixquality, GPS.satellites);
        
        if (GPS.fix) {
          ESP_LOGI("GPS", "Lat:%.6f Lon:%.6f Alt:%.1fm",
                   GPS.latitudeDegrees, GPS.longitudeDegrees, GPS.altitude);
          ESP_LOGI("GPS", "Speed:%.1fkm/h Course:%.1fdeg",
                   GPS.speed * 1.852f, GPS.angle);
        } else {
          ESP_LOGI("GPS", "En attente fix...");
        }
        last_debug = now;
      }
#endif
    }
    
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1));
  }
}

bool create_gps_task() {
  if (!gps_initialized) {
#ifdef DEBUG_MODE
    ESP_LOGE("GPS", "GPS non init");
#endif
    return false;
  }

  BaseType_t result = xTaskCreate(
    gps_task,
    "GPS",
    GPS_TASK_STACK,
    NULL,
    GPS_TASK_PRIORITY,
    &gpsTaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE("GPS", "Erreur creation tache");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("GPS", "Tache creee");
#endif
  return true;
}

void stop_gps_task() {
  if (gpsTaskHandle != NULL) {
    vTaskDelete(gpsTaskHandle);
    gpsTaskHandle = NULL;
  }
}

#endif // GPS_TASK_H