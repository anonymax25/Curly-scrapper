//
// Created by natin56y on 11/17/19.
//

#include "includes/setupDirCurly.h"

int countFilesInDir(char* location){

    //char command[255];
    struct dirent **namelist;
    int n = scandir(location, &namelist, NULL, alphasort);
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
    while(result[i] != '\0'){
        if(result[i] == ' ' || result[i] == '\n'){
            result[i]='_';
        }
        i++;
    }

    return result;
}

int remove_directory(const char *path){

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

char * createCurlyStorageDestinationDirectories(char * linkStorageDir, int doVersion, char* url, char * curlStorageDir){
    DIR* dir2 = opendir(linkStorageDir);
    if (dir2 && doVersion == 0) {

        closedir(dir2);

        remove_directory(linkStorageDir);
        mkdir(curlStorageDir, 0777);

        mkdir(linkStorageDir, 0777);

        fprintf(stdout, "\t-{%s} exists, folder is overwritten\n", linkStorageDir);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);

        fprintf(stdout, "\t-{%s} exists, folder is overwritten\n", linkStorageDir);

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
        fprintf(stdout,"\t-{%s} newly created folder with versioning\n",linkStorageDir);

    } else if (ENOENT == errno && doVersion == 0) {

        mkdir(linkStorageDir, 0777);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);
        fprintf(stdout,"\t-{%s} newly created folder without versioning\n",linkStorageDir);

    } else if (ENOENT == errno && doVersion == 1) {

        mkdir(linkStorageDir, 0777);

        strcat(linkStorageDir,"/");
        strcat(linkStorageDir,url);
        strcat(linkStorageDir,"_");
        strcat(linkStorageDir,getTimeForDirName());
        strcat(linkStorageDir,"-V1");

        mkdir(linkStorageDir, 0777);
        fprintf(stdout,"\t-{%s} newly created with version\n",linkStorageDir);
    }
}

void miscFolderSetup(char *currentURL, int deepLevel, int versioning){
    //create misc Curly dir if not there
    char curlMiscDir[255] = "Curly Misc Files";
    DIR* dir = opendir(curlMiscDir);
    if (dir) {
        closedir(dir);
    } else {
        mkdir(curlMiscDir, 0777);
    }

    //log this action if versionning is active in Curly ann
    FILE *imageLog;
    imageLog = fopen(strcat(curlMiscDir,"/Action history.txt"), "a");
    if( imageLog != NULL ){
        fprintf(imageLog,"Curly action: url = %s, max_deepness = %d, do_versionning = %d \n",currentURL,deepLevel,versioning);
        fclose(imageLog);
    }else{
        fprintf(stdout,"-Cannot open action log file!\n");
    }
}