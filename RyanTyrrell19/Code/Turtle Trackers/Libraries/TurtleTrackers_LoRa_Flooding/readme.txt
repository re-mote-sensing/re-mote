OVERVIEW:
The Flooding Mesh Network works as follows:
The End-Node boradcast its message, and all relayer's within range rebroadcast the message. Eventually, the message will be received by the Gateway.
The Gatetway then sends an ACK back. Precations are taken to avoid broadcast storms and discard stale messages.


ISSUES:
Message may be missed if Node/Relay/Gateway is currently processing another message
This issue would exists with any Mesh Network, so how does the RadioHead Mesh Network Protocol fix it?
I think maybe if I just have certain nodes delay equal to the process + send time, this should minimize any conflict issues

NOTES:
All messages sent and received by this RH_RF95 Driver conform to this packet format:

LoRa mode:
8 symbol PREAMBLE
Explicit header with header CRC (handled internally by the radio)
4 octets HEADER: (TO, FROM, ID, FLAGS)
0 to 251 octets DATA
CRC (handled internally by the radio)

Message Format - [ACK]|[Lifespan]|[Message ID]|[Message Data]
ACK - 0 or 1 ( 0 = not an ACK, 1 = is an ACK)
Lifespan - Exceeding a threshold will result in the message being terminated
Message ID - Uniquly determined using the Node ID, # of nodes in the network, and the # of messages the node has sent
Message Data - Reason for the message tranmssion. Thsi data could be GPS coordiantes, commands, etc.

Any global pointer arrays I have need to have the memory allocated before hand to ensure defined behaviour occurs. With memory already allocated, I can safely store the actual value
(not the address) without it disappearing. (since the location the pointer points to was defined earlier and will not disappear)

Using pointers in the Loop(). Will that mean every run of the loop will delete where they point to (since they may point to the memory of a local varaible)?
What happens is, every iteration of the loop makes a new pointer that points to a local varaible. However, the local varaibles address is always the same. So all pointers
point to the same location. Calling the value the pointers point to will print the current value of the local variable