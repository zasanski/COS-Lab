#include <arpa/inet.h>   // For inet_ntoa
#include <netinet/in.h>  // For sockaddr_in, INADDR_ANY, htons
#include <stdio.h>       // For printf, fprintf, perror, snprintf
#include <stdlib.h>      // For EXIT_SUCCESS, EXIT_FAILURE, atoi
#include <string.h>      // For memset, strlen
#include <sys/socket.h>  // For socket, bind, recvfrom, sendto
#include <unistd.h>      // For close

// Function to parse and calculate the expression
int calculate_expression(const char *expression_str, long long *result) {
  int num1, num2;
  char op;

  // Parse the expression: Expects format like "13*3"
  if (sscanf(expression_str, "%d%c%d", &num1, &op, &num2) != 3) {
    return -1;  // Parsing failed
  }

  switch (op) {
    case '+':
      *result = (long long)num1 + num2;
      break;
    case '-':
      *result = (long long)num1 - num2;
      break;
    case '*':
      *result = (long long)num1 * num2;
      break;
    case '/':
      if (num2 == 0) {
        return -2;  // Division by zero
      }
      *result = (long long)num1 / num2;
      break;
    default:
      return -3;  // Invalid operator
  }
  return 0;
}

// Function to prepare the response string based on calculation status
void prepare_response(int calc_status, long long result_value, char *response_buffer, size_t buffer_size) {
  switch (calc_status) {
    case 0:  // Success
      snprintf(response_buffer, buffer_size, "Result: %lld", result_value);
      break;
    case -1:  // Parsing error
      snprintf(response_buffer, buffer_size, "Error: Invalid expression format. Use e.g., '13*3'.");
      break;
    case -2:  // Division by zero
      snprintf(response_buffer, buffer_size, "Error: Division by zero");
      break;
    case -3:  // Invalid operator
      snprintf(response_buffer, buffer_size, "Error: Invalid operator. Use +, -, *, /.");
      break;
    default:  // Unexpected error
      snprintf(response_buffer, buffer_size, "Error: Unexpected error.");
      break;
  }
}

int main(int argc, char *argv[]) {
  int sockfd;
  int port;
  char buffer[BUFSIZ];
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len;
  long long calculation_result;
  char response_str[BUFSIZ];

  // 1. Check command-line arguments for port number
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port_number>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port = atoi(argv[1]);
  if (port <= 0 || port > 65535) {
    fprintf(stderr, "Invalid port number: %d. Please use a port between 1 and 65535.\n", port);
    exit(EXIT_FAILURE);
  }

  printf("Starting Wolfram Lite UDP Server on port %d...\n", port);

  // 2. Create UDP socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }
  printf("Socket created successfully.\n");

  // 3. Configure server address structure
  memset(&server_addr, 0, sizeof(server_addr));  // Clear structure
  server_addr.sin_family = AF_INET;              // IPv4
  server_addr.sin_addr.s_addr = INADDR_ANY;      // Listen on all available network interfaces
  server_addr.sin_port = htons(port);            // Convert port to network byte order

  // 4. Bind the socket to the server address and port
  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("Socket bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }
  printf("Socket bound to port %d.\n", port);

  // 5. Main server loop: Receive, compute, send
  printf("Server listening for expressions...\n");
  while (1) {
    memset(buffer, 0, BUFSIZ);              // Clear buffer for new message
    client_addr_len = sizeof(client_addr);  // Initialize client address length

    // Receive message from client
    ssize_t bytes_received =
        recvfrom(sockfd, (char *)buffer, BUFSIZ - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
    if (bytes_received == -1) {
      perror("recvfrom failed");
      continue;
    }
    // Null-terminate the received data
    buffer[bytes_received] = '\0';

    // Process expression
    printf("\nReceived from %s:%d: \"%s\"\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
    int calc_status = calculate_expression(buffer, &calculation_result);
    prepare_response(calc_status, calculation_result, response_str, BUFSIZ);

    // Send response back to the client
    ssize_t bytes_sent = sendto(sockfd, (const char *)response_str, strlen(response_str), 0,
                                (const struct sockaddr *)&client_addr, client_addr_len);
    if (bytes_sent == -1) {
      perror("sendto failed");
    } else {
      printf("Sent response to %s:%d: \"%s\"\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
             response_str);
    }
  }

  close(sockfd);
  return EXIT_SUCCESS;
}
