#include <sys/types.h>         //contains socket function used to create a socket
#include <sys/socket.h>        //contains domain argument which specifies a communication domain; this selects the protocol family which will be used for communication
#include <netinet/in.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <bits/stdc++.h>
#include<string.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <thread>
#include <iostream>
#include <vector>

void handle_error(char *msg)
{
perror(msg); 
}

