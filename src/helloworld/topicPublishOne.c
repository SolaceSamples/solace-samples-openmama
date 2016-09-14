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
 * Demonstrates publishing one direct message to a topic
 */

#include <stdio.h>
#include <mama/mama.h>

int main(int argc, const char** argv)
{
    printf("Solace OpenMAMA tutorial.\nPublishing one message with OpenMAMA.\n");

    mama_status status;
    mamaBridge bridge = NULL;
    // load Solace middleware bridge and open it
    if (((status = mama_loadBridge(&bridge, "solace")) == MAMA_STATUS_OK) &&
        ((status = mama_openWithProperties(".","mama.properties")) == MAMA_STATUS_OK))
    {
        // 1. create transport and 2. create publisher
        mamaTransport transport = NULL;
        mamaPublisher publisher = NULL;
        if (((status = mamaTransport_allocate(&transport)) == MAMA_STATUS_OK) &&
            ((status = mamaTransport_create(transport, "vmr", bridge)) == MAMA_STATUS_OK) &&
            ((status = mamaPublisher_create(&publisher, transport,
                                            "tutorial.topic", NULL, NULL)) == MAMA_STATUS_OK))
        {
            // 3. create message and add a string field to it
            mamaMsg message = NULL;
            if (((status = mamaMsg_create(&message)) == MAMA_STATUS_OK) &&
                ((status = mamaMsg_addString(message, "MyGreetingField", 99,
                                             "Hello World")) == MAMA_STATUS_OK))
            {
                // 4. send the message
                if ((status = mamaPublisher_send(publisher, message)) == MAMA_STATUS_OK)
                {
                    // notice that "destroy" calls are in the opposite order of "create" calls
                    mamaMsg_destroy(message);
                    printf("Message published, closing Solace middleware bridge.\n");
                    mamaPublisher_destroy(publisher);
                    mamaTransport_destroy(transport);
                    mama_close();
                    // normal exit
                    exit(0);
                }
            }
        }
    }
    printf("OpenMAMA error: %s\n", mamaStatus_stringForStatus(status));
    exit(status);
}



