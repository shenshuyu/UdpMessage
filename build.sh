echo "mkdir bin"
mkdir -p bin

echo "build server"
#server 
g++ udpserver.cpp -lpthread -std=c++11 -o bin/server


echo "build clien"
#client
g++ udpclient.cpp -lpthread -std=c++11 -o bin/client

