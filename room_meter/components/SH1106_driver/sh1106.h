#ifndef __SH1160_H__
#define __SH1160_H__

void sh1106_init(void);
void task_sh1106_display_text(const void *arg_text);
void sh1106_print_line(int line, const char *text);
void task_sh1106_display_clear(void *ignore);

#endif
