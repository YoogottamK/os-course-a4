#ifndef __SERVERS_H
#define __SERVERS_H

typedef struct Server {
    int uid;
} Server;

/*
 * This function will accept the payment for
 *  a single rider. The payment is supposed
 *  to take 2 seconds
 */
void * acceptPayment(void * s);

/*
 * This function sets up the payment servers,
 *  initializes threads, etc
 */
void initPaymentServer(int uid);

#endif // __SERVERS_H
