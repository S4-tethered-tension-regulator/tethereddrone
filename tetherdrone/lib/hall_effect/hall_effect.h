#pragma once

void hall_init(void);
void hall_zero(void);        // call at end of calibration
void hall_tick(void);        // call repeatedly in your main loop
int  hall_get_count(void);
int  hall_get_length_cm(void);
void hall_set_direction(int dir);  // +1 = paying out, -1 = reeling in