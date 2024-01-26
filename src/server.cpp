#include <_types/_uint8_t.h>
#include <cstddef>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>

const uint8_t PATH_INDEX = 1;

std::string extractPath(char* buff)
{
  // Input should as follow: GET /index.html HTTP/1.1\r\nHost: localhost:4221\r\nUser-Agent: curl/7.64.1\r\n
  std::vector<std::string> lines;
  std::string line;

  if(buff == NULL)
  {
    std::cerr << __PRETTY_FUNCTION__ << " buffer passed is NULL!!\n";
  }

  std::istringstream iss(buff);
  while(iss >> line)
  {
    lines.push_back(line);
  }

  return lines[PATH_INDEX];
}

int main(int argc, char **argv) {
  // You can use print statements as follows for debugging, they'll be visible when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
   std::cerr << "Failed to create server socket\n";
   return 1;
  }
  
  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);
  
  if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }
  
  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }
  
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  
  std::cout << "Waiting for a client to connect...\n";
  
  int client_socket = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
  std::cout << "Client connected\n";

  if(client_socket == -1)
  {
    std::cerr << "accept failed\n";
    return 1;
  }

  char buffer[1024];
  while(ssize_t bytes_recieved = recv(client_socket, buffer, sizeof(buffer), 0))
  {
    if(bytes_recieved == -1)
    {
      std::cerr << "reception failed\n";
      return 1;
    }

    std::string path = extractPath(buffer);

    std::string http_response;
    if(path.compare("/"))
    {
      http_response = "HTTP/1.1 200 OK\r\n\r\n";
    }
    else {
      http_response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }

    
    ssize_t bytes_sent = send(client_socket, http_response.c_str(), strlen(http_response.c_str()), 0);
    if (bytes_sent == -1) {
      std::cerr << "send failed\n";
      return 1;
    }

    memset(buffer, 0, sizeof(buffer));
  }

  close(client_socket);
  close(server_fd);

  return 0;
}
