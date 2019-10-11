//This will store bandwidths in separate files
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include "statnet.h"
#include <string.h>



#define BUFF_SIZE 600
#define SUCCESS 1
#define FAILURE 0
#define NAME_LENGTH 31
#define OLD 2
#define NEW 3
#define IN 4
#define OUT 5







struct timeval beginning,ending;//for updating times
struct timeval delay;//this stores the delay put by user. By default, it is 1 second


int interfaces;//keeps track of interfaces
char **interface_names;//dynamically stores names of interfaces


unsigned long *old_bytes_in; //storing bytes in for first time
unsigned long *old_bytes_out; //storing bytes out for first time


unsigned long *new_bytes_in;//storing the bytes in for second time
unsigned long *new_bytes_out;//storing the bytes out for second time

int first_delay;//this will keep file parsing different for the first time




void save_bandwidth(int i,int type);



int main(int argc,char *argv[])
{
/*setting up interrupt signal's mask. This will prevent the signal from stopping the program until the loop
is completely executed*/
	sigset_t intmask;//interrupt's mask
	sigemptyset(&intmask);//setting up the mask
	sigaddset(&intmask, SIGINT);//adding interrupt signal to its mask


/*variables declaration*/
	double kbps;//variable that stores bandwidth calculated by kb_per_second function

	char *infilename;//variable for storing filename for file that stores incoming bandwidth
	char *outfilename;//variable for storing filename for file that stores outgoing bandwidth
	char datafilename[]="/proc/net/dev";//filename of the file that contains latest data of interfaces

	FILE *infile;//points to incoming bandwidth storage file
	FILE *outfile;//points to outgoing bandwidth storage file

	int success_old;//flag for success of obtaining initial bytes for a given iteration
	int success_new;//flag for success of obtaining final bytes for the given iteration
	int delay_arg;//counts arguments
	delay_arg = 2;//first argument of a program is its filename, so we are reading the second argument
			//which is command line's first argument


/*sets delay to either user input or to custom*/
	if (argc == delay_arg)
	{
		delay.tv_sec = (int)atof(argv[1]);
		delay.tv_usec=( ( int )atof( argv[1] )-delay.tv_sec )*( 1e6 );
	}
	else
	{
		delay.tv_sec = 1;
		delay.tv_usec = 0;
	}
first_delay = 1;
do
{
sigprocmask(SIG_BLOCK,&intmask,NULL);//this blocks the signal, so that the interruption does not mess with the program

/*file parsing to obtain initial bytes*/
	success_old = fileParser(datafilename,OLD);

/*waiting for the amount of delay specified in the beginning*/
	select(0,NULL,NULL,NULL,&delay);
	if (argc == delay_arg)//required to reset the delay, as select function modifies its values to zero
	{
		delay.tv_sec = (int)atof(argv[1]);
		delay.tv_usec=( ( int )atof( argv[1] )-delay.tv_sec )*( 1e6 );
	}
	else
	{
		delay.tv_sec = 1;
		delay.tv_usec = 0;
	}
/*file parsing to obtain final bytes*/
	success_new = fileParser(datafilename,NEW);

/*checks whether the first time's delay flag is up*/
	if (first_delay == 1)
	{
		first_delay = 0;//setting it down for subsequent delays
	}
/*error reporting if failure in obtaining initial bytes*/
	if (success_old == FAILURE)
	{
		printf("Error in extracting old dataset\n");
		continue;
	}
/*error reporting if failure in obtaining final bytes*/
	if (success_new == FAILURE)
	{
		printf("Error in extracting new dataset\n");

		free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		continue;
	}



/*file manipulation for each interface recorded*/
	for (int i = 0; i < interfaces; i++)
	{
		int success_store;//stores success in storing file

	/*setting filename for incoming bandwidth storage file*/
		infilename = setfilename(interface_names[i],IN);

	/*error reporting if failure in setting the name*/
		if(infilename == NULL)
		{
			printf("Error in creating incoming bandwidth file\n");

			free(infilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
			continue;
		}
	/*storing data on success in setting the name*/

	/*opening the file in overwrite mode*/
		infile=fopen(infilename,"w");
	/*error reporting on failure to open the file*/
		if(infile == NULL)
		{
			printf("Error in opening incoming bandwidth file\n");

			fclose(infile);
			free(infilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
			continue;
		}

	/*calcuating  incoming bandwidth*/
		kbps = kb_per_second(old_bytes_in[i],new_bytes_in[i],&beginning,&ending);
	/*storing incoming bandwidth*/
		success_store=fprintf(infile,"%.2f",kbps);
	/*error reporting on failure to store data in file*/
		if (success_store == 0)
		{
			printf("Error in storing data within incoming bandwidth file\n");

			fclose(infile);
			free(infilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
			continue;

		}

		free(infilename);
		fclose(infile);


	/*setting name for outgoing bandwidth file*/
		outfilename= setfilename(interface_names[i],OUT);
	/*error reporting on failure to set the filename*/
		if(outfilename == NULL)
		{
			printf("Error in creating outgoing bandwidth file\n");

			free(outfilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);

			continue;

		}


	/*opening file in overwrite mode*/
		outfile=fopen(outfilename,"w");
	/*error reporting on failure to open the file*/
		if(infile == NULL)
		{
			printf("Error in opening outgoing bandwidth file\n");

			fclose(outfile);
			free(outfilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
			continue;
		}
	/*calculating outgoing bandwidth*/
		kbps = kb_per_second(old_bytes_out[i],new_bytes_out[i],&beginning,&ending);
	/*storing the bandwidth in the file*/
		success_store = fprintf(outfile,"%.2f",kbps);
	/*error reporting on failure to save the data in the file*/
		if (success_store == 0)
		{
			printf("Error in storing data within outgoing bandwidth file\n");

			fclose(outfile);
			free(outfilename);
			free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
			continue;

		}


		free(outfilename);
		fclose(outfile);


	}
sigprocmask(SIG_UNBLOCK,&intmask,NULL);
}
while(1);
	free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
	return 0;
}


void save_bandwidth(int i,int type)
{
	int success_store;
	char *filename;
	double kbps;

	char inout[9];
	if (type == OUT)
	{
		strcpy(inout,"outgoing");
		kbps = kb_per_second(old_bytes_out[i],new_bytes_out[i],&beginning,&ending);

	}
	else
	{
		strcpy(inout,"incoming");
		kbps = kb_per_second(old_bytes_in[i],new_bytes_in[i],&beginning,&ending);

	}


	FILE *file;


	filename =setfilename(interface_names[i],type);
	if(filename == NULL)
	{
		printf("Error in creating %s bandwidth file\n",inout);

		free(filename);
		free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;
	}

	file = fopen(filename,"w");
	if(file == NULL)
	{
		printf("Error in opening %s bandwidth file\n",inout);


		fclose(file);
		free(filename);
		free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;
	}


	success_store = fprintf(file,"%.2f",kbps);
	if (success_store == 0)
	{
		printf("Error in storing data within %s bandwidth file\n",inout);

		fclose(file);
		free(filename);
		free_memory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;

	}

	free(filename);
	fclose(file);
}
