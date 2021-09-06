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

#ifndef _FUNCTIONALITY_H_
#define _FUNCTIONALITY_H_

#ifdef FUNCTION_UI

void ui_setup(void);
void ui_run(void);

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

#include "Plants.h"

void control_setup(void);
void control_begin(void);
void control_run(void);

const char *control_state_name(void);
void control_act_input(int n);

Plants *get_plants(void);

#endif // FUNCTION_CONTROL

// ----------------------------------------------------------------------------

void blink_lcd(int n, int wait = 200);
void write_to_all(const char *a, const char *b,
                  const char *c, const char *d, int num_input);
void backspace(void);

bool sm_is_idle(void);

#endif // _FUNCTIONALITY_H_
