#include "header.h"
#include <stdio.h>
#include <string.h>
#define ARG_BUFFER 64
#define ARG_CHAR 1024   
#define CHUNKSIZE 524288
#define MAX_LINE 524288
using namespace std;
int buildMySocket(string ipaddress,string port);
vector<string> split(string s);
map<string,vector<int>> myfiles;

vector<string> splitbar(string s)
{   
     char delimiter='|';
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

struct clientinfo
{
    string ip;
    string port;
};

int sendfile(FILE *fp, int sockfd,int pieceNumber) 
{
    int sendchunk; 
    char sendline[MAX_LINE] = {0}; 
    int seekpoint=pieceNumber*CHUNKSIZE;
    fseek(fp,seekpoint , SEEK_SET);
    sendchunk = fread(sendline, sizeof(char), MAX_LINE, fp);

    if(sendchunk<MAX_LINE)
    {
      //if the data read into buffer is less than 4kb
      if(sendchunk<4096)
      {
         if (send(sockfd, sendline, sendchunk , 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        } 
      }
      else
      {
        int sendpiece=sendchunk/4096;
        int i;
        for(i=0;i<sendpiece;i++)
        {
                if (send(sockfd, sendline + i*4096, 4096 , 0) == -1)
              {
                  perror("Can't send file");
                  exit(1);
              }
         }
          if (send(sockfd, sendline + i*4096, sendchunk-i*4096 , 0) == -1)
              {
                  perror("Can't send file");
                  exit(1);
              }
      }
    }

    else if(sendchunk==MAX_LINE)
    {
      for(int i=0;i<128;i++)
      {
           if (send(sockfd, sendline + i*4096, 4096 , 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
      }

    }
    fseek(fp, 0, SEEK_SET);
  return sendchunk;
}

long getfilesize(string filename)
{
  FILE *f;
  const char* file=filename.c_str();
    f = fopen(file, "r");
    fseek(f, 0, SEEK_END);
    long filesize=0;
    filesize=ftell(f);
    fclose(f);
    return filesize;
}

void file_size_fun(int peersocket,vector<string> command)
{
string ack;
string filename=command[1];
long filesize=getfilesize(filename);
string sfilesize=to_string(filesize);
ack=sfilesize;
ack+="|";
ack+="done";
ack+="|";
       if( send(peersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
}

void send_file_fun(int peersocket,vector<string> details)
{
  ssize_t total;
//details[1]=filename,details[2]=chunkcount,details[3]=chunkmod,details[4]=peernum
char *cfilename = new char[details[1].length() + 1];
strcpy(cfilename, details[1].c_str());

FILE *fp = fopen(cfilename, "rb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
long chunkcount=stoi(details[2]);
int chunkmod=stoi(details[3]);
int peernum=stoi(details[4]);
for(int i=chunkmod;i<=chunkcount;i=i+peernum)
total+=sendfile(fp, peersocket,i);
cout<<"total bytes sent ";


}

void peerfunction(int peersocket)
{
char buffer[1024];

 char req[1024];
    bzero(req, 1024);
    int ReqLen =recv(peersocket, req, 1024, 0);
    string msg = req;
    vector<string> message = splitbar(msg);
    string comm = message[0];
    cout<<"received "<<comm<<endl;
  // Send message to the client socket 
if(ReqLen < 0)
    {
       printf("Receive failed\n");
    }
else
{
if(comm=="file_size")
{
      cout<<"[peer]:received filesize request"<<endl;
      file_size_fun(peersocket,message);
}  
if(comm=="send_file")
{
   cout<<"[peer]:received sendfile request"<<endl;
      send_file_fun(peersocket,message);
}   

}

}



void servermode(int peersocket) 
{
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    //Listen on the socket, with 10 max connection requests queued 
   if (listen(peersocket, 10) == -1) 
    {
        perror("Listen Error");
        exit(1);
    }
    while(1)
    {
        //Accept call creates a new socket for the incoming connection
        cout<<"[peer-server]:waiting to accept"<<endl;
        int newsocket =  accept(peersocket, (struct sockaddr *) &clientaddr, &addrlen);
        //for each client request creates a thread and assign the client request to it to process
       //so the main thread can entertain next request
       cout<<"[peer-server]:Connection established"<<endl;
       thread serverthread(peerfunction, newsocket); 
       serverthread.detach(); 
    }
}
//-------------------------------------------------------------------
void create_user_fun(int trackerfd,vector<string> command_statements,struct clientinfo c)
{
string command="create_user|";
command+=command_statements[1];
command+="|";
command+=command_statements[2];
command+="|";
command+=c.ip;
command+="|";
command+=c.port;
command+="|";
if( send(trackerfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[1024]={0};
if(recv(trackerfd, ack, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
cout<<ack<<endl;
}
//-------------------------------------------------------------------------------------

void create_group_fun(int trackerfd,vector<string> command_statements)
{
string commands="create_group|";
commands+=command_statements[1];
commands+="|";
if( send(trackerfd, commands.c_str(), strlen(commands.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char buffer[1024]={0};
if(recv(trackerfd, buffer, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
cout<<buffer<<endl;
}

void join_group_fun(int trackerfd,vector<string> command_statements)
{
string commands="join_group|";
commands+=command_statements[1];
commands+="|";
if( send(trackerfd, commands.c_str(), strlen(commands.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char buffer[1024]={0};
if(recv(trackerfd, buffer, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
cout<<buffer<<endl;
}

void login_fun(int trackerfd,vector<string> command_statements,struct clientinfo c)
{
string command="login|";
command+=command_statements[1];
command+="|";
command+=command_statements[2];
command+="|";
command+=c.ip;
command+="|";
command+=c.port;
command+="|";
if( send(trackerfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[1024]={0};
if(recv(trackerfd, ack, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
cout<<ack<<endl;
}


//--------------------------------------------------------------------------
//   get number of chunks of file
long getchunkcount(string filename)
{
  FILE *f;
  char * cstr = new char [filename.length()+1];
  strcpy (cstr, filename.c_str());
    f = fopen("odin.mp4", "r");
    //f = fopen(cstr, "r");
    fseek(f, 0, SEEK_END);
    long chunkcount =0,filesize=0;
    filesize=ftell(f);
    long b=512*1024;
    chunkcount=(long)ceil(filesize/((float)b));
    fclose(f);
    return chunkcount;
}



void upload_file_fun(int trackerfd,vector<string> command_statements)
{
  string filename=command_statements[1];
long chunkcount=getchunkcount(filename);
vector<int> chunkdetails(chunkcount,1);
myfiles[filename]=chunkdetails;

string command="upload_file|";
command+=command_statements[1];
command+="|";
command+=command_statements[2];
command+="|";
if( send(trackerfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[1024]={0};
if(recv(trackerfd, ack, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
cout<<ack<<endl;
}

void list_files_fun(int trackerfd,vector<string> command_statements)
{
  vector<string> files;
  string command="list_files|";
command+=command_statements[1];
command+="|";
if( send(trackerfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[4096]={0};
if(recv(trackerfd, ack, 4096, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
  string recvfile=ack;
 char delimiter='|';
   string token;
   istringstream tokenStream(recvfile);
   while (getline(tokenStream, token, delimiter))
   {
      files.push_back(token);
   }
   cout<<"[peer]:files in group"<<endl;
   int i;
   for(i=0;i<files.size();i++)
   {
     cout<<"-->"<<files[i]<<endl;
   }
}

void getfileinfo(string downloadfile,long long int &filesize,string peerinfo)
{
char *cpeer = new char[peerinfo.length() + 1];
strcpy(cpeer, peerinfo.c_str());
char* peer_ipaddress = strtok(cpeer, ":");
int peer_port =atoi(strtok(NULL, ":"));
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in serveraddr;
memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(peer_ipaddress);
    serveraddr.sin_port = htons(peer_port);
    int connectioncount=0;
    while(connectioncount<100)
    {
    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
      connectioncount++;
        continue;
    }
    else
    {
      break;
    }
    }
    if(sockfd==-1)
    cout<<"not able to connect"<<endl;
    else
    cout<<"Connection successfull"<<endl;
     cout<<endl;
string command="file_size|";
command+=downloadfile;
command+="|";
if( send(sockfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[1024];
bzero(ack, 1024);
if(recv(sockfd, ack, 1024, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }

string recvfilesize = ack;         
vector<string> p = splitbar(recvfilesize);
filesize=stoi(p[0]);
}

void createdumpfile(string filen,long long int filesize)
{
  char * cstr = new char [filen.length()+1];
  strcpy (cstr, filen.c_str());
  FILE *fp;
  char buffer[1]="";
  fp=fopen(cstr,"w+t");
  fseek(fp, filesize-1, SEEK_SET);
  fwrite (buffer , sizeof(char), sizeof(buffer), fp);
  fclose(fp);
}

int writefile(int sockfd, FILE *fp,int pieceNumber)
{
    ssize_t n;
    char buff[4096] = {0};
    cout<<endl;
   // cout<<"[client]:writing process begun...";
    cout<<endl;
    int seekpoint=pieceNumber*CHUNKSIZE;
    fseek(fp,0 , SEEK_SET);
    fseek(fp,seekpoint , SEEK_SET);

    n = recv(sockfd, buff, 4096, 0);
    if(n>0){
            //cout<<n<<" Bytes received for chunknumber "<<pieceNumber<<endl;
            while (n > 0) 
            {
              
                if(n>0)
                {
                  //pthread_mutex_lock(&lock);
                            int written=fwrite(buff, sizeof(char), n, fp); 
  
                 // pthread_mutex_unlock(&lock);
                     
                    if (written!= n)
                    {
                        perror("Write File Error");
                        exit(1);
                    }
                    else
                    {
                     // cout<<written<<" bytes written successfully on file"<<endl;
                      memset(buff, 0, 4096);
                      n = recv(sockfd, buff, 4096, 0);
                     // cout<<n<<" Bytes received for chunknumber "<<pieceNumber<<endl;
                    }
                    
                    
                }
                  else
                  {
                    cout<<"didnt receive any data"<<endl;
                  }

            }
    }
    else
    {
       cout<<"didnt receive any data"<<endl;
    }
    
    fseek(fp, 0, SEEK_SET);
    return n;
}

void connectwithpeers(string downloadfile,string downloadpath,long chunkcount,int chunkmod,int peernum,
                      string peerinfo)
{
  int total=0;
  char *cpeer = new char[peerinfo.length() + 1];
strcpy(cpeer, peerinfo.c_str());
char* peer_ipaddress = strtok(cpeer, ":");
int peer_port =atoi(strtok(NULL, ":"));
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
struct sockaddr_in serveraddr;
memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(peer_ipaddress);
    serveraddr.sin_port = htons(peer_port);
    int connectioncount=0;
    while(connectioncount<100)
    {
    if (connect(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    {
      connectioncount++;
        continue;
    }
    else
    {
      break;
    }
    }
    if(sockfd==-1)
    cout<<"not able to connect"<<endl;
    else
    cout<<"Connection successfull with seeder"<<peerinfo<<endl;

    char *cpath = new char[downloadpath.length() + 1];
strcpy(cpath, downloadpath.c_str());
     
  FILE *fp = fopen(cpath, "r+t");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
    
    string ack="send_file|";
    ack+=downloadfile;
    ack+="|";
    ack+=to_string(chunkcount);
    ack+="|";
    ack+=to_string(chunkmod);
    ack+="|";
    ack+=to_string(peernum);
    ack+="|";

      //send filename|numberofchunks|chunkmod|peernum
    if( send(sockfd, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }

    for(int i=chunkmod;i<=chunkcount;i=i+peernum)
    {
      cout<<"piecenumber "<<i<<peerinfo<<"peerinfo"<<endl;
    total=total+writefile(sockfd, fp,i);
    }
    printf("Receive Success, NumBytes = %ld\n", total);
    fclose(fp);
}


void downloadHandler(vector<string> seederlist,string downloadpath,string downloadfile)
{
  long long int filesize=0;
int downloadthreads=seederlist.size(),threadcount=0;
/*
while(threadcount<downloadthreads)
{
  string peerinfo=seederlist[threadcount];
  thread comthread(getfileinfo,downloadfile,filesize,peerinfo);
  comthread.detach();
  threadcount++;
}
*/
string peerinfo=seederlist[threadcount];
getfileinfo(downloadfile,filesize,peerinfo);
cout<<"file size "<<filesize<<endl;
createdumpfile(downloadpath,filesize);
long chunkcount;

long b=512*1024;
chunkcount=(long)ceil(filesize/((float)b));

cout<<"chunkcount "<<chunkcount<<endl;
vector<int> chunkdetails(chunkcount,0);
myfiles[downloadfile]=chunkdetails;
threadcount=0;

while(threadcount<downloadthreads)
{
  thread comthread(connectwithpeers,downloadfile,downloadpath,chunkcount,
                    threadcount,downloadthreads,seederlist[threadcount]);
  comthread.detach();
  threadcount++;
}

}

void download_file_fun(int trackerfd,vector<string> command_statements)
{
  vector<string> seederlist;
  string downloadpath=command_statements[3];
  string command="download_file|";
command+=command_statements[1];
command+="|";
command+=command_statements[2];
command+="|";
if( send(trackerfd, command.c_str(), strlen(command.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
char ack[4096]={0};
if(recv(trackerfd, ack, 4096, 0) < 0) //recieve ack
            {
              printf("Receive failed\n");
            }
  string recvfile=ack;
 char delimiter='|';
   string token;
   istringstream tokenStream(recvfile);
   while (getline(tokenStream, token, delimiter))
   {
      seederlist.push_back(token);
   }
   cout<<"ack received "<<ack<<endl;
   string downloadfile=command_statements[2];
   cout<<"file to download "<< downloadfile<<endl;
   cout<<"destination "<<downloadpath<<endl;
  downloadHandler(seederlist,downloadpath,downloadfile);

}




int main(int argc,char* argv[])
{
 
 int peerSocketDescriptor;

 //a. Run Client:
 if(argc<2)
 {
     //./Client​ <IP>:<PORT> tracker_info.txt
     //tracker_info.txt - Contains ip, port details of all the trackers
     cout<<"Invalid Format : <CLIENT_IP>:<UPLOAD_PORT> <tracker_info_file>"<<endl;
    //exit(EXIT_FAILURE);
 }   
 

     int trackerfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in trackeraddr;
    memset(&trackeraddr, 0, sizeof(trackeraddr)); 
    trackeraddr.sin_family = AF_INET;
    trackeraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //cout<<"tracker port:";
    int trackerport;
    trackerport=atoi(argv[2]);
    trackeraddr.sin_port = htons(trackerport);

    string peer_ipaddress = string(strtok(argv[1], ":"));
    string peer_port =string(strtok(NULL, ":"));
    struct clientinfo c;
    c.ip=peer_ipaddress;
    c.port=peer_port;

    peerSocketDescriptor=buildMySocket(peer_ipaddress,peer_port);
    cout<<"[peer]:socket build success"<<endl;
    sleep(1);
    cout<<"[peer]:launching server"<<endl;
    
   thread newserverthread(servermode, peerSocketDescriptor);  
    newserverthread.detach();
  

if(trackerfd==-1)
    cout<<"[peer]:not able to connect"<<endl;
    else
    cout<<"[peer]:Connection with tracker successful"<<endl;
vector<string> command_statements;

bool logout=false;
   
string str;
while(!logout)
{
  cout<<"Enter Command:"<<endl;
  getline(cin,str);
  

command_statements=split(str);
string command=command_statements[0];
//cout<<"command is"<<command<<endl;
//cout<<command;
//b. Create User Account:
//create_user​ <user_id> <passwd>
if(command=="create_user")
{
  int createuserfd=socket(AF_INET, SOCK_STREAM, 0);
  while(1){
    
    if (connect(createuserfd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
      cout<<"trying"<<endl;
        continue;
    }
    else
    {
      cout<<"done"<<endl;
      break;
    }
    }
create_user_fun(createuserfd,command_statements,c);
sleep(2);
close(createuserfd);
}

//d. Create Group:
//create_group​ <group_id>
else if(command=="create_group")
{
  int creategroupfd=socket(AF_INET, SOCK_STREAM, 0);
  while(1)
  {
    
    if (connect(creategroupfd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
        continue;
    }
    else
    {
      break;
    }
    }
 create_group_fun(creategroupfd,command_statements);
sleep(2);
close(creategroupfd);  
}

//k. Upload File:
//upload_file​ <file_path> <group_id>

else if(command=="upload_file")
{
  int uploadfilefd=socket(AF_INET, SOCK_STREAM, 0);

   while(1)
   {
    
    if (connect(uploadfilefd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
        continue;
    }
    else
    {
      break;
    }
    }
 
  upload_file_fun(uploadfilefd,command_statements); 
  
close(uploadfilefd);  

}


//j. List All sharable Files In Group:
//list_files​ <group_id>
else if(command=="list_files")
{   
   int listfilefd=socket(AF_INET, SOCK_STREAM, 0);

   while(1){
    
    if (connect(listfilefd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
        continue;
    }
    else
    {
      break;
    }
    }
 
  list_files_fun(listfilefd,command_statements); 
  
close(listfilefd);   

}


//l. Download File:
//download_file​ <group_id> <file_name> <destination_path>
else if(command=="download_file")
{   
   int downloadfilefd=socket(AF_INET, SOCK_STREAM, 0);

   while(1){
    
    if (connect(downloadfilefd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
        continue;
    }
    else
    {
      break;
    }
    }
 
  download_file_fun(downloadfilefd,command_statements); 
  
close(downloadfilefd);   

}


//c. Login:
//login ​ <user_id> <passwd>
else if(command=="login")
{
  int loginfd=socket(AF_INET, SOCK_STREAM, 0);
  while(1){
    
    if (connect(loginfd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
      cout<<"trying"<<endl;
        continue;
    }
    else
    {
      cout<<"done"<<endl;
      break;
    }
    }
login_fun(loginfd,command_statements,c);
sleep(2);
close(loginfd);
}


//e. Join Group:
//join_group​ <group_id>
else if(command=="join_group")
{   
   int joingroupfd=socket(AF_INET, SOCK_STREAM, 0);

   while(1){
    
    if (connect(joingroupfd, (const struct sockaddr *) &trackeraddr, sizeof(trackeraddr)) < 0)
    {
        continue;
    }
    else
    {
      break;
    }
    }
 
  join_group_fun(joingroupfd,command_statements); 
  
close(joingroupfd);   
}

//f. Leave Group:
//leave_group​ <group_id>
else if(command=="leave_group​")
{
cout<<"group left";    
}

//g. List pending join requests
//list_requests ​ <group_id>
else if(command=="list_requests")
{
  cout<<"list of request";  
}

//h. Accept Group Joining Request:
//accept_request​ <group_id> <user_id>
else if(command=="accept_request")
{
  cout<<"accept request";  
}


//i. List All Group In Network:
//list_groups
else if(command=="list_groups")
{
   //list_groups_fun(trackerfd); 
}

else if(command=="logout")
{
   logout=true;
}



/*n. Show_downloads
Show_downloads
Output format:
[D] [grp_id] filename
[C] [grp_id] filename
D(Downloading), C(Complete)
*/
else if(command=="Show_downloads")
{
 cout<<"download status";   
}

//o. Stop sharing
//stop_share ​ <group_id> <file_name>
else if(command=="stop_share")
{
    
}

else
{   
    //if incorrect command entered
    cout<<"[peer]:Command not found"<<endl;
}
command_statements.clear();
}

//newserverthread.join();
cout<<"[peer]:server thread returned..."<<endl;
sleep(1);
cout<<"[peer]:loggin off..."<<endl;
return 0;
}

vector<string> split(string s)
{   
     char delimiter=' ';
   vector<string> tokens;
   string token;
   istringstream tokenStream(s);
   while (getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}


int buildMySocket(string ipaddress,string network_socket_port)
{
    //create a socket
    //socket(domian,type,protocol)- socket() creates an endpoint for communication 
    //and returns a file descriptor that refers to that endpoint.
    //  
    //domain-create a socket which communicates with other sockets
    // containing address in the AF_INET family 
    //
    //type- type, which specifies the communicationsemantics. 
    // Sockets of type SOCK_STREAM are full-duplex byte streams.
    //The communications protocols which implement a SOCK_STREAM ensure that data is not lost or duplicated
    //
    //Protocol-The protocol specifies a particular protocol to be used with the socket-0 is set automactically.
    //
    // network_socket is a descriptor which refers to the socket we create
    int network_socket,new_network_socket,addrlen = sizeof(sockaddr);
    network_socket=socket(AF_INET,SOCK_STREAM,0);
    if(network_socket==-1)
        handle_error("Cannot create socket");
    
    struct sockaddr_in server_socket_addr_struct, client_socket_addr_struct;
    server_socket_addr_struct.sin_family = AF_INET;
    //Network Byte Order: High-order byte of the number is stored in memory 
    //at the lowest address. Network stack (TCP/IP) expects Network Byte Order
    //so we convert host 
    
    server_socket_addr_struct.sin_port=htons(stoi(network_socket_port));
    server_socket_addr_struct.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    //int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen)
    //
    //When a socket is created with socket(), it exists in a name space
    //(address family) but has no address assigned to it.  bind() assigns
    //the address specified by "addr" to the socket referred to by the file
    //descriptor "sockfd".  "addrlen" specifies the size, in bytes, of the
    //address structure pointed to by "addr".  Traditionally, this operation
    //is called “assigning a name to a socket”.
    int  bind_status = bind(network_socket , (struct sockaddr*)&server_socket_addr_struct , sizeof(server_socket_addr_struct));
    
    
    if ( bind_status < 0 )
        handle_error("Cannot bind socket with address");

    return network_socket;
    


}

