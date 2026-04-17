#ifndef MOTOR_H
#define MOTOR_H

void motor_init(void);

// positive -> forward, negative -> backwards
void motor_move(int speed);

// no movement
void motor_hold(void);

#endif 