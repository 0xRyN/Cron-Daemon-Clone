#ifndef CLIENT_REQUEST_H
#define CLIENT_REQUEST_H

#define CLIENT_REQUEST_LIST_TASKS 0x4c53              // 'LS'
#define CLIENT_REQUEST_CREATE_TASK 0x4352             // 'CR'
#define CLIENT_REQUEST_REMOVE_TASK 0x524d             // 'RM'
#define CLIENT_REQUEST_GET_TIMES_AND_EXITCODES 0x5458 // 'TX'
#define CLIENT_REQUEST_TERMINATE 0x544d               // 'TM'
#define CLIENT_REQUEST_GET_STDOUT 0x534f              // 'SO'
#define CLIENT_REQUEST_GET_STDERR 0x5345              // 'SE'

#endif // CLIENT_REQUEST_H
