#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char* JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);
    // Step 2: add the host
    compute_message(message,host);
    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf (line, "Cookie: %s", *cookies);
        compute_message(message, line);
    }

    if(JWT != NULL){
        sprintf (line, "Authorization: Bearer %s", JWT);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            char **cookies, int cookies_count, char* JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *length = calloc(LINELEN, sizeof(char));

    // Writes the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
       
    // Add host
    compute_message(message, host);

    // Add headers
    compute_message(message, content_type);
    sprintf(length, "Content-Length: %ld\r\n", strlen(*body_data));
    if(JWT != NULL){
        sprintf (line, "Authorization: Bearer %s", JWT);
        compute_message(message, line);
    }
    compute_message(message, length);
    
    // Add JSON data
    compute_message(message, *body_data);

    
    // Step 4 (optional): add cookies
    // Step 5: add new line at end of header
    // Step 6: add the actual payload data

    memset(line, 0, LINELEN);
    memset(length, 0, LINELEN);

    free(length);
    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count, char* JWT)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);

    compute_message(message, line);
    // Step 2: add the host
    compute_message(message,host);
    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    if (cookies != NULL) {
        sprintf (line, "Cookie: %s", *cookies);
        compute_message(message, line);
    }

    if(JWT != NULL){
        sprintf (line, "Authorization: Bearer %s", JWT);
        compute_message(message, line);
    }

    compute_message(message, "");
    return message;
}