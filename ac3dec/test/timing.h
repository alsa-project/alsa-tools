/* 
 *  timing.h
 *
 *	Aaron Holtzman - May 1999
 *
 */

//uint_64 get_time(void);
uint_64 timing_init(void);

void timing_test_2(void (*func)(void*,void*),void *arg_1,void *arg_2,char name[]);
void timing_test_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3,char name[]);
double timing_once_3(void (*func)(void*,void*,void*),void *arg_1,void *arg_2,void *arg_3);

