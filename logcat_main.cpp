/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "logcat.h"
#include <time.h>
#include <sys/types.h>

#include <string.h>





int last_kmsg_recordid = -1;
int last_ubtlog_recordid = -1;
size_t have_rotates = 0;
void copy_last_kmsg(){

	if(0 != access("/sys/fs/pstore/console-ramoops-0", F_OK)){
		printf("can`t access /sys/fs/pstore/console-ramoops-0 \n" );
		return;
	}
	char command[128] = {0};
	char filename[128] = {0};

	struct tm *ptime;
	time_t the_time;

	/* get current time */
	time(&the_time);
	ptime = localtime(&the_time);
	
	sprintf(filename,"%s/ubtlastkmsg.%03d.%02d%02d%02d_%02d%02d%02d",SYSTEM_PATH,
			last_kmsg_recordid+1,ptime->tm_year % 100, ptime->tm_mon + 1,
			ptime->tm_mday, ptime->tm_hour,
			ptime->tm_min, ptime->tm_sec);
	sprintf(command, "mv /sys/fs/pstore/console-ramoops-0 %s",filename);
	//printf("command:%s",command);
	system(command);
	last_kmsg_recordid++;
	if(access(filename, F_OK) >= 0){//file exist
		sprintf(command, "echo %d > %s",last_kmsg_recordid,last_kmsg_file);
		system(command);
	}
}

int read_lastno(const char *file){
	int no = -1;
	FILE *f = NULL;
	if ((f = fopen(file, "r")) == NULL)
		return no;
	fscanf(f,"%d",&no);
	//printf("%s:no=%d\n",file,no);
	if(no >= 999)
		no = -1;
	return no;
}

void tar_ubt_log(const char *path)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(path);
	if(pDir == NULL){
		printf("%s dir is null",path);
		exit(-1);
	}
    while (NULL != (ent=readdir(pDir)))
    {
        if (strstr(ent->d_name,"ubtlastkmsg"))
        {
            //printf("ubtlastkmsg:%s\n", ent->d_name);
			
        }
        else if(strstr(ent->d_name,"ubtlog") && (ent->d_type & DT_DIR))
        {
			char command[128];
			//printf("ubtlog:%s\n",ent->d_name);
			sprintf(command, "tar zcvf %s.tar.gz %s",ent->d_name,ent->d_name);
			//printf("command:%s",command);
			system(command);
			sprintf(command, "rm -rf %s",ent->d_name);
			//printf("command:%s",command);
			system(command);
        }
    }
	
}


void init_all(void)
{
	if(opendir(SYSTEM_PATH) == NULL){
		if(0 != mkdir(SYSTEM_PATH, DIR_PERMIT)){
			printf("can`t make dir %s ,return ~\n", SYSTEM_PATH);
			exit(-1);
		}
	}
    chdir(SYSTEM_PATH);
    last_kmsg_recordid = read_lastno(last_kmsg_file);
	last_ubtlog_recordid = read_lastno(last_ubtlog_file);
	copy_last_kmsg();
	tar_ubt_log(SYSTEM_PATH);
}

int main(int argc, char** argv, char** envp) {
	fprintf(stdout,"ubtlogcat runing...\n");
	mkdir("/data/ubtlogcatrund", DIR_PERMIT);
    android_logcat_context ctx = create_android_logcat();
    if (!ctx) return -1;
    signal(SIGPIPE, exit);
	init_all();
    int retval = android_logcat_run_command(ctx, -1, -1, argc, argv, envp);
    int ret = android_logcat_destroy(&ctx);
    if (!ret) ret = retval;
    return ret;
}
