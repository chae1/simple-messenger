

/* Main code of a messenger server designed to run on a linux machine. Thread is used to handle multiple client requests concurrently, which will read or modify the global variables. Semaphore is used to allow one thread to accesse the global variables at a time. */

/* The server expects to read commands from clients. A command is a string line composed of several words */


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <semaphore.h>

#include "constant.h"
#include "rio_package.h"
#include "data_structure.h"


/* functions */

void *thread(void *vargp);
void handle(int connfd);
int parse(char buf[BUF_SIZE], char words[WORDS_IN_A_LINE][WORD_SIZE], int *length_p);
int leave_chatroom(char buf[BUF_SIZE]);

/* Global variables */

User_list *user_list;
sem_t request_mutex;


/* Wait in a while loop for the client to connect to the socket and make a thread which is for communicating 
   with the client. */

int main(int argc, char **argv)
{
	char buf[30];
	
	if (argc == 1) {
		printf("Insert ip_address:port as an argument\n");
		return 0;
	}
	else {
		strcpy(buf, argv[1]);
	}
	char ip_str[30];
	char port_str[30];
	char *token;
	char *nextptr;

	token = strtok_r(buf, ":", &nextptr);
	strcpy(ip_str, token);
		
	token = strtok_r(NULL, ":", &nextptr);
	strcpy(port_str, token);

	
	int listenfd, optval = 1;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

    uint32_t network_addr;
    inet_pton(AF_INET, ip_str, &network_addr);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = network_addr;
    server_addr.sin_port = htons(atoi(port_str)); 

    if (bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("bind failed\n");
		return 0;
    }

    if (listen(listenfd, 1024) < 0) {
        printf("listen failed\n");
        return 0;
    }

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
   
    /* When the client disconnects and the thread tries to write to the client, SIGPIPE signal will be received.
       Default handling is to exit the process, but in this server, only the corresponding thread should die. */

    signal(SIGPIPE, SIG_IGN); 

    user_list = create_user_list();
    sem_init(&request_mutex, 0, 1);
    pthread_t tid;

    while(1) {
        int *connfdp;
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (struct sockaddr*) &client_addr, &client_len);
        pthread_create(&tid, NULL, thread, connfdp);
    }

    return 0;
}


void *thread(void *vargp) {
    int connfd = *((int *)vargp);
    printf("connfd %d joined.\n", connfd);
    pthread_detach(pthread_self());
    free(vargp);
    handle(connfd);
    close(connfd);
    printf("connfd %d died.\n", connfd);
}


void handle(int connfd) {
    User *user = NULL;

    char buf[BUF_SIZE];
    char words[WORDS_IN_A_LINE][WORD_SIZE];
    int words_num;

    rio_t rio;
    rio_readinitb(&rio, connfd);

    /* When the client disconnects, rio_readlineb() will see an EOF signal. When it sees it, it returns 0,
       then the thread associated with the client will eventually end. */
    
    while (rio_readlineb(&rio, buf, BUF_SIZE) != 0) {
        printf("connfd %d : %s", connfd, buf);
        
        sem_wait(&request_mutex);
        if (user != NULL && user->state == CHAT) {
            if (leave_chatroom(buf)) { // q
                user->state = AFTER_LOGIN;
                user->current_chat_list = NULL;
                strcpy(buf, "Complex key to make user leave chat room.\n");                                
                rio_writen(connfd, buf, strlen(buf));
            }
            else {
                Chat *chat = create_chat(user, buf);
                add_to_chat_list(user->current_chat_list, chat);                               
                
                char temp[BUF_SIZE];
                strcpy(temp, "\n                  ");
                strcat(temp, user->username);
                strcat(temp, "> ");
                strcat(temp, buf);

                User_and_chat_node_tuple_node *p = user->current_chat_list->participant_and_last_read_chat_node_tuple_list->head;
                while (p != NULL) {                    
                    User_and_chat_node_tuple *user2_tuple = p->user_and_chat_node_tuple;
                    User *user2 = user2_tuple->user;
                    
                    if (user2->state == CHAT && user2->current_chat_list == user->current_chat_list) {
                        user2_tuple->chat_node = user2->current_chat_list->tail;   
                        rio_writen(user2->connfd, temp, strlen(temp));
                    }
                    p = p->next_user_and_chat_node_tuple_node;
                }
            }
        }

        else {
            if (parse(buf, words, &words_num) != 0) {             
                if (words_num > 0) {                
                    if (!strcmp(words[0], "login")) { // login [username]
                        if (words_num == 2) {                        
                            if ((user = find_user_by_username(user_list, words[1])) != NULL) { // if found in the user list                                                                                 
                                user->connfd = connfd;
                                user->state = AFTER_LOGIN;

                                strcpy(buf, "succeeded.\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                            else {
                                strcpy(buf, "failed.\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                        }                    
                    }
                    else if (!strcmp(words[0], "signup")) {
                        if (words_num == 2) {
                            if (find_user_by_username(user_list, words[1]) == NULL) {
                                User *user = create_user(words[1]);
                                add_user_to_user_list(user_list, user);

                                strcpy(buf, "succeeded.\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                            else {
                                strcpy(buf, "failed.\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                        }
                    }
                    else if (!strcmp(words[0], "user_list") && user != NULL && user->state == AFTER_LOGIN) {                   
                        User_node *p = user_list->head;       
                        strcpy(buf, "succeeded.\n");                 
                        while (p != NULL) {
                            strcat(buf, p->user->username);
                            strcat(buf, "\n");
                            p = p->next_user_node;
                        }                          
                        strcat(buf, "end.\n");
                        rio_writen(connfd, buf, strlen(buf));                    
                    }
                    else if (!strcmp(words[0], "request") && user != NULL && user->state == AFTER_LOGIN) {
                        if (words_num == 2) {
                            User *user_to;
                            if ((user_to = find_user_by_username(user_list, words[1])) != NULL) {
                                if (user == user_to) {
                                    strcpy(buf, "Can't request to yourself.\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                                else if (find_user_by_user(user->friend_list, user_to) != NULL) {
                                    strcpy(buf, "\"");
                                    strcat(buf, user_to->username);
                                    strcat(buf, "\" is already a friend.\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                                else if (find_user_by_user(user_to->request_list, user) == NULL) {
                                    add_user_to_user_list(user_to->request_list, user);

                                    strcpy(buf, "succeeded.\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                                else {
                                    strcpy(buf, "Already sent a request to \"");
                                    strcat(buf, user_to->username);
                                    strcat(buf, "\".\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                            }
                            else {
                                strcpy(buf, "No such user \"");
                                strcat(buf, words[1]);
                                strcat(buf, "\".\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                        }
                    }
                    else if (!strcmp(words[0], "request_list") && user != NULL && user->state == AFTER_LOGIN) {
                        User_node *p = user->request_list->head;  
                        strcpy(buf, "succeeded.\n");                      
                        while (p != NULL) {
                            strcat(buf, p->user->username);
                            strcat(buf, "\n");
                            p = p->next_user_node;
                        }                         
                        strcat(buf, "end.\n");
                        rio_writen(user->connfd, buf, strlen(buf));                    
                    }
                    else if (!strcmp(words[0], "accept") && user != NULL && user->state == AFTER_LOGIN) {
                        if (words_num == 2) {
                            User *user_to;                        
                            if ((user_to = find_user_by_username(user_list, words[1])) != NULL) {
                                if (find_user_by_user(user->friend_list, user_to) != NULL) {
                                    strcpy(buf, "\"");
                                    strcat(buf, user_to->username);
                                    strcat(buf, "\" is already a friend.\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                                else if (find_user_by_user(user->request_list, user_to) != NULL) {                                
                                    delete_user_from_user_list(user->request_list, user_to);
                                    add_user_to_user_list(user->friend_list, user_to);
                                    add_user_to_user_list(user_to->friend_list, user);

                                    strcpy(buf, "succeeded.\n");
                                    rio_writen(user->connfd, buf, strlen(buf));   
                                }
                                else {
                                    strcpy(buf, "No request from \"");
                                    strcat(buf, user_to->username);
                                    strcat(buf, "\".\n");
                                    rio_writen(connfd, buf, strlen(buf));
                                }
                            }
                            else {
                                strcpy(buf, "No such user \"");
                                strcat(buf, words[1]);
                                strcat(buf, "\".\n");
                                rio_writen(connfd, buf, strlen(buf));
                            }
                        }
                    }
                    else if (!strcmp(words[0], "friend_list") && user != NULL && user->state == AFTER_LOGIN) {
                        User_node *p = user->friend_list->head;  
                        strcpy(buf, "succeeded.\n");                      
                        while (p != NULL) {
                            strcat(buf, p->user->username);
                            strcat(buf, "\n");
                            p = p->next_user_node;
                        }                         
                        strcat(buf, "end.\n");
                        rio_writen(user->connfd, buf, strlen(buf)); 
                    }
                    else if (!strcmp(words[0], "chat") && user != NULL && user->state == AFTER_LOGIN) {
                        if (words_num >= 2) {
                            User_list *user_list_to_find_a_chat_list = create_user_list();
                            add_user_to_user_list(user_list_to_find_a_chat_list, user);

                            int finding_user_num = words_num - 1;
                            int n = 1;
                            while (n <= finding_user_num) {
                                User *user_found;                        
                                if ((user_found = find_user_by_username(user_list, words[n])) != NULL) {
                                    if (find_user_by_user(user->friend_list, user_found) != NULL) {
                                        add_user_to_user_list(user_list_to_find_a_chat_list, user_found);
                                        n++;
                                    }
                                    else {
                                        strcpy(buf, "No such friend \"");
                                        strcat(buf, user_found->username);
                                        strcat(buf, "\".\n");
                                        rio_writen(user->connfd, buf, strlen(buf));
                                        break;
                                    }
                                }
                                else {
                                    strcpy(buf, "No such user \"");
                                    strcat(buf, user_found->username);
                                    strcat(buf, "\".\n");
                                    rio_writen(user->connfd, buf, strlen(buf));
                                    break;
                                }
                            }

                            if (n == (finding_user_num + 1)) { // When all of the users given in the line are friends.
                            // Change user's state to CHAT and set current_chat_list.

                                strcpy(buf, "succeeded.\n");
                                user->state = CHAT;

                                Chat_list *chat_list_found = find_chat_list_by_user_list(user->chat_list_list, user_list_to_find_a_chat_list);
                             
                                if (chat_list_found != NULL) {
                                    user->current_chat_list = chat_list_found;     

                                    User_and_chat_node_tuple *tuple = find_user_and_chat_node_tuple_by_user(chat_list_found->participant_and_last_read_chat_node_tuple_list, user);
                                    Chat_node *last_unread_chat_node = tuple->chat_node;                    
                                    Chat_node *p = chat_list_found->head;
                                    int unread_message_flag;

                                    if (last_unread_chat_node == NULL)
                                        unread_message_flag = 1;
                                    else
                                        unread_message_flag = 0;

                                    while (p != NULL) {             
                                        if (unread_message_flag == 0) {
                                            if (p == last_unread_chat_node)
                                                unread_message_flag = 1;
                                            strcat(buf, "                 ");
                                            strcat(buf, p->chat->writer->username);
                                            strcat(buf, "> ");
                                            strcat(buf, p->chat->message);                     
                                            p = p->next_chat_node;                                             
                                        }
                                        else {
                                            strcat(buf, "               * ");
                                            strcat(buf, p->chat->writer->username);
                                            strcat(buf, "> ");
                                            strcat(buf, p->chat->message);          
                                            p = p->next_chat_node; 
                                        }
                                    }
                                    tuple->chat_node = user->current_chat_list->tail;
                                }
                                else { 
                                    Chat_list *new_chat_list = create_chat_list(user_list_to_find_a_chat_list);
                                    user->current_chat_list = new_chat_list;

                                    User_node *p = user_list_to_find_a_chat_list->head;
                                    while (p != NULL) { 
                                        add_to_chat_list_list(p->user->chat_list_list, new_chat_list);
                                        p = p->next_user_node;
                                    }
                                }                                      
                                strcat(buf, "end.\n");             
                                rio_writen(user->connfd, buf, strlen(buf));
                            }
                            else {} // When no such user exist or no such friend exists.

                            free_user_list(user_list_to_find_a_chat_list);     
                        }
                    }
                    else if (!strcmp(words[0], "chat_list") && user != NULL && user->state == AFTER_LOGIN) {
                        strcpy(buf, "succeeded.\n");
                        Chat_list_node *p = user->chat_list_list->head;
                        while (p != NULL) {
                            User_and_chat_node_tuple_node *p2 = p->chat_list->participant_and_last_read_chat_node_tuple_list->head;
                            while (p2 != NULL) {
                                strcat(buf, p2->user_and_chat_node_tuple->user->username);
                                strcat(buf, " ");
                                p2 = p2->next_user_and_chat_node_tuple_node;
                            }
                            strcat(buf, "\n");
                            p = p->next_chat_list_node;
                        }
                        strcat(buf, "end.\n");
                        rio_writen(user->connfd, buf, strlen(buf));
                    }
                }
            }   
        }     
        sem_post(&request_mutex);
        if (user != NULL) {
            if (user->state == AFTER_LOGIN)
                printf("connfd %d user : %s, state : AFTER_LOGIN\n\n", connfd, user->username);
            else if (user->state == CHAT)
                printf("connfd %d user : %s, state : CHAT, current chat room : %p\n\n", connfd, user->username, user->current_chat_list);
        }
        else {
            printf("connfd %d not logged in\n\n", connfd);
        }
    }

    return;
}

/* parse function */

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
            printf("%s exceeded maximum word size %d\n", token, WORD_SIZE);
            return 0;
        }
        if (length >= WORDS_IN_A_LINE) {
            printf("%s exceeded maximum number of words in a line %d\n", buf, WORDS_IN_A_LINE);
            break;
        }
    }    
    *length_p = length;
    return 1;
}

int leave_chatroom(char buf[BUF_SIZE]) {
    char str[BUF_SIZE];
    char *token;
    char *saveptr;
    strcpy(str, buf);

    token = strtok_r(str, " \n", &saveptr);
    if (token == NULL)
        return 0;
    if (!strcmp(token, "q")) {
        return 1;
    }
    return 0;
}
