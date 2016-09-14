##scan devices

##scan service

##setup notify on UUID_SFIDA_COMMANDS_CHAR

##I write 3 0 0 0 (SFIDA_RESPONSE_CERTIFICATION_NOTIFY) to SFIDA_TO_CENTRAL_CHAR and UUID_SFIDA_COMMANDS_CHAR

##App writes 04 00 00 00 to CENTRAL_TO_SFIDA_CHAR

##I write 4 0 1 0 (SFIDA_RESPONSE_CERTIFICATION_CHALLENGE_1) to SFIDA_TO_CENTRAL_CHAR and UUID_SFIDA_COMMANDS_CHAR

##App writes 05 00 00 00 + 32byte random data to CENTRAL_TO_SFIDA_CHAR

##I write 5 0 0 0 (SFIDA_RESPONSE_CERTIFICATION_CHALLENGE_2) to SFIDA_TO_CENTRAL_CHAR and UUID_SFIDA_COMMANDS_CHAR

##App writes 03 00 00 00 03 to CENTRAL_TO_SFIDA_CHAR

##I write 4 0 2 0 (SFIDA_RESPONSE_CERTIFICATION_NOTIFY) to SFIDA_TO_CENTRAL_CHAR and UUID_SFIDA_COMMANDS_CHAR

##App disconnects immediately

Maybe some MITM attack may solve the problem.

Seems I only have around 10 seconds for all security data exchange, otherwise it will timeout. 

