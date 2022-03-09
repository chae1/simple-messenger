#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constant.h"
#include "rio_package.h"




/* Function declarations */

int stdin_readline (char *buf, size_t max);
int parse(char buf[BUF_SIZE], char words[WORDS_IN_A_LINE][WORD_SIZE], int *length_p);
void print_before_login_commands();
void print_after_login_commands();
void print_chat_commands();


int main(int argc, char **argv)
{
	char ip_port_buf[30];
	
	if (argc == 1) {
		printf("Insert ip_address:port as an argument\n");
		return 0;
	}
	else {
		strcpy(ip_port_buf, argv[1]);
	}
	char ip_str[30];
	char port_str[30];
	char *token;
	char *nextptr;

	token = strtok_r(ip_port_buf, ":", &nextptr);
	strcpy(ip_str, token);
		
	token = strtok_r(NULL, ":", &nextptr);
	strcpy(port_str, token);

	
	int client_fd;
    client_fd = socket(AF_INET, SOCK_STREAM, 0);   
    
    uint32_t network_addr;
    inet_pton(AF_INET, ip_str, &network_addr);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = network_addr;
    server_addr.sin_port = htons(atoi(port_str));

    if (connect(client_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
    {
        printf("connect failed\n");
        return 0;
    }

























    /* print information about commands */
    print_before_login_commands();
 
    /* standard input and client file descriptors for i/o multiplexing */
    fd_set read_set, ready_set;
    FD_ZERO(&read_set);
    FD_SET(STDIN_FILENO, &read_set);
    FD_SET(client_fd, &read_set);


    char buf[BUF_SIZE];
    char words[WORDS_IN_A_LINE][WORD_SIZE];
    int words_num;
    char username[WORD_SIZE];

    rio_t rio;
    rio_readinitb(&rio, client_fd);

    int state = BEFORE_LOGIN;

    /* There are three states BEFORE_LOGIN, AFTER_LOGIN, and CHAT. */
    while(1) {
        switch(state) {
            case BEFORE_LOGIN:
                printf("> ");    
                fflush(stdout);

                ready_set = read_set;
                select(client_fd + 1, &ready_set, NULL, NULL, NULL);

                if (FD_ISSET(STDIN_FILENO, &ready_set)) {
                    if (stdin_readline(buf, BUF_SIZE) == 0) { /* Expect that words are separated by spaces. ex) login user1 */
                        if (parse(buf, words, &words_num) != 0) {
                            if (words_num > 0) {
                                if (!strcmp(words[0], "login")) { // login [username] 
                                    if (words_num == 2) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0) { 
                                            strcpy(username, words[1]);
                                            state = AFTER_LOGIN;
                                            printf("\n-----------------------------------------------------------------------------\n");
                                            print_after_login_commands();
                                        }
                                        else 
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"login [username]\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "signup")) {
                                    if (words_num == 2) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0){
                                            
                                        }
                                        else
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"signup [username]\"\n");
                                    }
                                }
                                else {
                                    printf("no such command.\n");
                                }
                            }
                        }
                    }
                }
                if (FD_ISSET(client_fd, &ready_set)) {
                    if (rio_readlineb(&rio, buf, BUF_SIZE) == 0){
                        printf("\nserver died.\n");
                        return 0;
                    }
                } 
                break;               

            case AFTER_LOGIN:
                printf("%s> ", username); 
                fflush(stdout);

                ready_set = read_set;
                select(client_fd + 1, &ready_set, NULL, NULL, NULL);

                if (FD_ISSET(STDIN_FILENO, &ready_set)) {
                    if (stdin_readline(buf, BUF_SIZE) == 0) {                        
                        if (parse(buf, words, &words_num) != 0) {
                            if (words_num > 0) {
                                if (!strcmp(words[0], "user_list")) {  
                                    if (words_num == 1) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0) { 
                                            while (1) {
                                                rio_readlineb(&rio, buf, BUF_SIZE);
                                                if (strcmp(buf, "end.\n") == 0)
                                                    break;
                                                printf("%s", buf);
                                            }
                                        }
                                        else 
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"user_list\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "request")) {  
                                    if (words_num == 2) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0) {                                         
                                            printf("request succeeded.\n");
                                        }
                                        else 
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"request [username]\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "request_list")) {  
                                    if (words_num == 1) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0) { 
                                            while (1) {
                                                rio_readlineb(&rio, buf, BUF_SIZE);
                                                if (strcmp(buf, "end.\n") == 0)
                                                    break;
                                                printf("%s", buf);
                                            }
                                        }
                                        else 
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"request_list\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "accept")) {
                                    if (words_num == 2) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0)
                                            printf("accept succeeded.\n");
                                        else
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"accept [username]\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "friend_list")) {
                                    if (words_num == 1) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0){
                                            while (1) {
                                                rio_readlineb(&rio, buf, BUF_SIZE);
                                                if (strcmp(buf, "end.\n") == 0)
                                                    break;
                                                printf("%s", buf);
                                            }
                                        }
                                        else
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"friend_list\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "chat")) {
                                    if (words_num >= 2) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0) {
                                            state = CHAT;                                            
                                            printf("\n-----------------------------------------------------------------------------\n");
                                            while (1) {
                                                rio_readlineb(&rio, buf, BUF_SIZE);
                                                if (strcmp(buf, "end.\n") == 0)
                                                    break;
                                                printf("%s", buf);
                                            }      
                                            print_chat_commands();                  
                                        }
                                        else
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"chat [username]\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "chat_list")) {
                                    if (words_num == 1) {
                                        rio_writen(client_fd, buf, strlen(buf));
                                        rio_readlineb(&rio, buf, BUF_SIZE);
                                        if (strcmp(buf, "succeeded.\n") == 0){
                                            while (1) {
                                                rio_readlineb(&rio, buf, BUF_SIZE);
                                                if (strcmp(buf, "end.\n") == 0)
                                                    break;
                                                printf("%s", buf);
                                            }
                                        }
                                        else
                                            printf("%s", buf);
                                    }
                                    else {
                                        printf("try again with the format \"chat_list\"\n");
                                    }
                                }
                                else if (!strcmp(words[0], "help")) {
                                    if (words_num == 1) {
                                        print_after_login_commands();
                                    }
                                    else {
                                        printf("try again with the format \"commands\"\n");
                                    }
                                }
                                else {
                                    printf("no such command.\n");
                                }
                            }
                        }
                    }
                }   
                if (FD_ISSET(client_fd, &ready_set)) {
                    if (rio_readlineb(&rio, buf, BUF_SIZE) == 0){
                        printf("server died.\n");
                        return 0;
                    }
                }
                break;

            case CHAT:
                printf("%s> ", username); 
                fflush(stdout);

                ready_set = read_set;
                select(client_fd + 1, &ready_set, NULL, NULL, NULL);

                if (FD_ISSET(STDIN_FILENO, &ready_set)) {
                    if (stdin_readline(buf, BUF_SIZE) == 0) { 
                        rio_writen(client_fd, buf, strlen(buf));                        
                    }
                }
                if (FD_ISSET(client_fd, &ready_set)) {
                    if (rio_readlineb(&rio, buf, BUF_SIZE) == 0){
                        printf("server died.\n");
                        return 0;
                    }
                    else {
                        if (!strcmp(buf, "Complex key to make user leave chat room.\n")) {
                            state = AFTER_LOGIN;
                            printf("\n-----------------------------------------------------------------------------\n\n");
                            print_after_login_commands();
                        }
                        else {
                            printf("%s", buf);
                            if (rio_readlineb(&rio, buf, BUF_SIZE) == 0){
                                printf("server died.\n");
                                return 0;
                            }
                            printf("%s", buf);
                        }
                    }
                }
                break;

            default:
                break;
        }
    }
    return 0;
}

int stdin_readline (char *buf, size_t max) {
    if (!fgets(buf, max, stdin))
    {
        printf("fgets failed\n");
        return -1;
    }
    return 0;
}

int parse(char buf[BUF_SIZE], char words[WORDS_IN_A_LINE][WORD_SIZE], int *length_p) {
    char str[BUF_SIZE];
    char *token;
    char *saveptr;
    int length = 0;

    strcpy(str, buf);
    while (1) {
        if (length == 0)
            token = strtok_r(str, " \n", &saveptr);
        else
            token = strtok_r(NULL, " \n", &saveptr);

        if (token == NULL)
            break;
        if (strlen(token) < WORD_SIZE)
            strcpy(words[length++], token);
        else {
            printf("exceeded maximum word size %d\n", WORD_SIZE);
            return 0;
        }
        if (length >= WORDS_IN_A_LINE)
            break;
    }    
    *length_p = length;
    return 1;
}

void print_before_login_commands(){
    printf("\n");
    printf("Commands : \n\n");
    printf("login [username] \n");
    printf("signup [username] \n\n");
}
void print_after_login_commands(){
    printf("\n");
    printf("Commands : \n\n");
    printf("user_list \n");
    printf("request [username] \n");
    printf("request_list \n");
    printf("accept [username] \n");
    printf("friend_list \n");
    printf("chat [username 1] ... [username n] \n");
    printf("chat_list \n");
    printf("help \n\n");
}
void print_chat_commands(){
    printf("\n");
    printf("Commands : \n\n");
    printf("[message]\n");
    printf("q \n\n");
}
