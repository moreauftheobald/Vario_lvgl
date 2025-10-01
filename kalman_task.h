#ifndef KALMAN_TASK_H
#define KALMAN_TASK_H

#include "global.h"
#include "kalman_filter.h"
#include "bmp390_task.h"
#include "bno08x_task.h"

#define KALMAN_TASK_STACK 4096
#define KALMAN_TASK_PRIORITY 5

static TaskHandle_t kalmanTaskHandle = NULL;

void kalman_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("Kalman", "Demarrage tache");
#endif

  vTaskDelay(pdMS_TO_TICKS(2000));  // Attendre capteurs

  init_kalman_filter();

  TickType_t last_wake_time = xTaskGetTickCount();
  uint32_t last_time = millis();

  // NOUVEAU : Pour plotteur serie
  uint32_t last_plot = 0;

  while (1) {
    uint32_t current_time = millis();
    float dt = (current_time - last_time) / 1000.0f;

    if (dt > 0.0f && dt < 1.0f) {
      // Lire donnees capteurs
      float altitude_bmp = 0.0f;
      float vario_bmp = 0.0f;
      float accel_vertical = 0.0f;
      float vario_imu = 0.0f;

      bno08x_data_t imu_data;
      bool has_imu = get_bno08x_data(&imu_data);

      if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        altitude_bmp = flight_data.altitude_qnh;
        vario_bmp = flight_data.vario_ms;
        xSemaphoreGive(dataMutex);
      }

      if (has_imu) {
        accel_vertical = imu_data.vertical_accel;
        vario_imu = imu_data.vario_imu;
      }

      // Prediction avec acceleration
      kalman_predict(accel_vertical, dt);

      // Mises a jour avec mesures
      if (altitude_bmp != 0.0f) {
        kalman_update_altitude(altitude_bmp);
      }

      if (vario_bmp != 0.0f) {
        kalman_update_vario_bmp(vario_bmp);
      }

      if (has_imu && fabsf(vario_imu) < 50.0f) {  // Filtre aberrations
        kalman_update_vario_imu(vario_imu);
      }

      // Stocker resultats
      get_kalman_state(&kalman_data.altitude_filtered, &kalman_data.vario_filtered);
      kalman_data.kalman_active = true;

      // NOUVEAU : Envoi donnees pour plotteur (toutes les 100ms = 10Hz)
      if (current_time - last_plot >= 100) {
        Serial.print("Vario_BMP:");
        Serial.print(vario_bmp, 1);  // CHANGE: 3 -> 1
        Serial.print(",Vario_IMU:");
        Serial.print(vario_imu, 1);  // CHANGE: 3 -> 1
        Serial.print(",Vario_Kalman:");
        Serial.println(kalman_data.vario_filtered, 1);  // CHANGE: 3 -> 1

        last_plot = current_time;
      }

#ifdef DEBUG_MODE
      static uint32_t last_debug = 0;
      if (current_time - last_debug > 5000) {
        ESP_LOGI("Kalman", "Alt: %.1fm Vario: %.2fm/s",
                 kalman_data.altitude_filtered, kalman_data.vario_filtered);
        last_debug = current_time;
      }
#endif
    }

    last_time = current_time;
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(KALMAN_UPDATE_RATE_MS));
  }
}

bool create_kalman_task() {
  BaseType_t result = xTaskCreate(
    kalman_task,
    "Kalman",
    KALMAN_TASK_STACK,
    NULL,
    KALMAN_TASK_PRIORITY,
    &kalmanTaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE("Kalman", "Erreur creation tache");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("Kalman", "Tache creee");
#endif
  return true;
}

#endif  // KALMAN_TASK_H