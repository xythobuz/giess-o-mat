#ifndef _FUNCTIONALITY_H_
#define _FUNCTIONALITY_H_

#ifdef FUNCTION_UI

void ui_setup(void);
void ui_run(void);

#endif // FUNCTION_UI

// ----------------------------------------------------------------------------

#ifdef FUNCTION_CONTROL

void control_setup(void);
void control_begin(void);
void control_run(void);

#endif // FUNCTION_CONTROL

// ----------------------------------------------------------------------------

void blink_lcd(int n, int wait = 200);
void write_to_all(const char *a, const char *b,
                  const char *c, const char *d, int num_input);
void backspace(void);

#endif // _FUNCTIONALITY_H_
