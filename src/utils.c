#include "utils.h"
#include <dirent.h>


/* ------------------------------------------
 * mega2byte
 *
 * Converts MBytes into Bytes
 * ------------------------------------------ */
long mega2byte(long n)
{
    return (n * 1000 *1000);
} // end mega2byte

/* ------------------------------------------
 * kilo2byte
 *
 * Converts KBytes into Bytes
 * ------------------------------------------ */
long kilo2byte(long n)
{
    return (n * 1000);
} // end kilo2byte

/**
 * byte2mega
 * Converts Bytes in MBytes
 */
double byte2mega(long n)
{
	double result;
	result = (double) n / (1000 * 1000);
	return result;
}

/**
 * byte2kilo
 * Converts Bytes in KBytes
 */
double byte2kilo(long n)
{
	double result;
	result = (double) n / (1000);
	return result;
}


/* ------------------------------------------
 * findDataUnit
 *
 * Extracts the data unit from the given string.
 * If no unit is specified, returns 'Z'.
 * ------------------------------------------ */
char find_data_unit(const char *inarg)
{
    // units are B (Bytes), K (KBytes) and M (MBytes)
    const char unitArray[] =
        {'B', 'k', 'K', 'M'
        };
    char * unit = malloc(sizeof(char));

    if ((unit = strpbrk(inarg, unitArray)) == NULL)
    {
        unit = "Z";
    }

    if (unit[0] == 'K')
    	return 'k';

    return unit[0];
} // end find_data_unit

/* ------------------------------------------
 * findRateUnit
 *
 * Extracts the rate unit from the given string.
 * If no unit is specified, returns kbit/sec.
 * ------------------------------------------ */
char find_rate_unit(const char *inarg)
{
    // units are b (bundles/sec), K (Kbit/sec), and M (Mbit/sec)
    const char unitArray[] =
        {'k', 'K', 'M', 'b'
        };
    char * ptr;
    char unit;

    if ((ptr = strpbrk(inarg, unitArray)) == NULL)
    {
    	printf("\nWARNING: (-r option) invalid rate unit, assuming 'k' (kb/s)\n\n");
    	return 'k';
    }

    unit = ptr[0];

    if (unit == 'K')
    	return 'k';

    return unit;
} // end find_data_unit


/* ------------------------------------------
 * add_time
 * ------------------------------------------ */
struct timeval add_time(struct timeval * time_1, struct timeval * time_2)
{
	struct timeval result;
    result.tv_sec = time_1->tv_sec + time_2->tv_sec;
    result.tv_usec = time_1->tv_usec + time_2->tv_usec;

    if (result.tv_usec >= 1000000)
    {
        result.tv_sec++;
        result.tv_usec -= 1000000;
    }
    return result;

} // end add_time


/* ------------------------------------------
 * sub_time
 * Subtract time sub from time min and put the result in result
 * Result is set to NULL result is a negative value
 * ------------------------------------------ */
void sub_time(struct timeval min, struct timeval sub, struct timeval * result)
{
	if (result == NULL)
		return;
   if (min.tv_sec < sub.tv_sec)
	   result = NULL;
   else if (min.tv_sec == sub.tv_sec && min.tv_usec < sub.tv_usec)
	   result = NULL;
   else
   {
	   if (min.tv_usec < sub.tv_usec)
	   {
		   min.tv_usec += 1000 * 1000;
		   min.tv_sec -= 1;
	   }
	   result->tv_sec = min.tv_sec - sub.tv_sec;
	   result->tv_usec = min.tv_usec - sub.tv_usec;
   }

} // end add_time


/* --------------------------------------------------
 * csv_time_report
 * -------------------------------------------------- */
void csv_time_report(int b_sent, int payload, struct timeval start, struct timeval end, FILE* csv_log)
{
    const char* time_report_hdr = "BUNDLE_SENT,PAYLOAD (byte),TIME (s),DATA_SENT (Mbyte),GOODPUT (Mbit/s)";


    double g_put, data;

    data = (b_sent * payload)/1000000.0;

    fprintf(csv_log, "\n\n%s\n", time_report_hdr);

    g_put = (data * 8 * 1000) / ((double)(end.tv_sec - start.tv_sec) * 1000.0 +
                          (double)(end.tv_usec - start.tv_usec) / 1000.0);
    
    double time_s =  ((double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_usec - start.tv_usec) / 1000.0) / 1000;
    
    fprintf(csv_log, "%d,%d,%.1f,%E,%.3f\n", b_sent, payload, time_s, data, g_put);

} // end csv_time_report




/* --------------------------------------------------
 * csv_data_report
 * -------------------------------------------------- */
void csv_data_report(int b_id, int payload, struct timeval start, struct timeval end, FILE* csv_log)
{
    const char* data_report_hdr = "BUNDLE_ID,PAYLOAD (byte),TIME (s),GOODPUT (Mbit/s)";
    // const char* time_hdr = "BUNDLES_SENT,PAYLOAD,TIME,GOODPUT";
    double g_put;

    fprintf(csv_log, "\n\n%s\n", data_report_hdr);

    g_put = (payload * 8) / ((double)(end.tv_sec - start.tv_sec) * 1000.0 +
                             (double)(end.tv_usec - start.tv_usec) / 1000.0) / 1000.0;
                             
    double time_s =  ((double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_usec - start.tv_usec) / 1000.0) / 1000;

    fprintf(csv_log, "%d,%d,%.1f,%.3f\n", b_id, payload, time_s, g_put);

} // end csv_data_report



/* -------------------------------------------------------------------
 * pattern
 *
 * Initialize the buffer with a pattern of (index mod 10).
 * ------------------------------------------------------------------- */
void pattern(char *outBuf, int inBytes)
{
    assert (outBuf != NULL);
    while (inBytes-- > 0)
    {
        outBuf[inBytes] = (inBytes % 10) + '0';
    }
} // end pattern



/* -------------------------------------------------------------------
 * Set timestamp to the given seconds
 * ------------------------------------------------------------------- */
struct timeval set( double sec )
{
    struct timeval mTime;

    mTime.tv_sec = (long) sec;
    mTime.tv_usec = (long) ((sec - mTime.tv_sec) * 1000000);

    return mTime;
} // end set


/* -------------------------------------------------------------------
 * Add seconds to my timestamp.
 * ------------------------------------------------------------------- */
struct timeval add( double sec )
{
    struct timeval mTime;

    mTime.tv_sec = (long) sec;
    mTime.tv_usec = (long) ((sec - ((long) sec )) * 1000000);

    // watch for overflow
    if ( mTime.tv_usec >= 1000000 )
    {
        mTime.tv_usec -= 1000000;
        mTime.tv_sec++;
    }
    assert( mTime.tv_usec >= 0 && mTime.tv_usec < 1000000 );

    return mTime;
} // end add


/* --------------------------------------------------
 * show_report
 * -------------------------------------------------- */
void show_report (u_int buf_len, char* eid, struct timeval start, struct timeval end, long data, FILE* output)
{
    double g_put;

    double time_s = ((double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_usec - start.tv_usec) / 1000.0) / 1000.0;

    double data_MB = data / 1000000.0;
    
    if (output == NULL)
        printf("got %d byte report from [%s]: time=%.1f s - %E Mbytes sent", buf_len, eid, time_s, data_MB);
    else
        fprintf(output, "\n total time=%.1f s - %E Mbytes sent", time_s, data_MB);
  
    // report goodput (bits transmitted / time)
    g_put = (data_MB * 8) / time_s;
    if (output == NULL)
        printf(" (goodput = %.3f Mbit/s)\n", g_put);
    else
        fprintf(output, " (goodput = %.3f Mbit/s)\n", g_put);

    // report start - end time
    if (output == NULL)
        printf(" started at %u sec - ended at %u sec\n", (u_int)start.tv_sec, (u_int)end.tv_sec);
    else
        fprintf(output, " started at %u sec - ended at %u sec\n", (u_int)start.tv_sec, (u_int)end.tv_sec);

} // end show_report


char* get_filename(char* s)
{
    int i = 0, k;
    char* temp;
    char c = 'a';
    k = strlen(s) - 1;
    temp = malloc(strlen(s));
    strcpy(temp, s);
    while ((c != '/') && (k >= 0))
    {
        c = temp[k];
        k--;
    }

    if (c == '/')
        k += 2;

    else
        return temp;

    while (k != (int)strlen(temp))
    {
        temp[i] = temp[k];
        i++;
        k++;
    }
    temp[i] = '\0';

    return temp;
} // end get_filename

/* --------------------------------------------------
 * file_exists
 * returns TRUE if file exists, FALSE otherwise
 * -------------------------------------------------- */
boolean_t file_exists(const char * filename)
{
    FILE * file;
    if ((file = fopen(filename, "r")) != NULL)
    {
        fclose(file);
        return TRUE;
    }
    return FALSE;
} // end file_exists

char * correct_dirname(char * dir)
{
	if (dir[0] == '~')
	{
		char * result, buffer[100];
		char * home = getenv("HOME");
		strcpy(buffer, home);
		strcat(buffer, dir + 1);
		result = strdup(buffer);
		return result;
	}
	else return dir;
}

int find_proc(char * cmd)
{
	DIR * proc;
	struct dirent * item;
	char cmdline_file[100];
	char buf[256], cmdline[256];
	char cmd_exe[256], cmdline_exe[256];
	char cmd_args[256], cmdline_args[256];
	char * cmdline_args_ptr;
	int item_len, i, fd, pid, length;
	int result = 0;
	boolean_t found_not_num, found_null;

	// prepare cmd
	memset(cmd_args, 0, sizeof(cmd_args));
	memset(cmd_exe, 0, sizeof(cmd_exe));
	strcpy(cmd_args, strchr(cmd, ' '));
	strcpy(cmd_exe, get_exe_name(strtok(cmd, " ")));
	strncat(cmd_exe, cmd_args, strlen(cmd_args));

	proc = opendir("/proc/");
	while ((item = readdir(proc)) != NULL)
	{
		found_not_num = FALSE;
		item_len = strlen(item->d_name);
		for(i = 0; i < item_len; i++)
		{
			if (item->d_name[i] < '0' || item->d_name[i] > '9')
			{
				found_not_num = TRUE;
				break;
			}
		}
		if (found_not_num)
			continue;

		memset(cmdline_file, 0, sizeof(cmdline_file));
		memset(cmdline, 0, sizeof(cmdline));
		memset(buf, 0, sizeof(buf));

		sprintf(cmdline_file, "/proc/%s/cmdline", item->d_name);
		fd = open(cmdline_file, O_RDONLY);
		length = read(fd, buf, sizeof(buf));
		close(fd);

		found_null = FALSE;
		if(buf[0] == '\0')
			continue;

		for(i = 0; i < length; i++)
		{
			if(buf[i] == '\0')
			{
				if(found_null)
				{
					//reached end of cmdline
					cmdline[i -1] = '\0';
					break;
				}
				else
				{
					found_null = TRUE;
					cmdline[i] = ' ';
				}
			}
			else
			{
				found_null = FALSE;
				cmdline[i] = buf[i];
			}
		}

		memset(cmdline_args, 0, sizeof(cmdline_args));
		memset(cmdline_exe, 0, sizeof(cmdline_exe));
		cmdline_args_ptr = strchr(cmdline, ' ');
		strcpy(cmdline_args, cmdline_args_ptr != NULL ? cmdline_args_ptr : "");
		strcpy(cmdline_exe, get_exe_name(strtok(cmdline, " ")));
		strncat(cmdline_exe, cmdline_args, sizeof(cmdline_exe) - strlen(cmdline_exe) - 2);

		if (strncmp(cmdline_exe, cmd_exe, strlen(cmd_exe)) == 0)
		{
			pid = atoi(item->d_name);
			if (pid == getpid())
				continue;
			else
			{
				result = pid;
				break;
			}
		}

	}
	closedir(proc);
	return result;
}

char * get_exe_name(char * full_name)
{
	char * buf1 = strdup(full_name);
	char * buf2;
	char * result;
	char * token = strtok(full_name, "/");
	do
	{
		buf2 = strdup(token);
	} while ((token = strtok(NULL, "/")) != NULL);
	result = strdup(buf2);
	free (buf1);
	free (buf2);
	return result;
}

void pthread_sleep(double sec)
{
	pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;
	struct timespec abs_timespec;
	struct timeval current, time_to_wait;
	struct timeval abs_time;

	gettimeofday(&current,NULL);
	time_to_wait = set(sec);
	abs_time = add_time(&current, &time_to_wait);

	abs_timespec.tv_sec = abs_time.tv_sec;
	abs_timespec.tv_nsec = abs_time.tv_usec*1000;

	pthread_mutex_lock(&fakeMutex);
	pthread_cond_timedwait(&fakeCond, &fakeMutex, &abs_timespec);
	pthread_mutex_unlock(&fakeMutex);
}
