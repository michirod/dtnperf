#ifndef UTILS_H_
#define UTILS_H_

#include "includes.h"




char* get_filename(char* s);
void pattern(char *outBuf, int inBytes);


long mega2byte(long n);
long kilo2byte(long n);
double byte2mega(long n);
double byte2kilo(long n);
char find_data_unit(const char *inarg);
char find_rate_unit(const char *inarg);

void csv_time_report(int b_sent, int payload, struct timeval start, struct timeval end, FILE* csv_log);
void csv_data_report(int b_id, int payload, struct timeval start, struct timeval end, FILE* csv_log);
void show_report (u_int buf_len, char* eid, struct timeval start, struct timeval end, long data, FILE* output);

void add_time(struct timeval *tot_time, struct timeval part_time);
void sub_time(struct timeval min, struct timeval sub, struct timeval * result);
struct timeval set(double sec);
struct timeval add(double sec);

boolean_t file_exists(const char * filename);

char * correct_dirname(char * dir);
int find_proc(char * cmd);
char * get_exe_name(char * full_name);

void pthread_sleep(double sec);
#endif /*UTILS_H_*/
