#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#include "statnet.h"

#define BUFF_SIZE 300
#define FAILURE 0
#define NAME_LENGTH 31
#define OLD 2
#define NEW 3
#define IN 4
#define OUT 5


void InterruptHandler(int signum)
{
        FreeMemory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
	printf("Statnet Stopped");
	exit(1);
}

void AppendLog(const int i)
{
	/*Declaration of variables*/
	FILE *log_pointer;

	char log_name[18];
	char cur_time[9];

	time_t raw_time;
	struct tm *obtained_time;


	/*Filename creation*/
	time(&raw_time);
	obtained_time = localtime(&raw_time);
	strftime(log_name,sizeof(log_name),"/etc/%Y%m%d.log",obtained_time);



	/*Opening file*/
	if ( (   log_pointer=fopen(   log_name,"a"   )  ) != NULL )
	{
		time(&raw_time);
		obtained_time = localtime(&raw_time);
		strftime(cur_time,sizeof(cur_time),"%X",obtained_time);

		fprintf(log_pointer,"%s,%s,%.2f,%.2f\n",cur_time,interface_names[i],kbps_in,kbps_out);

		fclose(log_pointer);
		log_pointer = NULL;
	}

}





void SaveBandwidth(const int i,const int type)
{
	int success_store;
	char *filename;
	double *kbps;

	char inout[9];
	if (type == OUT)
	{
		strcpy(inout,"outgoing");
		kbps_out = Bandwidth(old_bytes_out[i],new_bytes_out[i],&beginning,&ending,OUT);
		kbps=&kbps_out;
	}
	else
	{
		strcpy(inout,"incoming");
		kbps_in = Bandwidth(old_bytes_in[i],new_bytes_in[i],&beginning,&ending,IN);
		kbps=&kbps_in;
	}


	FILE *file;


	filename =SetFileName(interface_names[i],type);
	if(filename == NULL)
	{
		printf("Error in creating %s bandwidth file\n",inout);

		free(filename);
		FreeMemory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;
	}

	file = fopen(filename,"w");
	if(file == NULL)
	{
		printf("Error in opening %s bandwidth file\n",inout);


		fclose(file);
		free(filename);
		FreeMemory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;
	}


	success_store = fprintf(file,"%.2f",*kbps);
	if (success_store == 0)
	{
		printf("Error in storing data within %s bandwidth file\n",inout);

		fclose(file);
		free(filename);
		FreeMemory2(interface_names,old_bytes_in,old_bytes_out,new_bytes_in,new_bytes_out);
		return;

	}

	free(filename);
	fclose(file);
}


double Bandwidth(const unsigned long old_bytes,const unsigned long new_bytes,struct timeval *beginning,struct timeval*ending,const int type)
{
	unsigned long bytes_difference = new_bytes - old_bytes;
	int seconds = ending->tv_sec - beginning->tv_sec;
	double delay_time = (double)(seconds) + (ending->tv_usec - beginning->tv_usec)/((double)1e6);
//	printf("\t%.2f\n",time);

	double rate = (double)bytes_difference/delay_time;
	rate /= 1024; //conversion to kilobytes/second
//	printf("%.2f",rate);
	return rate; //returning the success

}





char *SetFileName(char *interface_name,const int type)
{

	char directory[] ="/tmp/";
        char appended[5];
        if(type == IN)
                strcpy(appended,"_in");
        else
                strcpy(appended,"_out");

        char *filename;//points to the created filename

        int  filesize;//stores calculated filesize
        filesize =strlen(directory)+strlen(interface_name)+strlen(appended)+1;

        filename=calloc(filesize,sizeof(char));//<directory><interface name><appended>\0
	filename=strdup(directory);//adding directory to filename
        filename=strcat(filename,interface_name);//appending interface's name
        filename=strcat(filename,appended);//appending the in/out type

        return filename;

}





void FreeMemory(unsigned long *in,unsigned long *out,FILE *fp)
{
	if (fp != NULL)
	{
		free(fp);
	}
	free(in);
	free(out);
}



void FreeMemory2(char **a,unsigned long *b,unsigned long *c,unsigned long *d,unsigned long *e)
{
	for (int i = 0; i < interfaces; i++)
		free(a[i]);
	free(a);
	free(b);
	free(c);
	free(d);
	free(e);
}






int FileParser(const char *file,const int set)
{
	FILE *filepointer;

	unsigned long *bytes_in;
	unsigned long *bytes_out;

	char *stats;//used for traversing given line for statistics of an interface
	char *name; //used for pointing to interface name

	char buffer[BUFF_SIZE];


	filepointer = fopen(file,"r");
	if (set == OLD)
	{
//		if (first_delay == 1)
			gettimeofday(&beginning,NULL);
//		else
//		{
//			beginning.tv_sec=ending.tv_sec;
//			beginning.tv_usec=ending.tv_usec;
//		}
	}
	else if (set == NEW)
		gettimeofday(&ending,NULL);
	else
		{
			printf("Invalid set\n");
			return FAILURE;
		}


	//to reallcc these variables later on
	interface_names = calloc(1,sizeof(char *));//to make use of realloc in the loop
	bytes_in = calloc(1,sizeof(unsigned long ));
	bytes_out = calloc(1,sizeof(unsigned long ));



	if (filepointer != NULL)
	{
		int lines = 0; //for keeping check of lines traversed
		interfaces = 0;//for keeping track of interfaces

		int internal_buff = setvbuf(filepointer,NULL,_IONBF,0);//disabling internal buffer that messes up the program
		if (internal_buff != 0)//if disabling failed
		{
			printf("Error: Internal buffer not deactivated, which prevents filestream to buffer properly.\n");

			FreeMemory(bytes_in,bytes_out,NULL);
			return FAILURE;
		}


		while (fgets(buffer,BUFF_SIZE,filepointer) != NULL)
		{
			printf("\r");
			if (lines < 2)//to ignore first two lines of the file
			{
				lines++;
				continue;
			}
//----------------------------obtaining name----------------------------------//
			name = strtok(buffer,":");
//			printf("%s",name);
			if (name != NULL)
			{
				interface_names = realloc(interface_names,sizeof(char *)*interfaces+1);//to add interface
				if (interface_names == NULL)
				{
					printf("Error in adding interface name: not allocating space for name\n");

					FreeMemory(bytes_in,bytes_out,filepointer);
					return FAILURE;

				}

				interface_names[interfaces]=calloc(NAME_LENGTH,sizeof(char));
				if (interface_names[interfaces] == NULL)
				{
					printf("Error in adding interface name: not allocating space for name characters\n");

					FreeMemory(bytes_in,bytes_out,filepointer);
					return FAILURE;

				}

				while(strchr(name,' ') != NULL)//to make sure no space is added before interface
				{
					name++;
				}


				if (strcmp(name,"lo")==0)//no need of recording lo
				{
					continue;
				}

				interface_names[interfaces]=strdup(name);
				if (interface_names[interfaces] == NULL)
				{
					printf("Error in adding interface name: not saved\n");

					FreeMemory(bytes_in,bytes_out,filepointer);
					return FAILURE;
				}

				interface_names[interfaces]=realloc(interface_names[interfaces],strlen(interface_names[interfaces])+1);//memory optimization
				if (interface_names[interfaces] == NULL)
				{
					printf("Error in adding interface name: memory not optimized\n");

					FreeMemory(bytes_in,bytes_out,filepointer);
					return FAILURE;

				}
			}
			else
			{
				printf("Error in obtaining interface name\n");

				FreeMemory(bytes_in,bytes_out,filepointer);
				return FAILURE;
			}
//---------------------------------------name obtained------------------------------------//



//---------------------------------------obtaining data----------------------------------//
			stats = strtok(NULL," ");//to go to the bytes

			bytes_in = realloc(bytes_in,sizeof(unsigned long )*(interfaces+1));
			if (bytes_in == NULL)
			{
				printf("Error in adding bytes in: not allocating space for bytes\n");

				FreeMemory(bytes_in,bytes_out,filepointer);
				return FAILURE;

			}


			bytes_out = realloc(bytes_out,sizeof(unsigned long )*(interfaces+1));
			if (bytes_out == NULL)
			{
				printf("Error in adding bytes out: not allocating space for bytes\n");

				FreeMemory(bytes_in,bytes_out,filepointer);
				return FAILURE;

			}



			int success_bytes_in = sscanf(stats,"%lu",(bytes_in+interfaces));
			if (success_bytes_in == 0)
			{
				printf("Error in obtaining in_bytes");

				FreeMemory(bytes_in,bytes_out,filepointer);
				return FAILURE;

			}


			for (int i = 1; i <= 8; i++)
			{
				stats=strtok(NULL," ");
			}

			int success_bytes_out = sscanf(stats,"%lu",(bytes_out+interfaces));
			if (success_bytes_out == 0)
			{
				printf("Error in obtaining out_bytes");

				FreeMemory(bytes_in,bytes_out,filepointer);
				return FAILURE;

			}

				interfaces++;
		}



		fclose(filepointer);



		if(set == OLD)
		{
			old_bytes_in = realloc(old_bytes_in,(sizeof(unsigned long))*(interfaces));
			old_bytes_out = realloc(old_bytes_out,(sizeof(unsigned long))*(interfaces));
			for(int i = 0;i < interfaces;i++)
			{
		//static int nuo = 0;
		//nuo++;

				old_bytes_in[i] = bytes_in[i];
		//printf("Old: %s\t%lu\t",interface_names[i],old_bytes_in[i]);
				old_bytes_out[i]= bytes_out[i];
		//printf("%lu\t%i\n",old_bytes_out[i],nuo);

			}
		}
		if(set == NEW)
		{

			new_bytes_in = realloc(new_bytes_in,(sizeof(unsigned long))*(interfaces));
			new_bytes_out = realloc(new_bytes_out,(sizeof(unsigned long))*(interfaces));
			for (int i = 0; i < interfaces;i++)
			{
		//static int nu = 0;
	//	nu++;
				new_bytes_in[i] = bytes_in[i];
	//	printf("New: %s\t%lu\t",interface_names[i],new_bytes_in[i]);

				new_bytes_out[i] = bytes_out[i];
	//	printf("%lu\t%i\n",new_bytes_out[i],nu);

			}

		}
//-------------------------------------------data obtained--------------------//


		FreeMemory(bytes_in,bytes_out,NULL);

		return interfaces;
	}
	else
	{
		printf("File failed to open\n");
		return FAILURE;
	}


}
