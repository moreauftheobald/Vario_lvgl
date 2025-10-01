#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include "global.h"
#include <math.h>

// Configuration Kalman
#define KALMAN_UPDATE_RATE_MS 50  // 20 Hz

typedef struct {
  // Etat [altitude, vitesse_verticale]
  float altitude;
  float vario;
  
  // Matrice covariance (2x2 symetrique)
  float P[2][2];
  
  // Bruit processus
  float Q_altitude;
  float Q_vario;
  
  // Bruit mesures
  float R_altitude_bmp;
  float R_vario_bmp;
  float R_vario_imu;
  
  bool initialized;
  uint32_t last_update;
} kalman_filter_t;

static kalman_filter_t kalman_filter = {0};

/**
 * @brief Initialise le filtre de Kalman
 */
void init_kalman_filter() {
  kalman_filter.altitude = 0.0f;
  kalman_filter.vario = 0.0f;
  
  // Initialisation covariance
  kalman_filter.P[0][0] = 100.0f;  // altitude
  kalman_filter.P[0][1] = 0.0f;
  kalman_filter.P[1][0] = 0.0f;
  kalman_filter.P[1][1] = 10.0f;   // vario
  
  // Bruit processus
  kalman_filter.Q_altitude = 0.05f;  // Reduit de 0.1 à 0.05
  kalman_filter.Q_vario = 0.3f;      // Reduit de 0.5 à 0.3
  
  // Bruit mesures - AJUSTE POUR MOINS DE CONFIANCE AU VARIO BMP
  kalman_filter.R_altitude_bmp = 0.5f;   // Altitude BMP precise (reduit de 1.0)
  kalman_filter.R_vario_bmp = 2.5f;      // Vario BMP tres bruite (augmente de 0.5 à 2.5)
  kalman_filter.R_vario_imu = 0.3f;      // Vario IMU stable (reduit de 0.8 à 0.3)
  
  kalman_filter.initialized = true;
  kalman_filter.last_update = millis();
  
#ifdef DEBUG_MODE
  ESP_LOGI("Kalman", "Filtre initialise");
#endif
}

/**
 * @brief Prediction (propagation temporelle)
 */
void kalman_predict(float accel_vertical, float dt) {
  if (dt <= 0.0f || dt > 1.0f) return;
  
  // Prediction etat
  // altitude = altitude + vario * dt + 0.5 * accel * dt^2
  // vario = vario + accel * dt
  float dt2 = dt * dt;
  
  kalman_filter.altitude += kalman_filter.vario * dt + 0.5f * accel_vertical * dt2;
  kalman_filter.vario += accel_vertical * dt;
  
  // Matrice transition F = [[1, dt], [0, 1]]
  float F[2][2] = {
    {1.0f, dt},
    {0.0f, 1.0f}
  };
  
  // P = F * P * F' + Q
  float P_temp[2][2];
  
  // P_temp = F * P
  P_temp[0][0] = F[0][0] * kalman_filter.P[0][0] + F[0][1] * kalman_filter.P[1][0];
  P_temp[0][1] = F[0][0] * kalman_filter.P[0][1] + F[0][1] * kalman_filter.P[1][1];
  P_temp[1][0] = F[1][0] * kalman_filter.P[0][0] + F[1][1] * kalman_filter.P[1][0];
  P_temp[1][1] = F[1][0] * kalman_filter.P[0][1] + F[1][1] * kalman_filter.P[1][1];
  
  // P = P_temp * F' + Q
  kalman_filter.P[0][0] = P_temp[0][0] * F[0][0] + P_temp[0][1] * F[0][1] + kalman_filter.Q_altitude;
  kalman_filter.P[0][1] = P_temp[0][0] * F[1][0] + P_temp[0][1] * F[1][1];
  kalman_filter.P[1][0] = P_temp[1][0] * F[0][0] + P_temp[1][1] * F[0][1];
  kalman_filter.P[1][1] = P_temp[1][0] * F[1][0] + P_temp[1][1] * F[1][1] + kalman_filter.Q_vario;
}

/**
 * @brief Mise a jour avec altitude BMP
 */
void kalman_update_altitude(float altitude_mesure) {
  // Innovation
  float y = altitude_mesure - kalman_filter.altitude;
  
  // S = H * P * H' + R
  float S = kalman_filter.P[0][0] + kalman_filter.R_altitude_bmp;
  
  // Gain de Kalman K = P * H' * S^-1
  float K[2];
  K[0] = kalman_filter.P[0][0] / S;
  K[1] = kalman_filter.P[1][0] / S;
  
  // Mise a jour etat
  kalman_filter.altitude += K[0] * y;
  kalman_filter.vario += K[1] * y;
  
  // Mise a jour covariance P = (I - K * H) * P
  float P_temp[2][2];
  P_temp[0][0] = kalman_filter.P[0][0];
  P_temp[0][1] = kalman_filter.P[0][1];
  P_temp[1][0] = kalman_filter.P[1][0];
  P_temp[1][1] = kalman_filter.P[1][1];
  
  kalman_filter.P[0][0] = (1.0f - K[0]) * P_temp[0][0];
  kalman_filter.P[0][1] = (1.0f - K[0]) * P_temp[0][1];
  kalman_filter.P[1][0] = P_temp[1][0] - K[1] * P_temp[0][0];
  kalman_filter.P[1][1] = P_temp[1][1] - K[1] * P_temp[0][1];
}

/**
 * @brief Mise a jour avec vario BMP
 */
void kalman_update_vario_bmp(float vario_mesure) {
  // Innovation
  float y = vario_mesure - kalman_filter.vario;
  
  // S = H * P * H' + R (H = [0, 1])
  float S = kalman_filter.P[1][1] + kalman_filter.R_vario_bmp;
  
  // Gain K = P * H' * S^-1
  float K[2];
  K[0] = kalman_filter.P[0][1] / S;
  K[1] = kalman_filter.P[1][1] / S;
  
  // Mise a jour etat
  kalman_filter.altitude += K[0] * y;
  kalman_filter.vario += K[1] * y;
  
  // Mise a jour covariance
  float P_temp[2][2];
  P_temp[0][0] = kalman_filter.P[0][0];
  P_temp[0][1] = kalman_filter.P[0][1];
  P_temp[1][0] = kalman_filter.P[1][0];
  P_temp[1][1] = kalman_filter.P[1][1];
  
  kalman_filter.P[0][0] = P_temp[0][0] - K[0] * P_temp[1][0];
  kalman_filter.P[0][1] = P_temp[0][1] - K[0] * P_temp[1][1];
  kalman_filter.P[1][0] = P_temp[1][0] - K[1] * P_temp[1][0];
  kalman_filter.P[1][1] = P_temp[1][1] - K[1] * P_temp[1][1];
}

/**
 * @brief Mise a jour avec vario IMU
 */
void kalman_update_vario_imu(float vario_mesure) {
  // Innovation
  float y = vario_mesure - kalman_filter.vario;
  
  // S = H * P * H' + R
  float S = kalman_filter.P[1][1] + kalman_filter.R_vario_imu;
  
  // Gain K
  float K[2];
  K[0] = kalman_filter.P[0][1] / S;
  K[1] = kalman_filter.P[1][1] / S;
  
  // Mise a jour etat
  kalman_filter.altitude += K[0] * y;
  kalman_filter.vario += K[1] * y;
  
  // Mise a jour covariance
  float P_temp[2][2];
  P_temp[0][0] = kalman_filter.P[0][0];
  P_temp[0][1] = kalman_filter.P[0][1];
  P_temp[1][0] = kalman_filter.P[1][0];
  P_temp[1][1] = kalman_filter.P[1][1];
  
  kalman_filter.P[0][0] = P_temp[0][0] - K[0] * P_temp[1][0];
  kalman_filter.P[0][1] = P_temp[0][1] - K[0] * P_temp[1][1];
  kalman_filter.P[1][0] = P_temp[1][0] - K[1] * P_temp[1][0];
  kalman_filter.P[1][1] = P_temp[1][1] - K[1] * P_temp[1][1];
}

/**
 * @brief Recupere etat filtre
 */
void get_kalman_state(float* altitude, float* vario) {
  if (altitude) *altitude = kalman_filter.altitude;
  if (vario) *vario = kalman_filter.vario;
}

/**
 * @brief Reset filtre
 */
void reset_kalman_filter() {
  kalman_filter.initialized = false;
  init_kalman_filter();
}

#endif // KALMAN_FILTER_H