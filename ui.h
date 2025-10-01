#ifndef UI_H
#define UI_H

#include "global.h"
#include "lang.h"
#include "bno08x_task.h"  // AJOUTER CETTE LIGNE
#include <lvgl.h>
#include "lvgl_v8_port.h"

// =============================================================================
// INTERFACE UTILISATEUR PRINCIPALE
// =============================================================================

static lv_obj_t* main_container = NULL;
static lv_obj_t* data_label = NULL;
static lv_obj_t* imu_label = NULL;  // NOUVELLE ZONE POUR IMU
static lv_obj_t* metar_label = NULL;

// Declaration forward
void update_main_interface();

/**
 * @brief Affiche l'interface principale du variometre
 */
void show_main_interface() {
  lvgl_port_lock(-1);

  lv_obj_clean(lv_scr_act());

  main_container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(main_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(main_container, 0, 0);
  lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(main_container, lv_color_hex(0x0a0a0a), 0);
  lv_obj_set_style_border_width(main_container, 0, 0);
  lv_obj_set_style_pad_all(main_container, 20, 0);
  lv_obj_set_style_radius(main_container, 0, 0);

  // Titre
  lv_obj_t* title = lv_label_create(main_container);
  lv_label_set_text(title, tr(KEY_VARIOMETER_ACTIVE));
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00ff88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  // Zone de donnees principales (BMP390)
  data_label = lv_label_create(main_container);
  lv_obj_set_width(data_label, LV_HOR_RES - 40);
  lv_label_set_long_mode(data_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(data_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(data_label, lv_color_hex(0xffffff), 0);
  lv_obj_align(data_label, LV_ALIGN_TOP_LEFT, 0, 50);

  // Zone IMU (BNO08x) - NOUVEAU
  imu_label = lv_label_create(main_container);
  lv_obj_set_width(imu_label, LV_HOR_RES - 40);
  lv_label_set_long_mode(imu_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(imu_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(imu_label, lv_color_hex(0xffaa00), 0);  // Orange
  lv_obj_align(imu_label, LV_ALIGN_TOP_LEFT, 0, 180);

  // Zone METAR
  metar_label = lv_label_create(main_container);
  lv_obj_set_width(metar_label, LV_HOR_RES - 40);
  lv_label_set_long_mode(metar_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_font(metar_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(metar_label, lv_color_hex(0x88ff88), 0);
  lv_obj_align(metar_label, LV_ALIGN_BOTTOM_LEFT, 0, -10);

  lvgl_port_unlock();

  // Premiere mise a jour
  update_main_interface();
}

/**
 * @brief Met a jour les donnees affichees
 */
/**
 * @brief Met a jour les donnees affichees
 */
void update_main_interface() {
  if (!data_label || !imu_label || !metar_label) return;

  if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    char data_text[512];
    char imu_text[512];
    char metar_text[256];

    // Formatage des donnees de vol (BMP390)
    const char* vario_sign = (flight_data.vario_ms >= 0) ? "+" : "";

    snprintf(data_text, sizeof(data_text),
             "=== BMP390 ===\n"
             "Temperature: %.1f C\n"
             "Pression: %.1f hPa\n"
             "Altitude QNH: %.0f m\n"
             "Altitude QNE: %.0f m\n"
             "Hauteur sol: %.0f m\n"
             "Vario BMP: %s%.2f m/s",
             flight_data.temperature,
             flight_data.pressure_hpa,
             flight_data.altitude_qnh,
             flight_data.altitude_qne,
             flight_data.altitude_qfe,
             vario_sign,
             flight_data.vario_ms);

    // Formatage donnees IMU (BNO08x)
    bno08x_data_t imu_data;
    if (get_bno08x_data(&imu_data)) {
      const char* vario_imu_sign = (imu_data.vario_imu >= 0) ? "+" : "";

      snprintf(imu_text, sizeof(imu_text),
               "=== BNO08x IMU ===\n"
               "Vario IMU: %s%.2f m/s\n"
               "G-metre: %.2f G\n"
               "Accel verticale: %.2f m/s2",
               vario_imu_sign,
               imu_data.vario_imu,
               imu_data.g_force,
               imu_data.vertical_accel);
    } else {
      snprintf(imu_text, sizeof(imu_text),
               "=== BNO08x IMU ===\n"
               "En attente donnees...");
    }

    // Formatage METAR
    snprintf(metar_text, sizeof(metar_text),
             "QNH: %.1f hPa (%s)\nMETAR: %s",
             flight_data.qnh_pressure,
             metar_data.station_id,
             metar_data.raw_metar);

    xSemaphoreGive(dataMutex);

    // Mise a jour LVGL
    lvgl_port_lock(-1);
    lv_label_set_text(data_label, data_text);
    lv_label_set_text(imu_label, imu_text);
    lv_label_set_text(metar_label, metar_text);
    lvgl_port_unlock();
  }
}

/**
 * @brief Cree un ecran de test simple
 */
void show_test_screen() {
  lvgl_port_lock(-1);

  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

  lv_obj_t* container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_center(container);
  lv_obj_set_style_bg_color(container, lv_color_hex(0x1a1a2e), 0);

  lv_obj_t* label = lv_label_create(container);
  lv_label_set_text(label, "TEST SCREEN");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
  lv_obj_center(label);

  lvgl_port_unlock();
}

#endif  // UI_H