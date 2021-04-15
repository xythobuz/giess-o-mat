#ifndef _WIFI_STUFF_H_
#define _WIFI_STUFF_H_

#ifdef PLATFORM_ESP

void wifi_setup();
void wifi_run();

void wifi_set_message_buffer(String a, String b, String c, String d);
void wifi_schedule_websocket(void);
void wifi_send_status_broadcast(void);

void wifi_send_websocket(String s);

#endif

#endif // _WIFI_STUFF_H_
