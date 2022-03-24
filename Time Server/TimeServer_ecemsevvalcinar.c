#include <stdio.h>
#include <string.h> // for strlen
#include <sys/socket.h>
#include <arpa/inet.h> // for inet_addr
#include <unistd.h>    // for write
#include <stdlib.h>
#include <time.h>
#include <string.h>

// reference
//https://www.geeksforgeeks.org/strftime-function-in-c/
struct time
{
    int tm_sec;   // seconds
    int tm_min;   // minutes
    int tm_hour;  // hours
    int tm_mday;  // day of the month
    int tm_mon;   // month
    int tm_year;  // The number of years since 1900
    int tm_wday;  // day of the week
    int tm_yday;  // day in the year
    int tm_isdst; // daylight saving time
} stTime;

int main(int argc, char *argv[])
{
    time_t t;
    struct tmm *tmp;

    char MY_TIME[3000];
    // https://man7.org/linux/man-pages/man3/strftime.3.html
    // array is created to store elements which strftime accepts
    char *accepted[] = {"%a", "%A", "%b", "%B", "%c", "%C", "%d", "%D", "%e", "%E",
                        "%F", "%G", "%g", "%h", "%H", "%I", "%j", "%k", "%l", "%m", "%M", "%n", "%O", "%p", "%P",
                        "%r", "%R", "%s", "%S", "%t", "%T", "%u", "%U", "%V", "%w", "%W",
                        "%x", "%X", "%y", "%Y", "%z", "%Z",
                        "%Ec", "%EC", "%Ex", "%EX", "%Ey", "%EY", "%Od", "%Oe", "%OH", "%OI", "%Om"};

    // another array is created to store elements which strftime does not accept
    // they are numbers and other turkish chars and also "f"
    char *rejected[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "f", "ü", "ğ", "q", "i", "ö", "ş", "ç", "o","&"};

    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *message;
    char received_message[3000];

    // Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;         //IPv4 Internet protocols
    server.sin_addr.s_addr = INADDR_ANY; //IPv4 local host address
    server.sin_port = htons(8888);       // server will listen to port

    // Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }
    puts("Socket is binded");

    // Listen
    listen(socket_desc, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");

    c = sizeof(struct sockaddr_in);

    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
    if (new_socket < 0)
    {
        puts("Accept failed");
        return 1;
    }

    puts("Connection accepted");
    message = "To exit, please type exit.\n";
    write(new_socket, message, strlen(message));
    message = "Please do not forget to type 'GET_DATE' in first. Your command text has to be, for example, 'GET_DATE %H'.\n";
    write(new_socket, message, strlen(message));

    while (1)
    {
        message = "Type your request:\n";
        write(new_socket, message, strlen(message));  
        recv(new_socket, &received_message, 2000, 0); // getting input
        
        // if input is exit
        if (received_message[0] == 'e' && received_message[1] == 'x' && received_message[2] == 'i' && received_message[3] == 't')
        {
            break;
        }

        int myFlag = 1; // true
        // input has to start with GET_DATE
        if (!(received_message[0] == 'G' && received_message[1] == 'E' && received_message[2] == 'T' && received_message[3] == '_' && received_message[4] == 'D' && received_message[5] == 'A' && received_message[6] == 'T' && received_message[7] == 'E' && received_message[8]==' '))
        {
            myFlag = 0;
            
        }

        // if 'GET_DATE' is not in the first place, do not check for accepted strings, because request is already incorrect
        if (myFlag == 1)
        {
            int lenAccepted = sizeof(accepted) / sizeof(accepted[0]);
            for (int i = 0; i < lenAccepted; ++i)
            {
                if (strstr(received_message, accepted[i]) != NULL)
                {
                    myFlag = 1;
                }
            }
            // check for rejected strings
            int len2 = sizeof(rejected) / sizeof(rejected[0]);
            for (int i = 0; i < len2; i++)
            {
                if (strstr(received_message, rejected[i]) != NULL)
                {
                    myFlag = 0; // if one is found, incorrect request
                    break;
                }
            }
        }

        if (myFlag == 0) // outpu for incorrect input
        {
            message = "Incorrect Request\n";
            write(new_socket, message, strlen(message));
        }
        else // (myFlag == 1)
        {
            time(&t);
            tmp = localtime(&t);
            strftime(MY_TIME, sizeof(MY_TIME), received_message, tmp);

            char *messageWithoutGetDate = malloc(strlen(MY_TIME));
            strncpy(messageWithoutGetDate, MY_TIME + 9, strlen(MY_TIME)); // delete the GET_DATE part from output
            write(new_socket, messageWithoutGetDate, strlen(messageWithoutGetDate));
        }
    }

    close(socket_desc);
    close(new_socket);

    return 0;
}