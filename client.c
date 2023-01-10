#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "cJSON.h"

#define IP_ADRR "34.241.4.235"
#define MAXLINE 100

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;
    char* buffer = calloc(MAXLINE, 1);
    char** cookies = calloc(1, sizeof(char*));
    char* token = NULL;
    int logged = 0;

    while(1) {
        fgets(buffer, MAXLINE, stdin);
        
        if(!strcmp(buffer, "register\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            // create JSON
            cJSON* object = cJSON_CreateObject();
            printf("username=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';

            cJSON* user = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "username", user);

            printf("password=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';

            cJSON* password = cJSON_CreateString(buffer);
            cJSON_AddItemToObjectCS(object, "password", password);

            // compute message
            char* payload = cJSON_Print(object);
            message = compute_post_request(IP_ADRR, "/api/v1/tema/auth/register", "application/json", &payload, 1, NULL,  NULL, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                puts("Account created!");
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
            close(sockfd);
            cJSON_Delete(object);

        } else if(!strcmp(buffer, "login\n")) {
            if(logged == 0) {
                sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);
                
                // create JSON
                cJSON* object = cJSON_CreateObject();
                printf("username=");
                fgets(buffer, MAXLINE, stdin);
                buffer[strlen(buffer) - 1] = '\0';

                cJSON* user = cJSON_CreateString(buffer);
                cJSON_AddItemToObject(object, "username", user);

                printf("password=");
                fgets(buffer, MAXLINE, stdin);
                buffer[strlen(buffer) - 1] = '\0';

                cJSON* password = cJSON_CreateString(buffer);
                cJSON_AddItemToObjectCS(object, "password", password);

                // compute message
                char* payload = cJSON_Print(object);
                message = compute_post_request(IP_ADRR, "/api/v1/tema/auth/login", "application/json", &payload, 1, NULL, NULL, 0);
                send_to_server(sockfd, message);
                response = receive_from_server(sockfd);
                
                // handle output or error
                char* code = strstr(response, " ");
                code += 1;
                char* rest = strstr(code, " ");
                *rest = '\0';
                rest += 1;
                if(code[0] == '2') {
                    puts("Welcome!");
                    cookies[0] = strtok(strstr(rest, "Set-Cookie:") + 12, ";");
                    logged = 1;
                } else {
                    char* error = strstr(rest, "{\"");
                    if(error) {
                        puts(strstr(rest, "{\""));
                    } else {
                        puts("Unexpected error occured. Try again later!");
                    }   
                }
            } else {
                puts("You are already logged in!");
            }
            
        }
        else if(!strcmp(buffer, "enter_library\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            //compute message
            message = compute_get_request(IP_ADRR, "/api/v1/tema/library/access", NULL, NULL, cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                token = strtok(strstr(rest, "{\"token\":") + 9, "\"");
                printf("Your JWT token is: ");
                puts(token);
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }

            close(sockfd);

        } else if(!strcmp(buffer, "get_books\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            //compute message
            message = compute_get_request(IP_ADRR, "/api/v1/tema/library/books", NULL, token, cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                if(strstr(rest, "[{")) {
                    puts(strstr(rest, "[{"));
                } else {
                    puts("No books in library");
                }
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
            close(sockfd);

        } else if(!strcmp(buffer, "get_book\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            //compute message
            printf("id=");
            char* book = calloc(50, 1);
            int id;
            scanf("%d", &id);
            fgets(buffer, MAXLINE, stdin);
            sprintf(book, "%s%d", "/api/v1/tema/library/books/", id);
            message = compute_get_request(IP_ADRR, book, NULL, token, cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                puts(strstr(rest, "[{"));
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
            close(sockfd);
            free(book);

        } else if(!strcmp(buffer, "add_book\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            // create JSON
            cJSON* object = cJSON_CreateObject();

            printf("title=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            cJSON* title = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "title", title);

            printf("author=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            cJSON* author = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "author", author);

            printf("genre=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            cJSON* genre = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "genre", genre);

            printf("publisher=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            cJSON* publisher = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "publisher", publisher);

            printf("page_count=");
            fgets(buffer, MAXLINE, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            cJSON* page_count = cJSON_CreateString(buffer);
            cJSON_AddItemToObject(object, "page_count", page_count);

            // compute message
            char* payload = cJSON_Print(object);
            message = compute_post_request(IP_ADRR, "/api/v1/tema/library/books", "application/json", &payload, 1, token, cookies, 0);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                puts("Book added!");
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
            close(sockfd);
            cJSON_Delete(object);

        } else if(!strcmp(buffer, "delete_book\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            // compute message
            printf("id=");
            char* book = calloc(50, 1);
            int id;
            scanf("%d", &id);
            fgets(buffer, MAXLINE, stdin);
            sprintf(book, "%s%d", "/api/v1/tema/library/books/", id);
            message = compute_delete_request(IP_ADRR, book, token, cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                puts("Book deleted!");
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
            close(sockfd);


        } else if(!strcmp(buffer, "logout\n")) {
            sockfd = open_connection(IP_ADRR, 8080, AF_INET, SOCK_STREAM, 0);

            // compute message
            message = compute_get_request(IP_ADRR, "/api/v1/tema/auth/logout", NULL, token, cookies, 1);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);

            // handle output or error
            char* code = strstr(response, " ");
            code += 1;
            char* rest = strstr(code, " ");
            *rest = '\0';
            rest += 1;
            if(code[0] == '2') {
                puts("See you soon!");
                logged = 0;
                token = NULL;
                cookies[0] = NULL;
            } else {
                char* error = strstr(rest, "{\"");
                if(error) {
                    puts(strstr(rest, "{\""));
                } else {
                    puts("Unexpected error occured. Try again later!");
                }
            }
        }
        else if(!strcmp(buffer, "exit\n")) {
            free(buffer);
            return 0;
        } else {
            puts("Unrecognized command! Possible commands:");
            puts("register\nlogin\nenter_library (only if you are logged in)\nget_books (only if you are in library)");
            puts("get_book\nadd_book\ndelete_book\nlogout");
        }
    }
    return 0;
}
