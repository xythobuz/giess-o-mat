/*
 * Copyright (c) 2021 Thomas Buck <thomas@xythobuz.de>
 *
 * This file is part of Giess-o-mat.
 *
 * Giess-o-mat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Giess-o-mat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giess-o-mat.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _WIFI_STUFF_H_
#define _WIFI_STUFF_H_

#ifdef PLATFORM_ESP

void wifi_setup();
void wifi_run();

void wifi_set_message_buffer(String a, String b, String c, String d);
void wifi_schedule_websocket(void);
void wifi_send_status_broadcast(void);
void wifi_broadcast_state_change(const char *s);

void wifi_send_websocket(String s);

bool wifi_write_database(int duration, const char *type, int id);

#endif

#endif // _WIFI_STUFF_H_
