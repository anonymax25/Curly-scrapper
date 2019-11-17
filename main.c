#include "includes/mainCurly.h"

int main(int argc, char **argv) {

    fprintf(stdout,"---------------- Welcome to the curly program! ----------------\n\n");

    //input qui arrive depuis la partie config
    char ** seenLinks = NULL;

    char startURL[255] = "http://www.example.com";

    int mimeTpyesCount = 3;
    const char* mimeTypes[3] = { "text/html", "image/jpeg","image/svg+xml"};

    int deepLevel;
    int doVersion;

    int result = 0;

    doVersion = 0; // true/flases
    deepLevel = 2;

    //ICI TOUT CE LANCE
    result = curlLink(startURL,seenLinks,deepLevel,doVersion,mimeTypes,mimeTpyesCount);

    if(result == 0){
        fprintf(stdout,"End of Curly action, went without errors\n");
    }else{
        fprintf(stdout,"Error! End of Curly action, one or more errors occured!\n");
    }

    return result;
}







