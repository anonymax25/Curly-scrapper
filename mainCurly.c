//
// Created by natin56y on 11/17/19.
//


#include "includes/mainCurly.h"


int curlLink(char * currentURL,char** seenURL,int deepLevel,int versioning, char** mimeTypes, int mimeTypesCount){

    fprintf(stdout,"\n------------------------- Scan of new page!: %s -------------------------\n",currentURL);

    int error = 0;
    if(currentURL == NULL){ return -1;}

    CURL *curl_handle;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    /* set URL to get here */
    curl_easy_setopt(curl_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl_handle, CURLOPT_URL, currentURL);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.94 Safari/537.36");
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

    error = writeCurlHandleToFile(curl_handle,currentURL,versioning,deepLevel,seenURL,mimeTypes, mimeTypesCount);

    //cleanup
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return error;

}






int writeCurlHandleToFile(CURL *curl_handle,char* url,int doVersion, int deepLevel, char** seenURL, char ** mimeTypes, int mimeTypesCount){

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
    char curlStorageDir[255] = "Curly Storage";
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


    createCurlyStorageDestinationDirectories(linkStorageDir,doVersion,url,curlStorageDir);

    //sub directory creation if absent, with version number if versioning is active


    char imageDir[255] = "\0";
    strcat(imageDir, linkStorageDir);
    strcat(imageDir, "/images");
    mkdir(imageDir, 0777);




    char destinationFileName[255] ="";
    strcat(destinationFileName,linkStorageDir);
    strcat(destinationFileName,"/");
    strcat(destinationFileName,url);
    strcat(destinationFileName,".html");


    //file open, then write
    FILE* pagefile = fopen(destinationFileName, "w");
    if(pagefile != NULL) {
        // write page to file
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        // get it!
        curl_easy_perform(curl_handle);
        // close the header file
        fclose(pagefile);
    }else{
        fprintf(stdout,"Error opening file: %s\n",destinationFileName);
    }

    error = analyseHTMLFile(destinationFileName,linkStorageDir,deepLevel, doVersion, seenURL, url, mimeTypes, mimeTypesCount);

    return error;
}

static size_t write_data(char *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

int analyseHTMLFile(char* destinationFileName, char* linkStorageDir, int deepLevel, int doVersion, char** seenURL,char* currentURL,char ** mimeTypes, int mimeTypesCount){
    fprintf(stdout,"\nScan of HTML File in Progress: '%s'\n\n",destinationFileName);
    FILE* htmlFile = fopen(destinationFileName, "r");
    int fileSize = 0;

    if(htmlFile != NULL) {
        //stop if file too small
        fseek(htmlFile, 0L, SEEK_END);
        fileSize = ftell(htmlFile);
        rewind(htmlFile);
        if(fileSize < 10)
            return 2;

        //scan for images
        searchImageInHtml(htmlFile,linkStorageDir,currentURL,mimeTypes,mimeTypesCount);

        //reset cursor for next file search :D
        rewind(htmlFile);

        //scan for links or not
        if(deepLevel>0) {
            searchLinkInHtml(htmlFile,linkStorageDir,currentURL,deepLevel,seenURL,doVersion,mimeTypes,mimeTypesCount);
        }else{
            fprintf(stdout, "Not scanning for links, maximum deepness reached.\n");
        }

        fclose(htmlFile);
    }else{
        fprintf(stdout,"File couldn't open\n");
    }

    fprintf(stdout,"\n---------------------------------------------------------- End page Scan of: %s\n\n",currentURL);

    return 0;
}


int curlAnImage(char* imageURLPart,char* destination, char* currentURL, char** mimeTypes, int mimeTypesCount){

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
        curl_easy_setopt(image, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(image, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(image, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(image, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.94 Safari/537.36");

        // verifier qu'on a l'image
        imgresult = curl_easy_perform(image);

        int counter = 0;
        char *ct = NULL;
        if(!imgresult){

            imgresult = curl_easy_getinfo(image, CURLINFO_CONTENT_TYPE, &ct);
            if(!imgresult && ct) {

               for(int i=0;i<mimeTypesCount;i++){
                    if(strcmp(mimeTypes[i],ct) == 0){
                        counter++;
                    }


                }

            }
        }
        //if hasn't seen his content type, delete image file

        if(counter == 0){
            printf("\t\t-Content-Type not accepted: %s\n", ct);
            remove(localDestination);
        }else{
            printf("\t\t-Content-Type accepted: %s\n", ct);
        }


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

//goes through html to find image URL to pass it on
void searchImageInHtml(FILE* htmlFile, char* linkStorageDir, char* currentURL,char** mimeTypes, int mimeTypesCount){

    char *line = (char*)malloc(sizeof(char)*5000);
    size_t len = 0;
    char* endLink;
    char* linkPos = (char*)malloc(sizeof(char)*5000);
    char* tmp = linkPos;

    fprintf(stdout,"Scanning for images...\n");
    //reading htmlfile line by line
    while(getline(&line, &len, htmlFile) != -1) {
        //check if there is a image to download
        while (strstr(line, "<img") != NULL && strstr(line, "src=") != NULL) {

            //finds position of url and pus a \0 at the end
            linkPos = strstr(line, "<img");
            *linkPos = '?';
            linkPos = strstr(linkPos, "src=");
            *linkPos = '?';
            linkPos += 5;
            endLink = strchr(linkPos, '"');
            *endLink = '\0';

            //fprintf(stdout, "\t test %c %d\n", *strchr(linkPos, '"'),(int)(endLink-linkPos));
            fprintf(stdout, "\t-Found an image: (%s) saving in /images\n", linkPos);
            curlAnImage(linkPos,linkStorageDir,currentURL,mimeTypes,mimeTypesCount);

        }
    }
    free(line);
    free(tmp);

}

//goes through html to find page URL to curl it if max deepness is not yet achieved
void searchLinkInHtml(FILE* htmlFile, char* linkStorageDir, char* currentURL, int deepLevel, char ** seenURL, int doVersion, char ** mimeTypes, int mimeTypesCount){

    char *line = (char*)malloc(sizeof(char)*5000);
    size_t len = 0;
    char* endLink;
    char* linkPos = (char*)malloc(sizeof(char)*5000);
    char* tmp = linkPos;

    fprintf(stdout, "Scanning for links...\n");
    //reading htmlfile line by line
    while (getline(&line, &len, htmlFile) != -1) {
        //check if there is a new link
        while (strstr(line, "href=") != NULL) {

            //finds position of url and pus a \0 at the end
            linkPos = strstr(line, "href=");
            *linkPos = '?';
            linkPos += 6;
            endLink = strchr(linkPos, '"');
            *endLink = '\0';

            //check if its a good links
            if (strstr(linkPos, "://") != NULL && strstr(linkPos, ".docx") == NULL && strstr(linkPos, ".pdf") == NULL) {
                fprintf(stdout, "\t-Found href and is a link!!: %s starting scan of that page.\n", linkPos);
                curlLink(linkPos, seenURL, deepLevel - 1, doVersion,mimeTypes,mimeTypesCount);
            }
        }
    }
    free(line);
    free(tmp);

}