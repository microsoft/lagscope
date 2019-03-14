#include "util.h"

#define CSV_FILE_HEADER "Index, Latency(us)\n"

/* datastructure to hold latencies. */
typedef struct node
{
    unsigned long lat;
    struct node *next;
}node_t;

static node_t *head = NULL;     // start of the latency list
static node_t *tail = NULL;     // end of the latency list

/* A Hashtable where the keys are the latencies and the values are the frequencies of that latency */
static unsigned long *freq_table = NULL;

/* Private Functions */
/* Creates a new node */
static node_t *new_node(unsigned long lat)
{
    node_t *lat_node = (node_t *)malloc(sizeof(node_t));
    lat_node->lat = lat;
    lat_node->next = NULL;
    return lat_node;
}

/* Gets latency of the specified percentile in the frequency table */
static int get_percentile_latency(double percentile, unsigned long arr_size, unsigned long n_pings)
{
    unsigned int latency = 0;
    unsigned long location_of_ping = 0;
    unsigned long ping_counter = 0;
    if(percentile == 100)
    {
        latency = arr_size - 1;
    }
    else
    {
        location_of_ping = (unsigned long) (((percentile * (n_pings + 1)) / 100) - 1);
        while(ping_counter <= location_of_ping)
        {
            ping_counter += freq_table[latency];
            latency++;
        }
        latency--;
    }
    return latency;
}

/* Builds latency frequency table from linked list */
static int process_latencies(unsigned long max_latency)
{
    node_t * temp = NULL;

    freq_table = (unsigned long*) malloc(sizeof(unsigned long) * (max_latency + 1));

	if(!freq_table)
		return ERROR_MEMORY_ALLOC;

	memset(freq_table, 0, (max_latency + 1) * sizeof(unsigned long));

    if(head == NULL)
        return ERROR_GENERAL;

    /* Using the latencies stored in the linked list as keys,
       increments at latency in frequency table */
    temp = head;
    while(temp != NULL)
    {
        freq_table[temp->lat]++;
        temp = temp->next;
    }

    return NO_ERROR;
}

/* Public Functions */
int show_percentile(unsigned long max_latency, unsigned long n_pings)
{
    int err_check = 0;
    unsigned int i = 0;
    double percentile_array[] = {50, 75, 90, 99.9, 99.99, 99.999};
    size_t percentile_array_size = sizeof(percentile_array) / sizeof(percentile_array[0]);
    int percentile_idx = 0;

    if(!freq_table)
    {
        err_check = process_latencies(max_latency);
        if(err_check != NO_ERROR)
            return err_check;
    }

    /* Get percentiles at these specified points */
    printf("\nPercentile\t Latency(us)\n");
    for(i = 0; i < percentile_array_size; i++)
    {
        percentile_idx = get_percentile_latency(percentile_array[i], max_latency, n_pings);
        printf("%7g%% \t %d\n", percentile_array[i], percentile_idx);
    }

    return NO_ERROR;
}

/* Prints histogram with user specified inputs */
int show_histogram(int start, int len, int count, unsigned long max_latency)
{
    int err_check = 0;
    int i = 0;
    unsigned long freq_counter = 0;
    unsigned long final_interval = (len * count) + start;
    unsigned long lat_intervals = 0;
    int interval_start = 0;
    unsigned long after_final_interval = 0;
    unsigned long leftover = 0;

    if(!freq_table)
    {
        err_check = process_latencies(max_latency);
        if(err_check != NO_ERROR)
            return err_check;
    }

    /* Print frequencies between 0 and starting interval */
    printf("\nInterval(usec)\t Frequency\n");
    if (start > 0)
    {
        for(i = 0; i < start; i++)
            freq_counter += freq_table[i];

        printf("%7d \t %lu\n", 0, freq_counter);
    }

    /*  Prints frequencies between each interval */
    freq_counter = 0;
    for(lat_intervals = start; lat_intervals < final_interval; lat_intervals+=len)
    {
        interval_start = 0;
        if(lat_intervals > max_latency)
        {
            printf("%7lu \t %d\n", lat_intervals, 0);
            if(lat_intervals + len == final_interval)
                printf("%7lu \t %d\n", lat_intervals + len, 0);
            continue;
        }

        while(interval_start < len)
        {
            if(lat_intervals + interval_start > max_latency)
                break;
            freq_counter += freq_table[lat_intervals + interval_start];
            interval_start++;
        }
        printf("%7lu \t %lu\n", lat_intervals, freq_counter);
        freq_counter = 0;
    }

    /* Print all leftover latencies after the final interval only if final interval < max latency */
    if(final_interval < max_latency)
    {
        after_final_interval = final_interval;
        for(leftover = after_final_interval; leftover <= max_latency; leftover++)
            freq_counter += freq_table[leftover];

        printf("%7lu \t %lu\n", after_final_interval, freq_counter);
    }

    return NO_ERROR;
}

void create_latencies_csv(const char *csv_filename)
{
    node_t * temp = head;
    unsigned int latency_idx = 0;
    FILE *fp = NULL;

    fp = fopen(csv_filename, "w+");
    if(fp == NULL)
    {
        PRINT_ERR("could not create csv file");
        return;
    }

    fprintf(fp, CSV_FILE_HEADER);
    while(temp != NULL)
    {
        fprintf(fp, "%d, %lu\n", latency_idx, temp->lat);
        latency_idx++;
        temp = temp->next;
    }

    fclose(fp);
}

void push(unsigned long lat)
{
    node_t *tmp = new_node(lat);
    if(head == NULL)
        head = tail = tmp;
    else
    {
        tail->next = tmp;
        tail = tail->next;
    }
    return;
}

/* frees both latency list and frequency table */
void latencies_stats_cleanup(void)
{
    node_t *temp = NULL;
    while(head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
    head = NULL;
    if(freq_table)
        free(freq_table);
    return;
}
