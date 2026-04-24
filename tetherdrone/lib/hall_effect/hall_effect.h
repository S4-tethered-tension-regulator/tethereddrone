#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

void hall_sensor_init(void);

// Returns the current speed of the winch
float hall_sensor_get_speed(void);

#endif // HALL_SENSOR_H