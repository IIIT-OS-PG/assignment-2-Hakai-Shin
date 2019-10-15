#include "header.h"
#include <map> 
#include <fcntl.h>

using namespace std;
bool quit=false;
int guard(int n, char * err) { if (n == -1) { perror(err); exit(1); } return n; }
int buildMySocket(string ipaddress,string network_socket_port);
map<int,struct group> groupsinnetwork;

map<string, struct clientinfo> usersaccounts;


vector<string> split(string s,char delimiter)
{   
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
string clientname;
string passwd;
string portnumber;
string ipaddress;
}c;

struct group
{
string group_owner;
map< string, string> groupmember;
map< string ,vector<string> > availablefiles;
};

void listAllfilesinMygroup(int groupid)
{
for(map<string,vector<string>>::iterator it = groupsinnetwork[groupid].availablefiles.begin(); 
        it != groupsinnetwork[groupid].availablefiles.end(); ++it) 
{
  cout << it->first << "\n";
}
}

vector< string > listAllfileownersinfo(int groupid,string filename)
{
string ownername;
string cli;
vector< string> cinfo;
for(int i=0;i<groupsinnetwork[groupid].availablefiles[filename].size();i++)
{
    ownername=groupsinnetwork[groupid].availablefiles[filename][i]; 
    cli=groupsinnetwork[groupid].groupmember[ownername];
    cinfo.push_back(cli);
}
return cinfo;
}



void joingroup(struct group &g,struct clientinfo c)
{
string clientName=c.clientname;
//g.groupmember[clientName]=c;
}

//create clientinfo struct
struct clientinfo createaccount(string name,string ip,int portNumber)
{
    struct clientinfo c;
    c.clientname=name;
    c.ipaddress=ip;
    c.portnumber=portNumber;
    usersaccounts[name]=c;
    return c;
}

void uploadfile(int groupid,string username,string filename)
{
  groupsinnetwork[groupid].availablefiles[filename].push_back(username);
}

struct clientinfo create_user_fun(int trackersocket,vector<string> commands)
{
      struct clientinfo c;
      string ack;
      c.clientname=commands[1];
      c.passwd=commands[2];
      c.ipaddress=commands[3];
      c.portnumber=commands[4];
      usersaccounts[commands[1]]=c;
         ack="[tracker]:user created with username "+c.clientname;
       if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
          return c;
}

void join_group_fun(struct clientinfo c, int trackersocket,vector<string> command)
{
  string ack;
            int groupidx=stoi(command[1]);
            string clientName=c.clientname;
            groupsinnetwork[groupidx].groupmember[clientName]=c.clientname;
        ack="[tracker]:group with groupid " + command[1] + " joined and owner "+groupsinnetwork[groupidx].group_owner;
        if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
}

void create_group_fun(struct clientinfo c, int trackersocket,vector<string> command)
{
  struct group g;
  g.group_owner=c.clientname;
  g.groupmember[c.clientname]=c.clientname;
  string ack;
            int groupidx=stoi(command[1]);
            groupsinnetwork[groupidx]=g;
        ack="[tracker]:group created with groupid " + command[1] + " and owner "+g.group_owner;
        if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
}

struct clientinfo login_fun(int trackersocket,vector<string> command)
{
  struct clientinfo cli;
  string ack;
  cli.clientname=command[1];
  cli.passwd=command[2];
  cli.ipaddress=command[3];
  cli.portnumber=command[4];
  if ( usersaccounts.find(cli.clientname) == usersaccounts.end() ) 
        {
          ack="[tracker]:user name invalid";
           if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
        }
        else
        {
          if(cli.passwd==usersaccounts[cli.clientname].passwd)
          {
           ack="[tracker]:user " + command[1] + " logged in ";
           if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            } 
            usersaccounts[cli.clientname]=cli;
            return cli;
          }
          else
          {
            ack="[tracker]:invalid password ";
            if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
          }
          
        }
}


void upload_file_fun(int trackersocket,struct clientinfo c,vector<string> command)
{
  int groupid;
  string ack;
    string filepath = command[1];  
    groupid=stoi(command[2]);

        if ( groupsinnetwork.find(groupid) == groupsinnetwork.end() ) 
        {
          ack="[tracker]:no such group exists";
           if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
            {
                    printf("Send failed\n");
            }
        } 
        else 
        {
                  
                  if ( groupsinnetwork[groupid].groupmember.find(c.clientname) == groupsinnetwork[groupid].groupmember.end() ) 
                {
                  ack="[tracker]:you are not member of group.";
                  if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
                    {
                            printf("Send failed\n");
                    }
                } 
                else
                {
                  string sgroupid;
                  sgroupid=to_string(groupid);
                  ack="[tracker]:file uploaded in group "+sgroupid;
                    if( send(trackersocket, ack.c_str(), strlen(ack.c_str()), 0) < 0) //send request 
                      {
                              printf("Send failed\n");
                      }

                  uploadfile(groupid,c.clientname,filepath);
                }
        } 
}

void list_files_fun(int trackersocket,vector<string> command)
{
  int groupid;
groupid=stoi(command[1]);
string filename,listoffiles="";
    for(map<string,vector<string>>::iterator it = groupsinnetwork[groupid].availablefiles.begin(); 
        it != groupsinnetwork[groupid].availablefiles.end(); ++it) 
          {
            filename=it->first;
            filename+="|";
           listoffiles=listoffiles+filename;
          }
         if( send(trackersocket, filename.c_str(), strlen(filename.c_str()), 0) < 0) //send request 
                      {
                              printf("Send failed\n");
                      }             
    
}

void download_file_fun(int trackersocket,vector<string> command)
{
  int groupid;
groupid=stoi(command[1]);
string filename;
string seedersinfo="",seedername,seederdata;
filename=command[2];
vector<string> seederlist;
seederlist=groupsinnetwork[groupid].availablefiles[filename];
for(int i=0;i<seederlist.size();i++)
{
  seedername=seederlist[i];
  if ( usersaccounts.find(seedername) != usersaccounts.end() ) 
        {
          seederdata=usersaccounts[seedername].ipaddress;
          seederdata+=":";
          seederdata+=usersaccounts[seedername].portnumber;
          seederdata+="|";
          seedersinfo=seedersinfo+seederdata;
        }   
}   
         if( send(trackersocket, seedersinfo.c_str(), strlen(seedersinfo.c_str()), 0) < 0) //send request 
                      {
                              printf("Send failed\n");
                      }              
}

void trackerfunction(int trackersocket)
{
char buffer[1024];

 char req[1024];
    bzero(req, 1024);
    int ReqLen =recv(trackersocket, req, 1024, 0);
    string msg = req;
    vector<string> message = split(msg,'|');
    string comm = message[0];
    //cout<<comm<<endl;



  // Send message to the client socket 
if(ReqLen < 0)
    {
       printf("Receive failed\n");
    }
else
{
if(comm=="create_user")
{
      cout<<"[tracker]:received create_user request from port '"<<message[4]<<"'"<<endl;
      c=create_user_fun(trackersocket,message);
}     

if( comm =="create_group")
{
  cout<<"[tracker]:received create_group request from user '"<<c.clientname<<"'"<<endl;
  create_group_fun(c,trackersocket,message);
}

if(comm=="upload_file")
{
  
  cout<<"[tracker]:received upload file request from user '"<<c.clientname<<"'"<<endl;
  upload_file_fun(trackersocket,c,message);
}
if(comm=="list_files")
{

  cout<<"[tracker]:received list of sharable file request from user '"<<c.clientname<<"'"<<endl;
  list_files_fun(trackersocket,message);
}

if(comm=="download_file")
{

  cout<<"[tracker]:received download file request from user '"<<c.clientname<<"'"<<endl;
  download_file_fun(trackersocket,message);
}

if( comm =="join_group")
{
  cout<<"[tracker]:received join_group request from user '"<<c.clientname<<"'"<<endl;
  join_group_fun(c,trackersocket,message);;

}
if(comm=="login")
{
      cout<<"[tracker]:received login request from user '"<<message[1]<<"'"<<endl;
      c=login_fun(trackersocket,message);
}


/*
else if(strcmp(buffer,"list_files​")==0)
{
  list_files_fun(trackersocket);
}

else
{
  cout<<"invalid command"<<endl;
}
*/
}

}


void servermode(int peersocket) 
{
    struct sockaddr_in clientaddr;
    
    socklen_t addrlen = sizeof(clientaddr);
  //  int flags = guard(fcntl(peersocket, F_GETFL), "could not get flags on TCP listening socket");
   // guard(fcntl(peersocket, F_SETFL, flags | O_NONBLOCK), "could not set TCP listening socket to be non-blocking");
    //Listen on the socket, with 10 max connection requests queued 
   if (listen(peersocket, 100) == -1) 
    {
        perror("Listen Error");
        exit(1);
    }
       
 int new_socket_fd;
while(!quit) {

    //Accept call creates a new socket for the incoming connection
    cout<<"waiting to accept"<<endl;
    new_socket_fd = accept(peersocket, (struct sockaddr *) &clientaddr, &addrlen);
    if (new_socket_fd == -1) {
      if (errno == EWOULDBLOCK) {
        //printf("[tracker]:No pending connections; sleeping for one second.\n");
        sleep(1);
      } else {
        perror("error when accepting connection");
        cout<<"[tracker]:server thread stopped....";
        exit(1);
      }
    } else {
        cout<<"[tracker]:Connection established....."<<endl;
        thread serverthread(trackerfunction, new_socket_fd); 
        serverthread.detach(); 
      // thread creation;
    }
  }
 
}

int main(int argc,char* argv[])
{
 

//a. Run Tracker:
//./tracker​ tracker_info.txt ​ tracker_no
//tracker_info.txt - Contains ip, port details of all the trackers
if(argv[1]==NULL)
{
    cout<<"[tracker]-incorrect format- port ? tracker info file not specified"<<endl;
    //exit(EXIT_FAILURE);   
}
string port="9879";
//uncomment this!!!!!
/*
if(argv[2]==NULL)
{
    cout<<"[tracker]-incorrect format-tracker number not specified"<<endl;
    exit(EXIT_FAILURE);   
}
*/
string tracker_info_file=argv[1];
//change this to portnumber!!!!!!
int trackersocket=buildMySocket("127.0.0.1",tracker_info_file);

thread newserverthread(servermode, trackersocket); 
newserverthread.detach(); 


//b. Close Tracker:
//quit
string userinput;
getline(cin,userinput);
if(userinput=="quit")
{
    quit=true;
    cout<<"[tracker]-quit command executed."<<endl;
    exit(EXIT_SUCCESS);
}



else
{
    cout<<"[tracker]-No such command."<<endl;
}


    return 0;
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
