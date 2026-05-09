#ifndef LOAD_CELL_H
#define LOAD_CELL_H

void load_cell_init(void);
long load_cell_read(void);
long load_cell_read_average(int times);
void load_cell_tare(int times);

// Global variable to hold the zero offset if you want to use the tare function
extern long load_cell_offset;

#endif