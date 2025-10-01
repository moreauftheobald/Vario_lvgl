#ifndef BNO08X_TASK_H
#define BNO08X_TASK_H

#include "global.h"
#include <Adafruit_BNO08x.h>
#include <math.h>

// =============================================================================
// CONFIGURATION BNO08X
// =============================================================================

#define BNO08X_TASK_STACK 4096
#define BNO08X_TASK_PRIORITY 4
#define BNO08X_UPDATE_RATE_MS 50  // 20 Hz

// =============================================================================
// STRUCTURE DONNEES BNO08X
// =============================================================================

typedef struct {
  // Accelerations totales (m/s²)
  float accel_x;
  float accel_y;
  float accel_z;
  
  // Accelerations lineaires (sans gravite) (m/s²)
  float linear_accel_x;
  float linear_accel_y;
  float linear_accel_z;
  
  // Vecteur gravite normalise (direction du "bas")
  float gravity_x;
  float gravity_y;
  float gravity_z;
  
  // Valeurs calculees
  float vertical_accel;     // Acceleration verticale (m/s²)
  float vario_imu;          // Vario calcule par IMU (m/s)
  float g_force;            // G-metre (G)
  
  bool valid;
  uint32_t timestamp;
} bno08x_data_t;

// =============================================================================
// VARIABLES GLOBALES BNO08X
// =============================================================================

static Adafruit_BNO08x bno08x;
static TaskHandle_t bno08xTaskHandle = NULL;
static bool bno08x_initialized = false;
static bno08x_data_t bno08x_data = {0};

// Variables pour integration vario
static float prev_vertical_velocity = 0.0f;
static uint32_t prev_vario_time = 0;
static bool first_vario_reading = true;

// Filtre pour vecteur gravite (moyennage)
static float gravity_filtered_x = 0.0f;
static float gravity_filtered_y = 0.0f;
static float gravity_filtered_z = 0.0f;
static bool gravity_initialized = false;

// =============================================================================
// FONCTIONS CALCUL
// =============================================================================

/**
 * @brief Normalise un vecteur
 */
void normalize_vector(float* x, float* y, float* z) {
  float magnitude = sqrt((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
  if (magnitude > 0.01f) {
    *x /= magnitude;
    *y /= magnitude;
    *z /= magnitude;
  }
}

/**
 * @brief Produit scalaire
 */
float dot_product(float x1, float y1, float z1, float x2, float y2, float z2) {
  return x1*x2 + y1*y2 + z1*z2;
}

/**
 * @brief Calcule G-metre depuis acceleration lineaire
 */
float calculate_g_force(float ax, float ay, float az) {
  float magnitude = sqrt(ax*ax + ay*ay + az*az);
  return magnitude / 9.81f;
}

/**
 * @brief Integre acceleration pour calculer vario IMU
 */
float integrate_vertical_acceleration(float accel_z, uint32_t current_time) {
  if (first_vario_reading) {
    prev_vertical_velocity = 0.0f;
    prev_vario_time = current_time;
    first_vario_reading = false;
    return 0.0f;
  }
  
  float dt = (current_time - prev_vario_time) / 1000.0f;
  
  if (dt <= 0.0f || dt > 1.0f) {
    prev_vario_time = current_time;
    return prev_vertical_velocity;
  }
  
  // Integration avec filtre anti-derive
  float new_velocity = prev_vertical_velocity + (accel_z * dt);
  new_velocity = 0.98f * new_velocity + 0.02f * (accel_z * dt);
  
  prev_vertical_velocity = new_velocity;
  prev_vario_time = current_time;
  
  return new_velocity;
}

/**
 * @brief Traite les donnees BNO08x avec detection automatique du vertical
 */
void process_bno08x_data(uint32_t current_time) {
  
  // Mise a jour du vecteur gravite (filtre passe-bas)
  // La gravite est l'acceleration totale au repos
  const float alpha = 0.02f;  // Filtre fort pour stabilite
  
  if (!gravity_initialized) {
    gravity_filtered_x = bno08x_data.accel_x;
    gravity_filtered_y = bno08x_data.accel_y;
    gravity_filtered_z = bno08x_data.accel_z;
    gravity_initialized = true;
  } else {
    gravity_filtered_x = alpha * bno08x_data.accel_x + (1.0f - alpha) * gravity_filtered_x;
    gravity_filtered_y = alpha * bno08x_data.accel_y + (1.0f - alpha) * gravity_filtered_y;
    gravity_filtered_z = alpha * bno08x_data.accel_z + (1.0f - alpha) * gravity_filtered_z;
  }
  
  // Normaliser le vecteur gravite
  bno08x_data.gravity_x = gravity_filtered_x;
  bno08x_data.gravity_y = gravity_filtered_y;
  bno08x_data.gravity_z = gravity_filtered_z;
  normalize_vector(&bno08x_data.gravity_x, &bno08x_data.gravity_y, &bno08x_data.gravity_z);
  
  // Le vecteur vertical est l'oppose de la gravite (haut = -gravite)
  float vertical_x = -bno08x_data.gravity_x;
  float vertical_y = -bno08x_data.gravity_y;
  float vertical_z = -bno08x_data.gravity_z;
  
  // Projection de l'acceleration lineaire sur l'axe vertical
  // AccelVertical = AccelLineaire . VerticalUnitaire
  bno08x_data.vertical_accel = dot_product(
    bno08x_data.linear_accel_x, bno08x_data.linear_accel_y, bno08x_data.linear_accel_z,
    vertical_x, vertical_y, vertical_z
  );
  
  // Calcul G-metre
  bno08x_data.g_force = calculate_g_force(
    bno08x_data.linear_accel_x,
    bno08x_data.linear_accel_y,
    bno08x_data.linear_accel_z
  );
  
  // Integration pour vario IMU
  bno08x_data.vario_imu = integrate_vertical_acceleration(
    bno08x_data.vertical_accel, current_time
  );
  
  bno08x_data.timestamp = current_time;
  bno08x_data.valid = true;
}

// =============================================================================
// CONFIGURATION ET INITIALISATION
// =============================================================================

/**
 * @brief Configure les rapports du BNO08x
 */
bool configure_bno08x_reports() {
#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "Configuration rapports");
#endif

  // Accelerometre brut (pour detecter gravite)
  if (!bno08x.enableReport(SH2_ACCELEROMETER, 50000)) {  // 20Hz
#ifdef DEBUG_MODE
    ESP_LOGE("BNO08x", "Erreur activation Accelerometer");
#endif
    return false;
  }

  vTaskDelay(pdMS_TO_TICKS(50));

  // Linear Acceleration (sans gravite)
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION, 50000)) {  // 20Hz
#ifdef DEBUG_MODE
    ESP_LOGE("BNO08x", "Erreur activation Linear Acceleration");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "Rapports actives");
#endif
  return true;
}

/**
 * @brief Initialise le capteur BNO08x sur Wire1
 */
bool init_bno08x() {
#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "Init sur Wire1 (I2C partage avec BMP390)");
#endif

  extern TwoWire I2C_BMP;

  if (!bno08x.begin_I2C(0x4A, &I2C_BMP)) {
#ifdef DEBUG_MODE
    ESP_LOGW("BNO08x", "Essai adresse 0x4B...");
#endif
    if (!bno08x.begin_I2C(0x4B, &I2C_BMP)) {
#ifdef DEBUG_MODE
      ESP_LOGE("BNO08x", "BNO08x introuvable");
#endif
      return false;
    }
  }

#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "BNO08x detecte");
#endif

  vTaskDelay(pdMS_TO_TICKS(100));

  if (!configure_bno08x_reports()) {
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "BNO08x OK");
#endif

  bno08x_initialized = true;
  bno08x_data.valid = false;
  first_vario_reading = true;
  gravity_initialized = false;
  return true;
}

// =============================================================================
// TACHE FREERTOS
// =============================================================================

/**
 * @brief Tache FreeRTOS lecture BNO08x
 */
void bno08x_task(void* parameter) {
#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "Demarrage tache");
#endif

  vTaskDelay(pdMS_TO_TICKS(1000));

  TickType_t last_wake_time = xTaskGetTickCount();
  sh2_SensorValue_t sensor_value;
  
  uint32_t accel_count = 0;
  uint32_t linear_count = 0;

  while (1) {
    bool data_received = false;
    
    if (bno08x_initialized) {
      while (bno08x.getSensorEvent(&sensor_value)) {
        
        switch (sensor_value.sensorId) {
          case SH2_ACCELEROMETER:
            bno08x_data.accel_x = sensor_value.un.accelerometer.x;
            bno08x_data.accel_y = sensor_value.un.accelerometer.y;
            bno08x_data.accel_z = sensor_value.un.accelerometer.z;
            accel_count++;
            data_received = true;
            break;
            
          case SH2_LINEAR_ACCELERATION:
            bno08x_data.linear_accel_x = sensor_value.un.linearAcceleration.x;
            bno08x_data.linear_accel_y = sensor_value.un.linearAcceleration.y;
            bno08x_data.linear_accel_z = sensor_value.un.linearAcceleration.z;
            linear_count++;
            data_received = true;
            break;
        }
      }
    }
    
    if (data_received && accel_count > 0 && linear_count > 0) {
      uint32_t current_time = millis();
      process_bno08x_data(current_time);
    }

#ifdef DEBUG_MODE
    static uint32_t last_debug = 0;
    uint32_t current_time = millis();
    if (current_time - last_debug > 5000) {
      ESP_LOGI("BNO08x", "Accel:%lu Linear:%lu", accel_count, linear_count);
      if (bno08x_data.valid) {
        ESP_LOGI("BNO08x", "Gravity:(%.2f,%.2f,%.2f)", 
                 bno08x_data.gravity_x, bno08x_data.gravity_y, bno08x_data.gravity_z);
        ESP_LOGI("BNO08x", "Vario:%.2f G:%.2f AccZ:%.2f", 
                 bno08x_data.vario_imu, bno08x_data.g_force,
                 bno08x_data.vertical_accel);
      }
      last_debug = current_time;
    }
#endif

    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(BNO08X_UPDATE_RATE_MS));
  }
}

bool create_bno08x_task() {
  if (!bno08x_initialized) {
#ifdef DEBUG_MODE
    ESP_LOGE("BNO08x", "Capteur non init");
#endif
    return false;
  }

  BaseType_t result = xTaskCreate(
    bno08x_task,
    "BNO08x",
    BNO08X_TASK_STACK,
    NULL,
    BNO08X_TASK_PRIORITY,
    &bno08xTaskHandle);

  if (result != pdPASS) {
#ifdef DEBUG_MODE
    ESP_LOGE("BNO08x", "Erreur creation tache");
#endif
    return false;
  }

#ifdef DEBUG_MODE
  ESP_LOGI("BNO08x", "Tache creee");
#endif
  return true;
}

void stop_bno08x_task() {
  if (bno08xTaskHandle != NULL) {
    vTaskDelete(bno08xTaskHandle);
    bno08xTaskHandle = NULL;
  }
}

bool get_bno08x_data(bno08x_data_t* data) {
  if (!bno08x_data.valid || !data) {
    return false;
  }
  memcpy(data, &bno08x_data, sizeof(bno08x_data_t));
  return true;
}

void reset_bno08x_vario() {
  first_vario_reading = true;
  prev_vertical_velocity = 0.0f;
  bno08x_data.vario_imu = 0.0f;
}

#endif  // BNO08X_TASK_H