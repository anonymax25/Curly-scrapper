#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/stat.h>

int curlLink(char * currentURL,char** seenURL,int deepLevel,int versioning);

int countFilesInDir(char* location);

char* getTimeForDirName();

int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion);

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

int remove_directory(const char *path);

void makeDirectory(char* url,int doVersion);


int main(int argc, char *argv[]) {

    //input qui arrive depuis la partie config
    char ** seenLinks = NULL;
    char url[255] = "https://www.iana.org/domains/example";
    //char url[255] = "http://www.example.com";
    int deepLevel;
    int doVersion;

    doVersion = 1; // true/flase
    deepLevel = 2;

    return curlLink(url,seenLinks,deepLevel,doVersion);

}

int curlLink(char * currentURL,char** seenURL,int deepLevel,int versioning){

    if(versioning == 0){ versioning = 0;}
    if(deepLevel == 0){ deepLevel = 2;}
    if(currentURL == NULL){ return -1;}

    CURL *curl_handle;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_URL, currentURL);
    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);


    writeCurlHandleToFile(curl_handle,currentURL,versioning);

    //cleanup
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return 0;

}

int countFilesInDir(char* location){

    //char command[255];
    struct dirent **namelist;
    int n = scandir(location, &namelist, NULL, alphasort);

    /*char cmd[255];

    strcat(cmd,"ls -l ");
    strcat(cmd,location);
    strcat(cmd,"| wc -l");

    int count = (int)system(cmd);
    */

    return n-2;
}

char* getTimeForDirName(){
    char* result;
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    result = asctime(timeinfo);

    int i=0;
    char tmp;
    while(tmp != '\0'){
        tmp=result[i];
        if(tmp == ' ' || tmp == '\n'){
            result[i]='_';
        }
        i++;
    }

    return result;
}
int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion){

    //remove "http://wwww."
    char doubleU;
    int indexChar = 0;
    while(url[indexChar] != 'w'){
        indexChar++;
    }

    indexChar+=4;
    url = &url[indexChar];

    indexChar = 0;
    while(url[indexChar] != '\0'){
        if(url[indexChar] == '/')
            url[indexChar]='_';
        indexChar++;
    }


    //main directory creation if absent
    char curlStorageDir[255] = "CurlStorage";
    DIR* dir = opendir(curlStorageDir);
    if (dir) {
        fprintf(stdout,"{%s} exists\n",curlStorageDir);
        closedir(dir);
    } else {
        fprintf(stdout,"{%s} doesn't exists, it is created\n",curlStorageDir);
        mkdir(curlStorageDir, 0777);
    }





    //sub directory name for current link ex: "/example.com"
    char linkStorageDir[255] = "";
    strcat(linkStorageDir,curlStorageDir);
    strcat(linkStorageDir,"/");
    strcat(linkStorageDir,url);




    //sub directory creation if absent, with version number if versioning is active
    DIR* dir2 = opendir(linkStorageDir);
    if (dir2 && doVersion == 0) {

        closedir(dir2);

        remove_directory(linkStorageDir);
        mkdir(curlStorageDir, 0777);

        mkdir(linkStorageDir, 0777);

        fprintf(stdout, "{%s} exists, file is overwritten\n", linkStorageDir);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);

        fprintf(stdout, "{%s} exists, file is overwritten\n", linkStorageDir);

    }else if (dir2 && doVersion == 1) {

        closedir(dir2);

        char versionNumberString[255];
        sprintf(versionNumberString, "%d", countFilesInDir(linkStorageDir)+1);


        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V");
        strcat(linkStorageDir,versionNumberString);

        mkdir(linkStorageDir, 0777);
        fprintf(stdout,"{%s} newly created with version\n",linkStorageDir);

    } else if (ENOENT == errno && doVersion == 0) {

        mkdir(linkStorageDir, 0777);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);
        fprintf(stdout,"{%s} newly created without version\n",linkStorageDir);

    } else if (ENOENT == errno && doVersion == 1) {

        mkdir(linkStorageDir, 0777);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);
        fprintf(stdout,"{%s} newly created with version\n",linkStorageDir);
    }






    char destinationFileName[255] ="";
    strcat(destinationFileName,linkStorageDir);
    strcat(destinationFileName,"/");
    strcat(destinationFileName,url);
    strcat(destinationFileName,".txt");


    //file open, then write
    FILE* pagefile = fopen(destinationFileName, "w");
    if(pagefile != NULL) {
        // write the page body to this file handle
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

        // get it!
        curl_easy_perform(curl_handle);

        // close the header file
        fclose(pagefile);
    }else{
        fprintf(stdout,"Error opening file: %s\n",destinationFileName);
    }

    return 0;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int remove_directory(const char *path)
{
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1;

    if (d){
        struct dirent *p;
        r = 0;
        while (!r && (p=readdir(d))){
            int r2 = -1;
            char *buf;
            size_t len;

            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")){
                continue;
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf){
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);
                if (!stat(buf, &statbuf)){
                    if (S_ISDIR(statbuf.st_mode)){
                        r2 = remove_directory(buf);
                    }else{
                        r2 = unlink(buf);
                    }
                }
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }
    if (!r){
        r = rmdir(path);
    }
    return r;
}
