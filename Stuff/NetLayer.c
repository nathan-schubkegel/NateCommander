#include "NetLayer.h"

#include "SDL.h"
#include "Utils.h"
#include "StringEater.h"
#include <string.h>

// Yay C object-oriented programming.

struct NetAddress
{
  Uint32 address;
  Uint16 bindToAllInterfaces; // 1 or 0
  Uint16 port;
};

// for IPv4:
//    just bind to 1 interface: "192.168.0.1:1337"
//    bind to all interfaces: "*:1337";
// this object makes a clone of the passed-in data
NetAddress * NetAddress_Create(const char * addressInfo)
{
  NetAddress * address;
  size_t maxSize;
  size_t currentPos;
  
  address = MallocAndInitOrDie(sizeof(NetAddress));
  maxSize = strlen(addressInfo);

  StringEater_Init(addressInfo, maxSize, &currentPos);
  StringEater_(addressInfo, maxSize, &currentPos);
}

void NetAddress_Destroy(NetAddress ** obj);
// for IPv4 this is 192.168.0.1 port 1337
// for IPv6 this is something else
// for steam this is some special data, or maybe nothing, who knows
// I guess it's opaque, it just needs to answer some questions like
// which one are you among various others known on this system?
int NetAddress_GetLocalIdentifier(NetAddress * obj);


// is either "read" or "write", not both
// is datagram-oriented, max 1500, hopefully sensitive to the network and self-adjusting
// (simple model is "just do 1400")
struct NetStream;
typedef struct NetStream NetStream;
// try to queue bytes to be sent / dequeue acquired bytes
// caller should query NetStream_GetMaxBytesPerDatagram for max buffer size
// returns number of bytes queued / dequeued
int NetStream_TryEnqueueDatagram(NetStream * obj, byte * buffer, int numBytes);
int NetStream_TryDequeueDatagram(NetStream * obj, byte * buffer, int maxLength, int * outOfSequenceDetected);
// 0 means "no limit", this will be 1400 by default
int NetStream_GetMaxBytesPerDatagram(NetStream * obj);
// for "sending", the application would probably delay queuing another packet
// if these are greater than zero
// for "reading", this probably isn't very valuable
int NetStream_GetNumQueuedBytes(NetStream * obj);
int NetStream_GetNumQueuedDatagrams(NetStream * obj);
// indicates internal buffer sizes
int NetStream_GetMaxBytes(NetStream * obj); // 0 means "no limit"
int NetStream_GetMaxDatagrams(NetStream * obj); // 0 means "no limit"


// This is "whatever it takes to send/receive data between two programs"
struct NetConnection;
typedef struct NetConnection NetConnection;
NetAddress * NetConnection_LocalAddress(NetConnection * data);
NetAddress * NetConnection_RemoteAddress(NetConnection * data);
NetStream * NetConnection_GetReadStream(NetConnection * data);
NetStream * NetConnection_GetWriteStream(NetConnection * data);
int NetConnection_GetIsAlive(NetConnection * data); // implementation includes application-level heartbeats and timeouts
int NetConnection_GetIsEstablished(NetConnection * data); // for all it means "have we received data from the other side yet?"
int NetConnection_GetHeartbeatRate(NetConnection * data); // 1 seconds - a blag packet every once in a while
int NetConnection_GetTimeoutRate(NetConnection * data); // 5 seconds - no communication in this time, drop the client
void NetConnection_Disconnect(NetConnection * data);


// This is "whatever it takes to accept net clients, as a server"
// it memory-owns all the data accessed through it
struct NetServer;
typedef struct NetServer NetServer;
// memory-ownership of the NetAddress passes to this object
NetServer * NetServer_Create(NetAddress * localListeningAddress);
void NetServer_Destroy(NetServer ** obj);
int NetServer_GetMaxConnections(NetServer * obj); // for now return 32
int NetServer_GetNumConnections(NetServer * obj);
NetConnection * NetServer_GetClientConnectionById(NetServer * obj, int id); // id never changes per connection, but index can change as folks connect & drop
int NetServer_GetIdForIndex(NetServer * obj, int index);
int NetServer_GetIndexForId(NetServer * obj, int id);
// The application must ask periodically about new/dropped connections
// This object stores history of joined/dropped connections (some internal hard buffer)
int NetServer_AcceptNewConnectionId(NetServer * obj); // -1 means "nobody joined"
int NetServer_AcceptDroppedConnectionId(NetServer * obj); // -1 means "nobody dropped"
// FUTURE: blocked addresses?


// this memory-owns all the data accessed through it
struct NetClient;
typedef struct NetClient NetClient;
// memory-ownership of the NetAddresses passes to this object
NetClient * NetClient_Create(NetAddress * localAddress, NetAddress * remoteAddress);
void NetClient_Destroy(NetClient ** obj);
NetConnection * NetClient_GetConnection(NetClient * obj);


#endif