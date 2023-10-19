#include "../include/driver.h"
FILE *spine_outfiles[2];
void open_switch_outfiles(char *base_filename)
{
    char csv_suffix[4] = ".csv";
    for (int i = 0; i < 2; i++)
    {
        char filename[520] = "out/";
        char spine_suffix[6] = ".spine";
        char spine_id[5];
        sprintf(spine_id, "%d", i);
        strncat(filename, base_filename, 500);
        strncat(filename, spine_suffix, 6);
        strncat(filename, spine_id, 5);
        strncat(filename, csv_suffix, 4);
        spine_outfiles[i] = fopen(filename, "w");
        if (spine_outfiles[i] == NULL)
        {
            printf("%s\n", filename);
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
        fprintf(spine_outfiles[i], "flow_id, src, dst, port, arrival_time, creation_time\n");
    }
}
int main()
{
    char fn[500] = "abctest";

    open_switch_outfiles(fn);
}
