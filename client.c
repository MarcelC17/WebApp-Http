#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"
#include "helpers.h"
#include "requests.h"

/*
*Sends get books request. Returns NULL if fails
*/
char* get_books(char* cookie, char* JWT, char* host, char* booksURL, int sockfd){
    if (cookie == NULL) {
        printf("Client not logged in");
        return NULL;
    }
    if (JWT == NULL) {
        printf("Client did not request library access");
        return NULL;
    }

    char* message = compute_get_request(host, booksURL, NULL, &cookie, 1, JWT);
    send_to_server(sockfd, message);
    return receive_from_server(sockfd);
}


/*
*   Sends register request.
*/
void register_client(char* username, char* password, JSON_Object* credentials, JSON_Value* root_credentials, char* message,
                     char* host, char* registerURL, char* contentype, int sockfd, char* response){
            printf("username=");
            fgets(username, LINELEN, stdin);
            username[strcspn(username, "\n")] = 0;
            
            printf("password=");
            fgets(password, LINELEN, stdin);
            password[strcspn(password, "\n")] = 0;

            /* add client credentials to json*/
            json_object_set_string(credentials, "username", username);
            json_object_set_string(credentials, "password", password);
            char *json_string = json_serialize_to_string(root_credentials);

            /* makes register request and sends to server*/
            message = compute_post_request(host, registerURL, contentype, (char **)&json_string, NULL, 0,NULL);
            send_to_server(sockfd, message);
            
            /* treat server response */
            response = receive_from_server(sockfd);
            strtok(response," ");
            char *exit_code = strtok(NULL, " ");
            if (strcmp(exit_code, "201") == 0) printf("Succes\n");
            else printf("Username already taken\n");

            free(json_string);
}

/*
*   Sends get login request.
*/
char* login_client(char* username, char* password, JSON_Object* credentials, JSON_Value* root_credentials,
                char* message, char* host, char* loginURL, char* contentype, int sockfd, char* response, char* cookie){
    /* read new credentials */
    printf("username=");
    fgets(username, LINELEN, stdin);
    username[strcspn(username, "\n")] = 0;
    printf("\n");
    
    printf("password=");
    fgets(password, LINELEN, stdin);
    password[strcspn(password, "\n")] = 0;
    printf("\n");

    /* checks username validity */
    if (strpbrk(username, " ")) {
        printf("invalid username");
        return NULL;
    }

    /* add client credentials to json*/
    json_object_set_string(credentials, "username", username);
    json_object_set_string(credentials, "password", password);
    char *json_string = json_serialize_to_string(root_credentials);
    /* makes register request and sends to server*/
    message = compute_post_request(host, loginURL, contentype, (char **)&json_string, NULL, 0, NULL);
    send_to_server(sockfd, message);

    /* checks response  and extracts cookie */
    response = receive_from_server(sockfd);
    strtok(response," ");
    char *exit_code = strtok(NULL, " ");
    if (strcmp(exit_code, "200") == 0) {
        
        while (exit_code)
        {
            exit_code = strtok(NULL," ");
            if (strncmp(exit_code, "connect", 6) == 0){
                cookie = malloc(LINELEN);
                cookie = exit_code;
                cookie = strtok(cookie,";");
                break;
            }
        }
        printf("Succes\n");
    }
    else printf("Credentials dont match\n");
    free(json_string);
    return cookie;
}

/*
*   Sends enter_library request.
*/
char* enter_library(char* cookie, char* message, char* host, char* libraryURL, int sockfd, char* response, char* JWT){
    if(!cookie) {
        printf("Log in to use this feature\n");
        return NULL;
    }
    
    message = compute_get_request(host, libraryURL, NULL, &cookie, 1, NULL);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    char *json_string = basic_extract_json_response(response);
    strtok(json_string,"\"");
    /* extracts JWT token */
    for (int i = 0; i < 2; i++)
    {
        strtok(NULL, "\"");
    }
    JWT = strtok(NULL, "\"");
    printf("Succes\n");
    return JWT;
}

/*
*   Sends get_book request.
*/
void get_book(char* cookie, char* URL, char* bookURL, char* JWT, char* id, char* message, char* host, int sockfd, char* response){
    if (cookie == NULL) {
        printf("Client not logged in\n");
        return;
    }

    if (JWT == NULL) {
        printf("Client did not request library access\n");
        return;
    }

    /* prompts for id and adds it to URL*/
    printf("id=");
    fgets(id, LINELEN, stdin);
    id[strcspn(id, "\n")] = 0;
    strcat(URL, bookURL);
    strcat(URL, id);
    /* creates request and sends it */
    message = compute_get_request(host, URL, NULL, &cookie, 1, JWT);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    char* exit = basic_extract_json_response(response);
    
    if (exit) {
        printf("%s", exit);
    } else {
       printf("no such book");
    }
    memset(URL, 0, LINELEN);
}

/*
*   Sends add_book request.
*/
void add_book(char* cookie, char* JWT, char* title, char* author, char* genre, char* publisher, char* page_count, 
             JSON_Object* bookinfo, JSON_Value* root_book, char* message, char* host, char* booksURL,
             char* contentype, int sockfd, char* response){
    if (cookie == NULL) {
        printf("Client not logged in");
        return;
    }
    if (JWT == NULL) {
        printf("Client did not request library access");
        return;
    }
    /* get book info */
    printf("title=");
    fgets(title, LINELEN, stdin);
    title[strcspn(title, "\n")] = 0;
 
    printf("author=");
    fgets(author, LINELEN, stdin);

    printf("genre=");
    fgets(genre, LINELEN, stdin);
    genre[strcspn(genre, "\n")] = 0;

    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    publisher[strcspn(publisher, "\n")] = 0;

    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    page_count[strcspn(page_count, "\n")] = 0;

    /*check if info is valid*/
    if (strlen(title) == 0 || strlen(author) == 0 ||strlen(genre) == 0 
        || strlen(publisher) == 0 || strlen(page_count) == 0)
        {
            printf("wrong arguments");
            return;
        }
    
    int valid_count = 0;
    for (int i = 0; i < strlen(page_count); i++)
    {
        if (page_count[i] < '0' || page_count[i] > '9'){
            printf("wrong arguments");
            valid_count = 1;
            break;
        }
    }
    
    if (valid_count){
        return;
    }

     /*add info to json and sent the request*/
    json_object_set_string(bookinfo, "title", title);
    json_object_set_string(bookinfo, "author", author);
    json_object_set_string(bookinfo, "publisher", publisher);
    json_object_set_string(bookinfo, "genre", genre);
    json_object_set_string(bookinfo, "page_count", page_count);
    char *json_string = json_serialize_to_string(root_book);
    message = compute_post_request(host, booksURL, contentype, (char **)&json_string, NULL, 0, JWT);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    free(json_string);
    printf("Succes\n");
}

/*
*   Sends delete_book request.
*/
void delete_book(char* cookie, char* JWT, char* id, char* URL, char* bookURL, char* message, char* host, int sockfd, char* response){
    if (cookie == NULL) {
        printf("Client not logged in\n");
        return;
    }

    if (JWT == NULL) {
        printf("Client did not request library access\n");
        return;
    }
    /* prompts for id and adds it to URL*/
    printf("id=");
    fgets(id, LINELEN, stdin);
    id[strcspn(id, "\n")] = 0;
    strcat(URL, bookURL);
    strcat(URL, id);

    message = compute_delete_request(host, URL, &cookie, 1, JWT);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    strtok(response, " ");
    char *exit_code = strtok(NULL, " ");
    if (strcmp(exit_code, "200") != 0) printf("no such book\n");
    else printf("Succes\n");
    memset(URL, 0, LINELEN);
}

/*
*   Sends logout request.
*/
void logout(char* cookie, char* message, char* host , char* logoutURL, int sockfd){
    if (!cookie) {
        printf("Client not logged in\n");
        return;
    }

    message = compute_get_request(host, logoutURL, NULL, &cookie, 1, NULL);
    send_to_server(sockfd, message);
    cookie = NULL;
    printf("Log out succcesful\n");
}

int main(int argc, char *argv[])
{
    /* server data */
    char ipserver[LINELEN] = "34.254.242.81";
    u_int16_t port = 8080;
    char *message = NULL;
    char *response = NULL;
    int sockfd;
    char* cookie = NULL;
    char* JWT = NULL;

    /* verificarea erorilor*/
    int vef = 1;

    
    /* command buffer */
    char command[COMMANDSIZE];

    /* register data */
    char username[LINELEN];
    char password[LINELEN];
    char id[LINELEN];
    char title[LINELEN];
    char author[LINELEN];
    char publisher[LINELEN];
    char genre[LINELEN];
    char page_count[LINELEN];

    /* http lines */
    char host[LINELEN] = "Host: 34.254.242.81";
    char contentype[LINELEN] = "Content-Type: application/json";

    /* url */
    char registerURL[LINELEN] = "/api/v1/tema/auth/register";
    char loginURL[LINELEN] = "/api/v1/tema/auth/login";
    char libraryURL[LINELEN] = "/api/v1/tema/library/access";
    char booksURL[LINELEN] = "/api/v1/tema/library/books";
    char bookURL[LINELEN] =  "/api/v1/tema/library/books/";
    char logoutURL[LINELEN] = "/api/v1/tema/auth/logout";
    char *URL = calloc(sizeof(char),LINELEN);

    
    /* initialize json */
    JSON_Value *root_credentials = json_value_init_object();
    JSON_Object *credentials = json_value_get_object(root_credentials);
    JSON_Value *root_book = json_value_init_object();
    JSON_Object *bookinfo = json_value_get_object(root_book);
    

    /* command loop */
    while (1)
    {
        /* read command */
        fgets(command, COMMANDSIZE, stdin);
        command[strcspn(command, "\n")] = 0;
        /* connects to server */
        sockfd = open_connection(ipserver, port, AF_INET, SOCK_STREAM, 0);
        

        if (strcmp(command, "exit") == 0)
        break;

        if (strcmp(command, "register") == 0){
            register_client(username, password, credentials, root_credentials, 
                            message, host, registerURL, contentype, sockfd, response);
        }

        if (strcmp(command, "login") == 0){
            cookie = login_client(username, password, credentials, root_credentials, message, 
                        host, loginURL, contentype, sockfd, response, cookie);
        }

        if (strcmp(command, "enter_library") == 0){
            JWT = enter_library(cookie, message, host, libraryURL, sockfd, response, JWT);
        }

        if (strcmp(command, "get_books") == 0){
            response = get_books(cookie,JWT, host, booksURL, sockfd);
            if (response) printf("%s",basic_extract_json_response(response));
        }

        if (strcmp(command, "get_book") == 0){
            get_book(cookie, URL, bookURL, JWT, id, message, host, sockfd, response);
        }

        if(strcmp(command, "add_book") == 0){
            add_book(cookie, JWT, title, author, genre, publisher, page_count, bookinfo, root_book, 
            message, host, booksURL, contentype, sockfd, response);
        } 

        if (strcmp(command, "delete_book") == 0){
            delete_book(cookie, JWT, id, URL, bookURL, message, host, sockfd, response);
        }

        if (strcmp(command, "logout") == 0){
            logout(cookie, message, host, logoutURL, sockfd);
        }
    }

    return 0;
}