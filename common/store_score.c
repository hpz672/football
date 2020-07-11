/*************************************************************************
	> File Name: store_score.c
	> Author: suyelu 
	> Mail: suyelu@126.com
	> Created Time: Tue 07 Jul 2020 01:04:33 AM CST
 ************************************************************************/

#include "head.h"

long timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

void store_score(char *testname, char *id, char *name, char *ip, double score) {
    char dir[128] = {0}, filename[512] = {0};
    char data_dir[] = "/home/suyelu/x.2020whu/x.测评记录/";
    sprintf(dir, "%s/%s", data_dir, testname);
    mkdir(dir, 0755);
    sprintf(filename, "%s/%s_%ld", dir, ip, timestamp());
    FILE *file = fopen(filename, "w+");
    if (file == NULL) {
        perror("fopen");
    }
    fprintf(file, "%ld|%s|%s|%s|%.2lf\n", timestamp(), id, name, ip, score);
    fclose(file);
    return;
}
