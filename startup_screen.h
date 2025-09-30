#ifndef STARTUP_SCREEN_H
#define STARTUP_SCREEN_H

#include <lvgl.h>
#include "global.h"
#include "lang.h"  // Ajout du systÃ¨me de traduction

// Position par defaut : Hayange, Moselle, France
#define DEFAULT_LATITUDE 49.3283  // Hayange latitude
#define DEFAULT_LONGITUDE 6.0627  // Hayange longitude

// Structure pour gerer l'ecran de demarrage
typedef struct {
  lv_obj_t* container;
  lv_obj_t* status_label;
  lv_obj_t* log_label;
  lv_obj_t* progress_bar;
  char log_buffer[2048];
  uint8_t progress_value;
} startup_screen_t;

static startup_screen_t* startup_screen = nullptr;

/**
 * @brief Cree l'ecran de demarrage
 */
void create_startup_screen() {
  lvgl_port_lock(-1);

  // Obtenir les dimensions de l'ecran
  lv_disp_t* disp = lv_disp_get_default();
  lv_coord_t screen_width = lv_disp_get_hor_res(disp);
  lv_coord_t screen_height = lv_disp_get_ver_res(disp);

  // Allocation de la structure
  startup_screen = (startup_screen_t*)malloc(sizeof(startup_screen_t));
  memset(startup_screen, 0, sizeof(startup_screen_t));

  // Container principal avec style
  startup_screen->container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(startup_screen->container, screen_width, screen_height);
  lv_obj_center(startup_screen->container);
  lv_obj_clear_flag(startup_screen->container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(startup_screen->container, lv_color_hex(0x1a1a2e), 0);
  lv_obj_set_style_border_width(startup_screen->container, 0, 0);
  lv_obj_set_style_pad_all(startup_screen->container, 20, 0);

  // Status actuel
  startup_screen->status_label = lv_label_create(startup_screen->container);
  lv_label_set_text(startup_screen->status_label, tr(KEY_INITIALIZATION));
  lv_obj_set_style_text_font(startup_screen->status_label, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(startup_screen->status_label, lv_color_hex(0xffffff), 0);
  lv_obj_align(startup_screen->status_label, LV_ALIGN_TOP_MID, 0, 10);

  // Zone de logs - ETENDUE AU MAXIMUM
  startup_screen->log_label = lv_label_create(startup_screen->container);
  lv_obj_set_size(startup_screen->log_label, screen_width - 40, screen_height - 120);
  lv_obj_align(startup_screen->log_label, LV_ALIGN_TOP_MID, 0, 50);

  // Style fond noir avec bordure verte
  lv_obj_set_style_bg_color(startup_screen->log_label, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(startup_screen->log_label, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(startup_screen->log_label, lv_color_hex(0x00ff88), 0);
  lv_obj_set_style_border_width(startup_screen->log_label, 2, 0);
  lv_obj_set_style_pad_all(startup_screen->log_label, 10, 0);

  // Texte vert visible sur fond noir
  lv_obj_set_style_text_color(startup_screen->log_label, lv_color_hex(0x00ff88), 0);
  lv_obj_set_style_text_font(startup_screen->log_label, &lv_font_montserrat_14, 0);

  // PAS de defilement - affichage statique des dernieres lignes
  lv_label_set_long_mode(startup_screen->log_label, LV_LABEL_LONG_WRAP);

  // Initialiser le buffer et afficher le texte initial
  snprintf(startup_screen->log_buffer, sizeof(startup_screen->log_buffer), "%s\n", tr(KEY_SYSTEM_LOGS));
  lv_label_set_text(startup_screen->log_label, startup_screen->log_buffer);

  // Barre de progression en bas
  startup_screen->progress_bar = lv_bar_create(startup_screen->container);
  lv_obj_set_size(startup_screen->progress_bar, screen_width - 60, 20);
  lv_obj_align(startup_screen->progress_bar, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_set_style_bg_color(startup_screen->progress_bar, lv_color_hex(0x0f0f1e), 0);
  lv_obj_set_style_bg_color(startup_screen->progress_bar, lv_color_hex(0x00ff88), LV_PART_INDICATOR);
  lv_bar_set_range(startup_screen->progress_bar, 0, 100);
  lv_bar_set_value(startup_screen->progress_bar, 0, LV_ANIM_ON);

  lvgl_port_unlock();
}

/**
 * @brief Ajoute une ligne de log a l'ecran
 * @param log_text Texte a ajouter
 * @param is_error true si c'est une erreur (affichage en rouge)
 */
void add_startup_log(const char* log_text, bool is_error = false) {
  if (!startup_screen || !startup_screen->log_label) return;

  lvgl_port_lock(-1);

  // Ajouter le nouveau log au buffer
  char new_line[256];
  if (is_error) {
    snprintf(new_line, sizeof(new_line), tr(KEY_STATUS_ERROR), log_text);
    strcat(new_line, "\n");
  } else {
    snprintf(new_line, sizeof(new_line), tr(KEY_STATUS_OK), log_text);
    strcat(new_line, "\n");
  }

  // Limiter la taille du buffer
  size_t current_len = strlen(startup_screen->log_buffer);
  size_t new_len = strlen(new_line);

  if (current_len + new_len >= sizeof(startup_screen->log_buffer) - 50) {
    // Trouver la premiere ligne a supprimer
    char* first_newline = strchr(startup_screen->log_buffer, '\n');
    if (first_newline) {
      first_newline++;
      memmove(startup_screen->log_buffer, first_newline, strlen(first_newline) + 1);
    } else {
      // Si pas de newline trouvee, vider partiellement
      snprintf(startup_screen->log_buffer, sizeof(startup_screen->log_buffer), "%s\n", tr(KEY_SYSTEM_LOGS));
    }
  }

  strcat(startup_screen->log_buffer, new_line);
  lv_label_set_text(startup_screen->log_label, startup_screen->log_buffer);

  // Changer couleur selon le type
  if (is_error) {
    lv_obj_set_style_text_color(startup_screen->log_label, lv_color_hex(0xff4444), 0);
  } else {
    lv_obj_set_style_text_color(startup_screen->log_label, lv_color_hex(0x44ff44), 0);
  }

  // Forcer le rafraichissement
  lv_obj_invalidate(startup_screen->log_label);

  lvgl_port_unlock();
}

/**
 * @brief Met a jour le statut principal
 */
void update_startup_status(const char* status) {
  if (!startup_screen) return;

  lvgl_port_lock(-1);
  lv_label_set_text(startup_screen->status_label, status);
  lvgl_port_unlock();
}

/**
 * @brief Met a jour la progression
 */
void update_startup_progress(uint8_t progress) {
  if (!startup_screen) return;

  lvgl_port_lock(-1);
  startup_screen->progress_value = progress;
  lv_bar_set_value(startup_screen->progress_bar, progress, LV_ANIM_ON);
  lvgl_port_unlock();
}

/**
 * @brief Detruit l'ecran de demarrage
 */
void destroy_startup_screen() {
  if (!startup_screen) return;

  lvgl_port_lock(-1);

  if (startup_screen->container) {
    lv_obj_del(startup_screen->container);
  }

  free(startup_screen);
  startup_screen = nullptr;

  lv_obj_clean(lv_scr_act());

  lvgl_port_unlock();
}

/**
 * @brief Animation de fin de demarrage avec compte a rebours
 * @param wait_seconds Nombre de secondes a attendre (par defaut 30)
 */
void startup_complete_animation(uint32_t wait_seconds = 30) {
  if (!startup_screen) return;

  update_startup_status(tr(KEY_SYSTEM_READY));
  update_startup_progress(100);

  // Changer les couleurs pour indiquer le succes
  lvgl_port_lock(-1);
  lv_obj_set_style_text_color(startup_screen->status_label, lv_color_hex(0x00ff88), 0);
  lvgl_port_unlock();

  // Ajouter un message de temporisation
  add_startup_log(tr(KEY_LOG_STARTUP_SUCCESS));

  // Compte a rebours
  char countdown_text[128];
  uint32_t start_time = millis();
  uint32_t elapsed_seconds = 0;
  uint32_t remaining_seconds = wait_seconds;

  while (remaining_seconds > 0) {
    elapsed_seconds = (millis() - start_time) / 1000;
    remaining_seconds = (elapsed_seconds < wait_seconds) ? (wait_seconds - elapsed_seconds) : 0;

    snprintf(countdown_text, sizeof(countdown_text), tr(KEY_LAUNCHING_INTERFACE), remaining_seconds);
    update_startup_status(countdown_text);

    vTaskDelay(250);  // Rafraichir 4 fois par seconde
  }

  update_startup_status(tr(KEY_MAIN_INTERFACE_LAUNCH));
  vTaskDelay(500);
}

#endif  // STARTUP_SCREEN_H