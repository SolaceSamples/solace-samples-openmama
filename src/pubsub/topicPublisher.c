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
 * PublishSubscribe tutorial - Topic Publisher
 * Demonstrates publishing multiple direct messages to a topic
 */


#include <stdio.h>
#include <signal.h>
#include <mama/mama.h>

#include <time.h> // for message timestamp


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


// used in main routune
void initializeBridge(const char *);
void connectTransport(const char *);
void configurePublishing(const char *, double);


// global pointers
struct
{
    mamaBridge bridge;
    mamaTransport transport;
    mamaPublisher publisher;
    mamaTimer publishTimer;
}
global;


int main(int argc, const char** argv)
{
    printf("Solace OpenMAMA tutorial.\nPublishing messages with OpenMAMA.\n");
    
    // set Ctrl-C handler
    signal(SIGINT, stopHandler);

    initializeBridge("solace");
    connectTransport("vmr"); // see mama.properties
    // publish one message to "tutorial/topic" every 3 seconds
    configurePublishing("tutorial.topic", 3);

    // start publishing until Ctrl-C is pressed
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
 * Stop publisher, transport and MAMA
 */
void stopAll()
{
    // order is important
    mama_stop(global.bridge);
    if (global.publishTimer)
    {
        mamaTimer_destroy(global.publishTimer);
        global.publishTimer = NULL;
    }
    if (global.publisher)
    {
        mamaPublisher_destroy(global.publisher);
        global.publisher = NULL;
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


// this callback will be called when timer expires
void timerCallback(mamaTimer, void*);

/*
 * Configure publishing of messages to the specified topic with specified interval
 */
void configurePublishing(const char * publishTopicName, double intervalSeconds)
{
    global.publisher = NULL;
    mama_status status;
    if ((status = mamaPublisher_create(&global.publisher, global.transport,
                    publishTopicName, NULL, NULL)) == MAMA_STATUS_OK)
    {
        // create a timer for publishing of messages
        global.publishTimer = NULL;
        mamaQueue defaultQueue;
        if (((status = mama_getDefaultEventQueue(global.bridge, &defaultQueue)) == MAMA_STATUS_OK) &&
            ((status = mamaTimer_create(&global.publishTimer, defaultQueue,
                                        timerCallback,
                                        intervalSeconds, 
                                        NULL)) == MAMA_STATUS_OK))
        {
            // normal exit
            return;
        }
    }
    // error exit
    printf("Error configuring publisher for topic %s, error code: %s\n", publishTopicName,
            mamaStatus_stringForStatus(status));
    mama_close();
    exit(status);
}


/*
 * This routine is called when timer expires, it creates a message and publishes it
 */
void timerCallback(mamaTimer timer, void* closure)
{
    // generate a timestamp as one of the message fields
    char buffer[32];
    time_t currTime = time(NULL);
    sprintf(buffer, "%s", asctime(localtime(&currTime)));
    // create message and add three fields to it
    mamaMsg message = NULL;
    mama_status status;
    if (((status = mamaMsg_create(&message)) == MAMA_STATUS_OK) &&
        ((status = mamaMsg_addI32(message, MamaFieldMsgType.mName, MamaFieldMsgType.mFid,
                                  MAMA_MSG_TYPE_INITIAL)) == MAMA_STATUS_OK) &&
        ((status = mamaMsg_addI32(message, MamaFieldMsgStatus.mName, MamaFieldMsgStatus.mFid,
                                  MAMA_MSG_STATUS_OK)) == MAMA_STATUS_OK) &&
        ((status = mamaMsg_addString(message, "MdMyTimestamp", 99,
                                     buffer)) == MAMA_STATUS_OK))
    {
        if ((status = mamaPublisher_send(global.publisher, message)) == MAMA_STATUS_OK)
        {
            printf("Message published: %s", buffer);
            mamaMsg_destroy(message);
            // normal exit
            return;
        }
    }
    // error exit
    printf("Error publishing message: %s\n", mamaStatus_stringForStatus(status));
}

