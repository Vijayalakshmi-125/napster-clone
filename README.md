# napster clone

### CONTEXT:

This code implements a simple 'Napster' like P2P system for file search and retrieval. A user (peer) joins a P2P system by contacting a central server (server). The peer publishes the files she wishes to share by specifying the same to the central server. The peer searches for a file, again by contacting the central server. Based on input from the server, peer fetches the file from another peer (who has the file).

### CODE FUNCTIONALITY:

There is a single central server (server.cpp) and many peers. Peers are different instances of the same code peer.cpp. The server handles multiple peers simultaneously. All peers know the central server details, via a command-line argument ./peer server-ip server-port.

The peer has to implement three functions: join, publish and fetch. Search functionality is subsumed within fetch.

1. Join: This should happen automatically when the peer code is run via “./peer server-ip server-port”. The peer should inform the central server of its existence.

2. Publish: The peer should have an appropriate interface (stdin, or GUI) to take necessary arguments to achieve the publish functionality. For example, you should be able to specify the name of the file and the path where the file is. The peer then will use this information to contact the server as well as do any needed local housekeeping.

3. Fetch: The peer should have an appropriate interface (stdin, or GUI) to take necessary arguments to achieve the fetch functionality as well. For example, you should be able to specify the name of the file you want and the path where the file is to be stored. In fact the peer should ask if it is a fetch or publish option you desire and act accordingly.

If it is a fetch operation, the peer should first contact the server to determine which peer has the file (search) and then initiate a connection to that peer to get the file. If there are more than one peer who have the file, the selection can be random. If there is no file available, it should print the same.
