//Header file for statnet
#ifndef NETSTAT_H
#define NETSTAT_H


extern struct timeval beginning,ending;//for updating times

extern int first_delay;//this will allow us to keep first delay's behavior different from the rest

extern int interfaces;
extern char **interface_names;//dynamically stores name of interface

extern int INTERRUPT_DETECTED;//this will become 1 on detection of interrupt from the user


extern unsigned long *old_bytes_in; //storing bytes in for first time
extern unsigned long *old_bytes_out; //storing bytes out for first time


extern unsigned long *new_bytes_in;//storing the bytes in for second time
extern unsigned long *new_bytes_out;//storing the bytes out for second time



//functions:
double kb_per_second(const unsigned long old_bytes,const unsigned long new_bytes,struct timeval *beginning,struct timeval*ending);
char *setfilename(char *interface_name,const int type);
void free_memory(unsigned long *in,unsigned long *out,FILE *fp);
void free_memory2(char **a,unsigned long *b,unsigned long *c,unsigned long *d,unsigned long *e);
int fileParser(const char *file,const int set);
void interrupt_detector(int signum);


#endif
