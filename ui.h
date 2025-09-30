#ifndef UI_H
#define UI_H

#include "global.h"
#include "lang.h"
#include <lvgl.h>
#include "lvgl_v8_port.h"

// =============================================================================
// INTERFACE UTILISATEUR PRINCIPALE
// =============================================================================

/**
 * @brief Affiche l'interface principale du variometre
 */
void show_main_interface() {
  lvgl_port_lock(-1);

  // Nettoyer l'ecran
  lv_obj_clean(lv_scr_act());

  // Container principal
  lv_obj_t* main_container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(main_container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(main_container, 0, 0);
  lv_obj_clear_flag(main_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(main_container, lv_color_hex(0x0a0a0a), 0);
  lv_obj_set_style_border_width(main_container, 0, 0);
  lv_obj_set_style_pad_all(main_container, 20, 0);

  // ===== TITRE =====
  lv_obj_t* title = lv_label_create(main_container);
  lv_label_set_text(title, tr(KEY_VARIOMETER_ACTIVE));
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00ff88), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  // ===== INFORMATIONS PRINCIPALES =====
  lv_obj_t* info_label = lv_label_create(main_container);
  char info_text[512];
  
  snprintf(info_text, sizeof(info_text),
           "QNH: %.1f hPa\nStation: %s (%.1f km)\nWiFi: %s\n",
           flight_data.qnh_pressure,
           metar_data.station_id,
           metar_data.distance_km,
           wifi_connected ? "Connecte" : "Hors ligne");

  lv_label_set_text(info_label, info_text);
  lv_obj_set_style_text_font(info_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(info_label, lv_color_hex(0xffffff), 0);
  lv_obj_align(info_label, LV_ALIGN_TOP_LEFT, 0, 0);

  // ===== METAR BRUT =====
  lv_obj_t* metar_label = lv_label_create(main_container);
  lv_obj_set_width(metar_label, LV_HOR_RES - 40);
  lv_label_set_long_mode(metar_label, LV_LABEL_LONG_WRAP);
  
  char metar_text[300];
  snprintf(metar_text, sizeof(metar_text), "METAR:\n%s", metar_data.raw_metar);
  
  lv_label_set_text(metar_label, metar_text);
  lv_obj_set_style_text_font(metar_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(metar_label, lv_color_hex(0x88ff88), 0);
  lv_obj_align(metar_label, LV_ALIGN_TOP_LEFT, 0, 120);

  lvgl_port_unlock();
}

/**
 * @brief Cree un ecran de test simple
 */
void show_test_screen() {
  lvgl_port_lock(-1);

  lv_obj_clean(lv_scr_act());

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

/**
 * @brief Met a jour les donnees affichees
 */
void update_main_interface() {
  // TODO: Mise a jour dynamique
}

#endif  // UI_H