//
// Created by natin56y on 11/17/19.
//

#include <stddef.h>
#include <stdio.h>
#include <curl/curl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "setupDirCurly.h"

#ifndef TEST_CURL_MAINCURLYPROCESS_H
#define TEST_CURL_MAINCURLYPROCESS_H
#endif //TEST_CURL_MAINCURLYPROCESS_H



int curlLink(char * currentURL,char** seenURL,int deepLevel,int versioning, char ** mimeTypes, int mimeTypesCount);

int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion, int deepLevel, char** seenURL,char ** mimeTypes, int mimeTypesCount);

static size_t write_data(char *ptr, size_t size, size_t nmemb, void *stream);

int analyseHTMLFile(char* destinationFileName, char* linkStorageDir, int deepLevel, int doVersion, char** seenURL,char* currentURL,char ** mimeTypes, int mimeTypesCount);

int curlAnImage(char* imageURLPart,char* destination, char* currentURL,char** mimeTypes, int mimeTypesCount);

void searchImageInHtml(FILE* htmlFile, char* linkStorageDir, char* currentURL,char** mimeTypes, int mimeTypesCount);

void searchLinkInHtml(FILE* htmlFile, char* linkStorageDir, char* currentURL, int deepLevel, char ** seenURL, int doVersion,char ** mimeTypes, int mimeTypesCount);