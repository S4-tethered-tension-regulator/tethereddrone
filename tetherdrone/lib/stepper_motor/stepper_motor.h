#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <stdint.h>

void stepper_motor_init(void);
void stepper_motor_move(int8_t speed_percent);  // int8_t instead of uint8_t
void stepper_motor_stop(void);
int8_t stepper_motor_get_speed(void);

#endif