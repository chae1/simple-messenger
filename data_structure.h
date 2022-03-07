#ifndef __DS_H__
#define __DS_H__

#include "constant.h"

/* Each signed up user has its User structure in the server which has every current information about the user. 
   When the client logins with the username, server creates a connected file descriptor which is for communicating
   with the client program. And it sets the User structure's connfd with this and changes status. */

typedef struct User{
    char username[WORD_SIZE];
    int connfd;
    int state;
    struct User_list *friend_list;
    struct User_list *request_list;
    struct Chat_list_list *chat_list_list;
    struct Chat_list *current_chat_list;
} User;

/* User_list is a linked list of a pointer to a User. */

typedef struct User_node{
    struct User *user;
    struct User_node *next_user_node;
} User_node;

typedef struct User_list{
    int n;
    struct User_node *head;
    struct User_node *tail;
} User_list;



/* Chat_list and Chat_list_list*/

typedef struct Chat{
    struct User *writer;
    char message[BUF_SIZE];
} Chat;

typedef struct Chat_node{
    struct Chat *chat;
    struct Chat_node *next_chat_node;    
} Chat_node;

/* Each participant of a chat list has the first unread chat node so that when the users enter to the chat list,
   they can be notified what are new messages. In this chat list, those information is stored in User_and_chat_
   node_tuple_list */

typedef struct Chat_list{
    int n;
    struct Chat_node *head;
    struct Chat_node *tail;
    struct User_and_chat_node_tuple_list *participant_and_last_read_chat_node_tuple_list;
} Chat_list;

/* Chat_room_list is a linked list of a pointer to a User and a Chat_list */

typedef struct Chat_list_node{
    struct Chat_list *chat_list;
    struct Chat_list_node *next_chat_list_node;
} Chat_list_node;

typedef struct Chat_list_list{
    int n;
    struct Chat_list_node *head;
    struct Chat_list_node *tail;
} Chat_list_list;



/* User_and_chat_node_tuple_list */

typedef struct User_and_chat_node_tuple{
    User *user;
    Chat_node *chat_node;
} User_and_chat_node_tuple;

typedef struct User_and_chat_node_tuple_node{
    struct User_and_chat_node_tuple *user_and_chat_node_tuple;
    struct User_and_chat_node_tuple_node *next_user_and_chat_node_tuple_node;    
} User_and_chat_node_tuple_node;    

typedef struct User_and_chat_node_tuple_list{
    int n;
    struct User_and_chat_node_tuple_node *head;
    struct User_and_chat_node_tuple_node *tail;
} User_and_chat_node_tuple_list;






User *create_user(char *username);
Chat *create_chat(User *writer, char *message);
User_and_chat_node_tuple *create_user_and_chat_node_tuple(User *user, Chat_node *chat_node);

User_node *create_user_node(User *user);
Chat_node *create_chat_node(Chat *chat);
Chat_list_node *create_chat_list_node(Chat_list *chat_list);
User_and_chat_node_tuple_node *create_user_and_chat_node_tuple_node(User_and_chat_node_tuple *user_and_chat_node_tuple);

User_list *create_user_list();
Chat_list *create_chat_list(User_list *user_list);
Chat_list_list *create_chat_list_list();
User_and_chat_node_tuple_list *create_user_and_chat_node_tuple_list();

void free_user_list(User_list *user_list);

void add_user_to_user_list(User_list *user_list, User *user);
void delete_user_from_user_list(User_list *user_list, User *user);

void add_to_user_and_chat_node_tuple_list(User_and_chat_node_tuple_list *tuple_list, User_and_chat_node_tuple *tuple);
void add_to_chat_list(Chat_list *chat_list, Chat *chat);
void add_to_chat_list_list(Chat_list_list *chat_list_list, Chat_list *chat_list);


User *find_user_by_user(User_list *user_list, User *user);
User *find_user_by_username(User_list *user_list, char *username);
User_and_chat_node_tuple *find_user_and_chat_node_tuple_by_user(User_and_chat_node_tuple_list *user_and_chat_node_tuple_list, User *user);
Chat_list *find_chat_list_by_user_list(Chat_list_list *chat_list_list, User_list *user_list);

void print_user_info(User *user);
void print_user_list(User_list *user_list);
void print_user_and_chat_node_tuple_list(User_and_chat_node_tuple_list *tuple_list);

#endif