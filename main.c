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

int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion, int deepLevel, char** seenURL);

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

int remove_directory(const char *path);

int analyseHTMLFile(char* destinationFileName, char* linkStorageDir, int deepLevel, int doVersion, char** seenURL,char* currentURL);

int curlAnImage(char* imageURLPart,char* destination, char* currentURL);


int main(int argc, char *argv[]) {

    //input qui arrive depuis la partie config
    char ** seenLinks = NULL;
    //char startURL[255] = "https://www.iana.org/domains/example";
    char startURL[255] = "http://www.example.com";
    int deepLevel;
    int doVersion;

    doVersion = 0; // true/flase
    deepLevel = 2;

    return curlLink(startURL,seenLinks,deepLevel,doVersion);

}

int curlLink(char * currentURL,char** seenURL,int deepLevel,int versioning){

    fprintf(stdout,"\n------------------------- Scan of new page!: %s -------------------------\n",currentURL);

    int error = 0;
    if(currentURL == NULL){ return -1;}

    CURL *curl_handle;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl_handle, CURLOPT_URL, currentURL);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0");
    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
    /* disable progress meter, set to 0L to enable and disable debug output */
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

    /* For completeness */
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 2L);
    curl_easy_setopt(curl_handle, CURLOPT_COOKIEFILE, "");
    curl_easy_setopt(curl_handle, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl_handle, CURLOPT_UNRESTRICTED_AUTH, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
    curl_easy_setopt(curl_handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 0L);

    error = writeCurlHandleToFile(curl_handle,currentURL,versioning,deepLevel,seenURL);

    //cleanup
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return error;

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
    while(result[i] != '\0'){
        if(result[i] == ' ' || result[i] == '\n'){
            result[i]='_';
        }
        i++;
    }
    //fprintf(stdout," test  %s\n",result);
    return result;
}
int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion, int deepLevel, char** seenURL){

    fprintf(stdout,"Setup in progress...\n");

    int error = 0;
    //remove "http://wwww."
    char doubleU;
    int indexChar = 0;

    if(strstr(url,"www.") != NULL) {
        while (url[indexChar] != 'w') {
            indexChar++;
        }
        indexChar += 4;
        url = &url[indexChar];
    }else{
        url = strstr(url,"://") + 3;

    }

    //fprintf(stdout,"\t-{%s}\n",url);

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
        fprintf(stdout,"\t-{%s} folder exists\n",curlStorageDir);
        closedir(dir);
    } else {
        fprintf(stdout,"\t-{%s} folder doesn't exists, it is created\n",curlStorageDir);
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

    char imageDir[255] = "\0";
    strcat(imageDir, linkStorageDir);
    strcat(imageDir, "/images");
    mkdir(imageDir, 0777);




    char destinationFileName[255] ="";
    strcat(destinationFileName,linkStorageDir);
    strcat(destinationFileName,"/");
    strcat(destinationFileName,url);
    strcat(destinationFileName,".txt");


    //file open, then write
    FILE* pagefile = fopen(destinationFileName, "w");
    if(pagefile != NULL) {
        // write page to file
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        usleep(50);
        // get it!
        curl_easy_perform(curl_handle);
        usleep(50);

        // close the header file
        fclose(pagefile);
    }else{
        fprintf(stdout,"Error opening file: %s\n",destinationFileName);
    }

    error = analyseHTMLFile(destinationFileName,linkStorageDir,deepLevel, doVersion, seenURL, url);

    return error;
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

int analyseHTMLFile(char* destinationFileName, char* linkStorageDir, int deepLevel, int doVersion, char** seenURL,char* currentURL){
    fprintf(stdout,"\nScan of HTML File in Progress: '%s'\n\n",destinationFileName);
    FILE* htmlFile = fopen(destinationFileName, "r");
    if(htmlFile == NULL)
        return 1;

    char *line = malloc(sizeof(char)*100000);
    int len = 0;
    int fileSize = 0;
    char* endLink;
    char* linkPos = malloc(sizeof(char)*100000);


    if(htmlFile != NULL) {

        fseek(htmlFile, 0L, SEEK_END);
        fileSize = ftell(htmlFile);
        rewind(htmlFile);
        if(fileSize < 10)
            return 1;


        fprintf(stdout,"Scanning for images...\n");
        //reading htmlfile line by line
        while(getline(&line, &len, htmlFile) != -1) {
            //check if there is a image to download
            while (strstr(line, "<img") != NULL && strstr(line, "src=") != NULL) {

                linkPos = strstr(line, "<img");
                *linkPos = '?';
                linkPos = strstr(linkPos, "src=");
                *linkPos = '?';
                linkPos += 5;
                endLink = strchr(linkPos, '"');
                *endLink = '\0';


                //*(linkPos+(endLink-linkPos))='\0';
                //strcpy(endLink,"\0");
                //*strchr(linkPos, '"') = '\0';


                //fprintf(stdout, "\t test %c %d\n", *strchr(linkPos, '"'),(int)(endLink-linkPos));
                fprintf(stdout, "\t-Found an image: (%s) saving in /images\n", linkPos);
                curlAnImage(linkPos,linkStorageDir,currentURL);

            }
        }

        //reset cursor for next file search :D
        rewind(htmlFile);

        if(deepLevel>0) {
            fprintf(stdout, "Scanning for links... , deepness = %d \n",deepLevel);
            //reading htmlfile line by line
            while (getline(&line, &len, htmlFile) != -1) {
                //check if there is a new link
                while (strstr(line, "href=") != NULL) {
                    linkPos = strstr(line, "href=");
                    *linkPos = '?';
                    linkPos += 6;
                    endLink = strchr(linkPos, '"');
                    *endLink = '\0';

                    if (deepLevel > 0 && strstr(linkPos, "://") != NULL) {
                        fprintf(stdout, "\t-Found href and is a link!!: %s starting scan of that page.\n", linkPos);

                        curlLink(linkPos, seenURL, deepLevel - 1, doVersion);
                    }
                }
            }
        }else{
            fprintf(stdout, "Not scanning for links, maximum deepness reached.\n");
        }

        fclose(htmlFile);
    }else{
        fprintf(stdout,"File couldn't open\n");
    }

    fprintf(stdout,"\n-------------------------------------------------------------------\n");
    //free(linkPos);
    return 0;
}


int curlAnImage(char* imageURLPart,char* destination, char* currentURL){

    CURL* image;
    CURLcode imgresult;
    FILE *fp;

    //Url for the image
    char searchImageURL[255] = "\0";

    char localDestination[255] = "\0";
    char imageLogFileDestination[255] = "\0";

    //get domain name before /
    if(strchr(currentURL,'_') != NULL)
        *strchr(currentURL,'_') = '\0';

    //change if is not a link or already is
    if(strstr(imageURLPart,"://") == NULL){
        strcat(searchImageURL,"www.");
        strcat(searchImageURL,currentURL);
        strcat(searchImageURL,imageURLPart);
    }else{
        imageURLPart = strstr(imageURLPart,"://") + 3;
        strcat(searchImageURL,imageURLPart);
    }

    strcat(imageLogFileDestination,destination);
    strcat(imageLogFileDestination,"/Images link list.txt");
    FILE *imageLog;
    imageLog = fopen(imageLogFileDestination, "a");
    if( imageLog != NULL ){
        fprintf(imageLog,"%s\n", searchImageURL);
        fclose(imageLog);
    }else{
        fprintf(stdout,"\t\t-Cannot open image log file!\n");
    }



    //replace '/' by '_' to get image name/path
    int i=0;
    while(imageURLPart[i] != '\0'){
        if(imageURLPart[i] == '/')
            imageURLPart[i] = '_';
        i++;
    }

    //destination for image file
    strcat(localDestination,destination);
    strcat(localDestination,"/images/");

    //add image file name to path/destination string
    strcat(localDestination,imageURLPart);

    image = curl_easy_init();
    if(image){
        // Open file
        fp = fopen(localDestination, "wb");
        if( fp == NULL )
            fprintf(stdout,"\t\t-Cannot open file! %s\n", localDestination);
        else
            fprintf(stdout,"\t\t-Open file! %s\n", localDestination);

        curl_easy_setopt(image, CURLOPT_URL, searchImageURL);
        //curl_easy_setopt(image, CURLOPT_URL, "www.icann.org/uploads/featured_item/image/7/thumb_icann67-attend.jpg");
        curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(image, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(image, CURLOPT_USERAGENT, "Mozilla/5.0");


        // Grab image
        imgresult = curl_easy_perform(image);


        fclose(fp);
    }
    if( imgresult ){
        fprintf(stdout,"\t\t-Cannot grab the image!\n");
    }else{
        fprintf(stdout,"\t\t-Image saved!\n");
    }

    // Clean up the resources
    curl_easy_cleanup(image);

    return 0;
}