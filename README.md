# Chatroom
A chatbox which connects different severs where people can chat .They can join and leave the chatroom as pe there wish .
A chatroom built in C++ using the concepts of socket programming and multi-threading. It supports chatting among multiple clients.
## How to run

1. Clone this repository
2. Run the following commands in your terminal :
```
g++ server.cpp -lpthread -o server
g++ client.cpp -lpthread -o client
```
3. To run the server application, use this command in the terminal :
```
./server
```

4. Now, open another terminal and use this command to run the client application :
```
./client
```

5. For opening multiple client applications, repeat step 4.
