# Planner:

1) GPS Powers on
2) LoRa Module sends empty message. If no ACK recieved, GPS is underwater (or Gateway is broken/out of range). Either way, power off GPS

3) Read from reciever
    * Time since last fix