#ifndef CENTER_COTROL_PRESENTER_H
#define CENTER_COTROL_PRESENTER_H

void flight_handle(lv_event_t *e);
void setting_handle(lv_event_t *e);
void _4g_handle(lv_event_t *e);
void sound_handle(lv_event_t *e);
void wifi_handle(lv_event_t *e);
void bluebooth_handle(lv_event_t *e);

bool setItemStatus(int index, bool status);
bool getItemStatus(int index);

void brightness_slider_handle(lv_event_t *e);

#endif
