#ifndef LOAD_CELL_H
#define LOAD_CELL_H

// setup
void load_cell_init(void);
// reads just one value from the sensor
long load_cell_read(void);
// reads it a couple times and takes the average so its more stable
long load_cell_read_average(int times);

#endif 