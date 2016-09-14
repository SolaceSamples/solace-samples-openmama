/*
 * Copyright 2016 Solace Systems, Inc. All rights reserved.
 *
 * http://www.solacesystems.com
 *
 * This source is distributed under the terms and conditions
 * of any contract or contracts between Solace and you or
 * your company. If there are no contracts in place use of
 * this source is not authorized. No support is provided and
 * no distribution, sharing with others or re-use of this
 * source is authorized unless specifically stated in the
 * contracts referred to above.
 */

/*
 * Solace messaging with OpenMAMA
 * PublishSubscribe tutorial - Topic Subscriber
 * Demonstrates subscribing to a topic for direct messages and receiving messages
 */


#include <stdio.h>
#include <signal.h>
#include <mama/mama.h>


// used in stopHandler routine
void stopAll();

/*
 * This routine is invoked when Ctrl-C is pressed from the console
 */
void stopHandler(int value)
{
    signal(value, SIG_IGN);
    printf(" Do you want to stop the program? [y/n]: ");
    char answer = getchar();
    if (answer == 'y')
        stopAll();
    else
        signal(SIGINT, stopHandler);
    getchar();
}


// used in main routine
void initializeBridge(const char *);
void connectTransport(const char *);
void subscribeToTopic(const char *);


// global pointers
struct
{
    mamaBridge bridge;
    mamaTransport transport;
    mamaSubscription subscription;
    mamaMsgCallbacks receiver;
}
global;


int main(int argc, const char** argv)
{
    printf("Solace OpenMAMA tutorial.\nReceiving messages with OpenMAMA.\n");
    
    // set Ctrl-C handler
    signal(SIGINT, stopHandler);

    initializeBridge("solace");
    connectTransport("vmr"); // see mama.properties
    subscribeToTopic("tutorial.topic");

    // receive messages until Ctrl-C is pressed
    mama_status status;
    if ((status = mama_start(global.bridge)) != MAMA_STATUS_OK)
    {
        printf("OpenMAMA start error: %s\n",
                mamaStatus_stringForStatus(status));
    }
    // normal exit after Ctrl-C was pressed
    printf("Closing Solace middleware bridge.\n");    
    mama_close();
    exit(status);
}


/*
 * Stop subscription, transport and MAMA
 */
void stopAll()
{
    // order is important
    mama_stop(global.bridge);
    if (global.subscription)
    {
        mamaSubscription_destroy(global.subscription);
        global.subscription = NULL;
    }
    if (global.transport)
    {
        mamaTransport_destroy(global.transport);
        global.transport = NULL;
    }
}


/*
 * Load and open specified middleware bridge
 */
void initializeBridge(const char * bridgeName)
{
    global.bridge = NULL;
    mama_status status;
    if (((status = mama_loadBridge(&global.bridge, bridgeName)) == MAMA_STATUS_OK) &&
        ((status = mama_openWithProperties(".","mama.properties")) == MAMA_STATUS_OK))
    {
        // normal exit;
        return;
    }
    // error exit
    mama_close();
    printf("Middleware bridge \"%s\" initialization error: %s\n", bridgeName, mamaStatus_stringForStatus(status));
    exit(status);
}


/*
 * Connect to specified transport using the previously loaded bridge
 */
void connectTransport(const char * transportName)
{
    global.transport = NULL;
    mama_status status;
    if (((status = mamaTransport_allocate(&global.transport)) == MAMA_STATUS_OK) &&
        ((status = mamaTransport_create(global.transport, transportName, global.bridge)) == MAMA_STATUS_OK))
    {
        // normal exit
        return;
    }
    // error exit
    printf("Transport %s connect error: %s\n", transportName, mamaStatus_stringForStatus(status));
    mama_close();
    exit(status);
}


/*
 * Call this routine when subscription is created
 */
void onCreate(mamaSubscription subscription, void * closure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    {
        printf("Created subscription to topic \"%s\"\n", topicName);
    }
}


/*
 * Call this routine when subscription error occurs
 */
void onError(mamaSubscription subscription, mama_status status, 
        void * platformError, const char * subject, void * closure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    printf("Error occured with subscription to topic \"%s\", error code: %s\n",
            topicName, mamaStatus_stringForStatus(status));
}


/*
 * Call this routine when a message is received
 */
void onMessage(mamaSubscription subscription, mamaMsg message, void * closure, void * itemClosure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    {
        printf("Message of type %s received on topic \"%s\"\n", mamaMsgType_stringForMsg(message), topicName);
    }
    // extract from the message our own custom field (see topicPublisherMultiple.c)
    const char * const MY_FIELD_NAME = "MdMyTimestamp";
    const int MY_FIELD_ID = 99, BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];
    if (mamaMsg_getFieldAsString(message, MY_FIELD_NAME, MY_FIELD_ID, buffer, BUFFER_SIZE) 
            == MAMA_STATUS_OK)
    {
        printf("This message has a field \"%s\" with value: %s", MY_FIELD_NAME, buffer);
    }
}


/*
 * Subscribe to specified message topic
 */
void subscribeToTopic(const char * topicName)
{
    global.subscription = NULL;
    mama_status status;
    if ((status = mamaSubscription_allocate(&global.subscription)) == MAMA_STATUS_OK)
    {
        // initialize functions called by MAMA on different subscription events
        memset(&global.receiver, 0, sizeof(global.receiver));
        global.receiver.onCreate = onCreate; // when subscription created
        global.receiver.onError = onError; // when error occurs
        global.receiver.onMsg = onMessage; // when a message arrives on subscribed topic
        
        // subscribe to messages on specified topic
        mamaQueue defaultQueue;
        if (((status = mama_getDefaultEventQueue(global.bridge, &defaultQueue)) == MAMA_STATUS_OK) &&
            ((status = mamaSubscription_createBasic(global.subscription, global.transport,
                                                    defaultQueue,
                                                    &global.receiver,
                                                    topicName,
                                                    NULL)) == MAMA_STATUS_OK))
        {
            // normal exit
            return;
        }
    }
    // error exit
    printf("Error subscribing to topic %s, error code: %s\n", topicName,
            mamaStatus_stringForStatus(status));
    mama_close();
    exit(status);
}

