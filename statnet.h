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

extern double kbps_in;		//declared as such in order to append this data to the log file
extern double kbps_out;		//declared as such in order to append this data to the log file



//functions:

/*calculates bandwidth in kbps*/
double Bandwidth(const unsigned long old_bytes,const unsigned long new_bytes,struct timeval *beginning,struct timeval*ending,const int type);

/*Sets filename for storing bandwidth*/
char *SetFileName(char *interface_name,const int type);

/*frees memory inside fileParser function*/
void FreeMemory(unsigned long *in,unsigned long *out,FILE *fp);

/*frees memory in the main program*/
void FreeMemory2(char **a,unsigned long *b,unsigned long *c,unsigned long *d,unsigned long *e);

/*parses file to fetch bytes in/out*/
int FileParser(const char *file,const int set);

/*saves bandwidths in files at /tmp/ */
void SaveBandwidth(const int i,const int type);

/*saves bandwidths permanently in a log file*/
void AppendLog(const int i);

/*handles interrupt given by the user. Ensures that memory is freeed before program exits*/
void InterruptHandler(int signum);


#endif
