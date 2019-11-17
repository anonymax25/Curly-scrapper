//
// Created by natin56y on 11/17/19.
//

#include <dirent.h>
#include <stddef.h>
#include <bits/types/time_t.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#ifndef TEST_CURL_SETUPCURL_H
#define TEST_CURL_SETUPCURL_H
#endif //TEST_CURL_SETUPCURL_H


int countFilesInDir(char* location);

char* getTimeForDirName();

int remove_directory(const char *path);

char * createCurlyStorageDestinationDirectories(char * linkStorageDir, int doVersion, char* url, char * curlStorageDir);