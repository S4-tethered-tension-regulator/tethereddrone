#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <stdint.h>

void stepper_motor_init(void);
void stepper_motor_move(uint8_t speed_percent);
void stepper_motor_stop(void);

#endif