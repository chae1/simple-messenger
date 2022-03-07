
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "data_structure.h"

User *create_user(char *username) {
    User *user = (User *) malloc(sizeof(User));    
    strcpy(user->username, username);
    user->connfd = -1;
    user->state = BEFORE_LOGIN;
    user->friend_list = create_user_list();
    user->request_list = create_user_list();
    user->chat_list_list = create_chat_list_list();
    user->current_chat_list = NULL;
    return user;
}

Chat *create_chat(User *writer, char *message){
    Chat *chat = (Chat *) malloc(sizeof(Chat));
    chat->writer = writer;
    strcpy(chat->message, message);
    return chat;
}

User_and_chat_node_tuple *create_user_and_chat_node_tuple(User *user, Chat_node *chat_node){
    User_and_chat_node_tuple *user_and_chat_node_tuple = (User_and_chat_node_tuple *) malloc(sizeof(User_and_chat_node_tuple));
    user_and_chat_node_tuple->user = user;
    user_and_chat_node_tuple->chat_node = chat_node;
    return user_and_chat_node_tuple;
}




User_node *create_user_node(User *user){
    User_node *user_node = (User_node *) malloc(sizeof(User_node));
    user_node->user = user;
    user_node->next_user_node = NULL;
    return user_node;
}

Chat_node *create_chat_node(Chat *chat){
    Chat_node *chat_node = (Chat_node *) malloc(sizeof(Chat_node));
    chat_node->chat = chat;
    chat_node->next_chat_node = NULL;
    return chat_node;
}

Chat_list_node *create_chat_list_node(Chat_list *chat_list){
    Chat_list_node *chat_list_node = (Chat_list_node *) malloc(sizeof(Chat_list_node));
    chat_list_node->chat_list = chat_list;
    chat_list_node->next_chat_list_node = NULL;
    return chat_list_node;
}

User_and_chat_node_tuple_node *create_user_and_chat_node_tuple_node(User_and_chat_node_tuple *user_and_chat_node_tuple){
    User_and_chat_node_tuple_node *user_and_chat_node_tuple_node = (User_and_chat_node_tuple_node *) malloc(sizeof(User_and_chat_node_tuple_node));
    user_and_chat_node_tuple_node->user_and_chat_node_tuple = user_and_chat_node_tuple;
    user_and_chat_node_tuple_node->next_user_and_chat_node_tuple_node = NULL;
    return user_and_chat_node_tuple_node;
}



User_list *create_user_list(){
    User_list *user_list = (User_list *) malloc(sizeof(User_list));
    user_list->n = 0;
    user_list->head = NULL;
    user_list->tail = NULL;
    return user_list;
}
Chat_list *create_chat_list(User_list *user_list){
    Chat_list *chat_list = (Chat_list *) malloc(sizeof(Chat_list));
    chat_list->n = 0;
    chat_list->head = NULL;
    chat_list->tail = NULL;
    chat_list->participant_and_last_read_chat_node_tuple_list = create_user_and_chat_node_tuple_list();

    User_node *p = user_list->head;
    while (p != NULL) { 
        User_and_chat_node_tuple *tuple = create_user_and_chat_node_tuple(p->user, NULL); 
        add_to_user_and_chat_node_tuple_list(chat_list->participant_and_last_read_chat_node_tuple_list, tuple);
        p = p->next_user_node;
    }
    return chat_list;
}
Chat_list_list *create_chat_list_list(){
    Chat_list_list *chat_list_list = (Chat_list_list *) malloc(sizeof(Chat_list_list));
    chat_list_list->n = 0;
    chat_list_list->head = NULL;
    chat_list_list->tail = NULL;
    return chat_list_list;
}
User_and_chat_node_tuple_list *create_user_and_chat_node_tuple_list(){
    User_and_chat_node_tuple_list *user_and_chat_node_tuple_list = (User_and_chat_node_tuple_list *) malloc(sizeof(User_and_chat_node_tuple_list));

    user_and_chat_node_tuple_list->n = 0;
    user_and_chat_node_tuple_list->head = NULL;
    user_and_chat_node_tuple_list->tail = NULL;
    return user_and_chat_node_tuple_list;
}

void free_user_list(User_list *user_list){
    User_node *p = user_list->head;
    User_node *p_temp;
    while (p != NULL) {
        p_temp = p->next_user_node;
        free(p);
        p = p_temp;
    }
    free(user_list);
}

void add_user_to_user_list(User_list *user_list, User *user){
    User_node *user_node = create_user_node(user);
    if (user_list->n == 0) {
        user_list->head = user_node;
        user_list->tail = user_node;
        (user_list->n)++;
    }
    else {
        user_list->tail->next_user_node = user_node;
        user_list->tail = user_node;
        (user_list->n)++;
    }
}

void delete_user_from_user_list(User_list *user_list, User *user) {
    if (user_list->n == 1) { /* when n = 1 and delete it */
        if (user_list->head->user == user) {
            User_node *d = user_list->head;
            user_list->head = NULL;
            user_list->tail = NULL;
            free(d);
            (user_list->n)--;
            return;
        }
    }
    for (User_node *ptr = user_list->head; ptr != NULL; ptr = ptr->next_user_node) { /* when n >= 2 */
        if (ptr == user_list->head && ptr->user == user) { /* delete head */
            User_node *d = user_list->head;
            user_list->head = user_list->head->next_user_node;
            free(d);
            (user_list->n)--;            
            return;
        }       
        if (ptr->next_user_node != NULL && ptr->next_user_node->user == user) { /* general case */
            User_node *d = ptr->next_user_node;
            ptr->next_user_node = d->next_user_node;
            if (d == user_list->tail)  /* delete tail */
               user_list->tail = ptr;            
            free(d);
            (user_list->n)--;
            return;
        }
    }
}

void add_to_user_and_chat_node_tuple_list(User_and_chat_node_tuple_list *tuple_list, User_and_chat_node_tuple *tuple){ 
    User_and_chat_node_tuple_node *tuple_node = create_user_and_chat_node_tuple_node(tuple);
    if (tuple_list->n == 0) {
        tuple_list->head = tuple_node;
        tuple_list->tail = tuple_node;
        (tuple_list->n)++;
    }
    else {
        tuple_list->tail->next_user_and_chat_node_tuple_node = tuple_node;
        tuple_list->tail = tuple_node;
        (tuple_list->n)++;
    }
}

void add_to_chat_list(Chat_list *chat_list, Chat *chat) {
    Chat_node *chat_node = create_chat_node(chat);
    if (chat_list->n == 0) {
        chat_list->head = chat_node;
        chat_list->tail = chat_node;
        (chat_list->n)++;
    }
    else {
        chat_list->tail->next_chat_node = chat_node;
        chat_list->tail = chat_node;
        (chat_list->n)++;
    }
}

void add_to_chat_list_list(Chat_list_list *chat_list_list, Chat_list *chat_list) {
    Chat_list_node *chat_list_node = create_chat_list_node(chat_list);
    if (chat_list_list->n == 0) {
        chat_list_list->head = chat_list_node;
        chat_list_list->tail = chat_list_node;
        (chat_list_list->n)++;
    }
    else {
        chat_list_list->tail->next_chat_list_node = chat_list_node;
        chat_list_list->tail = chat_list_node;
        (chat_list_list->n)++;
    }
}

User *find_user_by_user(User_list *user_list, User *user){
    struct User_node *p = user_list->head;
    while(p != NULL) {
        if (p->user == user) {
            return p->user;
        }
        p = p->next_user_node;
    }
    return NULL;
}

User *find_user_by_username(User_list *user_list, char *username){
    struct User_node *p = user_list->head;
    while(p != NULL) {
        if (strcmp(username, p->user->username) == 0) {
            return p->user;
        }
        p = p->next_user_node;
    }
    return NULL;
}

User_and_chat_node_tuple *find_user_and_chat_node_tuple_by_user(User_and_chat_node_tuple_list *user_and_chat_node_tuple_list, User *user){
    struct User_and_chat_node_tuple_node *p = user_and_chat_node_tuple_list->head;
    while(p != NULL) {
        if (p->user_and_chat_node_tuple->user == user) {
            return p->user_and_chat_node_tuple;
        }
        p = p->next_user_and_chat_node_tuple_node;
    }
    return NULL;
}

Chat_list *find_chat_list_by_user_list(Chat_list_list *chat_list_list, User_list *user_list){
    struct Chat_list_node *cln = chat_list_list->head;
    
    while(cln != NULL) {
        struct User_node *un = user_list->head;
        while(un != NULL) {
            if ((cln->chat_list->participant_and_last_read_chat_node_tuple_list->n == user_list->n) && (find_user_and_chat_node_tuple_by_user(cln->chat_list->participant_and_last_read_chat_node_tuple_list, un->user) != NULL)) {
                if (un->next_user_node != NULL)
                    un = un->next_user_node;
                else
                    return cln->chat_list;
            }
            else {
                break;
            }
        }
        cln = cln->next_chat_list_node;
    }
    return NULL;
}

/* Functions for debugging */
void print_user_info(User *user){
    printf("user_info\n");
    printf("username: %s\nconnfd: %d\nstate: %d\n", user->username, user->connfd, user->state);
}

void print_user_list(User_list *user_list){
    User_node *p = user_list->head;
    printf("user_list\n");
    while (p != NULL) {
        printf("%s\n", p->user->username);
        p = p->next_user_node;
    }
}

void print_user_and_chat_node_tuple_list(User_and_chat_node_tuple_list *tuple_list){
    User_and_chat_node_tuple_node *p = tuple_list->head;
    printf("tuple_list\n");
    while (p != NULL) {
        printf("%s\n", p->user_and_chat_node_tuple->user->username);
        p = p->next_user_and_chat_node_tuple_node;
    }
}