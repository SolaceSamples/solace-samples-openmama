---
layout: tutorials
title: Publish/Subscribe
summary: Learn how to pub/sub using OpenMAMA with the Solace middleware bridge.
icon: I_dev_P+S.svg
links:
    - label: mama.properties
      link: /blob/master/src/pubsub/mama.properties
    - label: topicSubscriber.c
      link: blob/master/src/pubsub/topicSubscriber.c
    - label: topicPublisher.c
      link: /blob/master/src/pubsub/topicPublisher.c
---
<br><br>

## Assumptions

This tutorial assumes the following:

*   You are familiar with OpenMAMA [core concepts]({{ site.docs-openmama-concepts }}){:target="_top"}.
    *   If not, see [this OpenMAMA guide](http://www.openmama.org/content/quick-start-guide){:target="_blank"} and [Solace OpenMAMA “Hello World” tutorial]({{ site.baseurl }}/hello-world).
*   You are familiar with Solace [core concepts]({{ site.docs-solace-concepts }}){:target="_top"}.
*   You have access to a properly installed OpenMAMA [release](https://github.com/OpenMAMA/OpenMAMA/releases){:target="_blank"}.
    *   Solace middleware bridge with its dependencies is also installed
*   You have access to Solace messaging with the following configuration details:
    *   Connectivity information for a Solace message-VPN
    *   Enabled client username and password
    
{% if jekyll.environment == 'solaceCloud' %}
One simple way to get access to Solace messaging quickly is to create a messaging service in Solace Cloud [as outlined here]({{ site.links-solaceCloud-setup}}){:target="_top"}. You can find other ways to get access to Solace messaging below.
{% else %}
One simple way to get access to a Solace message router is to start a Solace VMR load [as outlined here]({{ site.docs-vmr-setup }}){:target="_top"}. By default the Solace VMR will with the “default” message VPN configured and ready for guaranteed messaging. Going forward, this tutorial assumes that you are using the Solace VMR. If you are using a different Solace message router configuration adapt the tutorial appropriately to match your configuration.
{% endif %}

## Goals

The goal of this tutorial is to demonstrate basic messaging interactions using OpenMAMA with the **Solace middleware bridge**. This tutorial’s samples are similar to the OpenMAMA’s [mamapublisherc.c](https://github.com/OpenMAMA/OpenMAMA/blob/master/mama/c_cpp/src/examples/c/mamapublisherc.c){:target="_blank"} and [mamasubscriberc.c](https://github.com/OpenMAMA/OpenMAMA/blob/master/mama/c_cpp/src/examples/c/mamasubscriberc.c){:target="_blank"}, but with a distinct focus on configuring OpenMAMA with **Solace messaging**. See the [Resources](#resources) section below for some further links to other OpenMAMA tutorials and examples.

This tutorial will show you how to publish a message with one string field to a specific topic on Solace messaging using OpenMAMA C API.

This tutorial will show you how to use OpenMAMA C API:

*   to build and send a message on a topic
*   to subscribe to a topic and receive a message

## Installation

Installation instructions for OpenMAMA can be found on [OpenMAMA Wiki](http://www.openmama.org/content/quick-start-guide#main){:target="_blank"}.

Simplified installation instructions for OpenMAMA with Solace middleware bridge [are available]({{ site.baseurl }}/installation-linux).

For building OpenMAMA from source see [OpenMAMA Wiki](https://github.com/OpenMAMA/OpenMAMA/wiki/Build-Instructions){:target="_blank"}.

{% if jekyll.environment == 'solaceCloud' %}
  {% include solaceMessaging-cloud.md %}
{% else %}
    {% include solaceMessaging.md %}
{% endif %}  

Example of specifying these properties [see here]({{ site.repository }}/blob/master/src/pubsub/mama.properties){:target="_blank"} and detailed explanation of them is in the [Solace OpenMAMA “Hello World” tutorial]({{ site.baseurl }}/hello-world).

## Receiving Message

Before everything else, as you already know from the [Solace OpenMAMA “Hello World” tutorial]({{ site.baseurl }}/hello-world), we need to [initialize]({{ site.baseurl }}/hello-world/#initialize) the **Solace middleware bridge** and [create a transport]({{ site.baseurl }}/hello-world/#create-transport).

To learn more about how to initialize and configure **Solace middleware bridge as an OpenMAMA transport** see [Solace OpenMAMA User Guide]({{ site.docs-openmama-bridges }}){:target="_top"}

Now we can implement receiving a message.

First, let’s express interest in messages by subscribing to a topic. Then you can look at publishing a matching message and see it received.

In OpenMAMA subscribing to a topic means _creating a subscription_. When creating a subscription, we need to refer to a _transport_, a _receiver_ and to an _event queue_.

You already know what the **transport** is.

For details of the **event queue** see [OpenMAMA Developer’s Guide for C](http://www.openmama.org/sites/default/files/OpenMAMA%20Developer%27s%20Guide%20C.pdf){:target="_blank"}

When multi-threading is not required it is recommended to use the _default internal event queue_ as an _event queue_:

```cpp
mamaQueue defaultQueue;
mama_getDefaultEventQueue(global.bridge, &defaultQueue))
```

The **receiver** is a data structure with function pointers that are invoked by OpenMAMA when certain events happen.

It has a type of `mamaMsgCallbacks` and it is expected to have, as a minimum, the following function pointers:

*   `onCreate` that is invoked when a subscription is created
*   `onError` that is invoked when an error occurs
*   `onMsg` that is invoked when a message arrives on the subscribed topic

This is how the routine for creating a subscription is implemented:

```cpp
void subscribeToTopic(const char * topicName)
{
    global.subscription = NULL;
    mama_status status;
    if ((status = mamaSubscription_allocate(&global.subscription)) == MAMA_STATUS_OK)
    {
        // initialize functions called by MAMA on different subscription events
        memset(&global.receiver, 0, sizeof(global.receiver));
        global.receiver.onCreate = onCreate;
        global.receiver.onError = onError;
        global.receiver.onMsg = onMessage;

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
```

Notice that `global.receiver` is assigned with pointers to functions we want to be invoked by OpenMAMA when corresponding events happen.

When a subscription is created, we want to see its topic name on the console:

```cpp
void onCreate(mamaSubscription subscription, void * closure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    {
        printf("Created subscription to topic \"%s\"\n", topicName);
    }
}
```

When an error occurs, we want to see on the console the error code:

```cpp
void onError(mamaSubscription subscription, mama_status status,
        void * platformError, const char * subject, void * closure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    printf("Error occured with subscription to topic \"%s\", error code: %s\n",
            topicName, mamaStatus_stringForStatus(status));
}
```

When a message arrives, we extract from the message the topic name and a custom string field we know was added to that message:

```cpp
void onMessage(mamaSubscription subscription, mamaMsg message, void * closure, void * itemClosure)
{
    const char * topicName = NULL;
    if (mamaSubscription_getSymbol(subscription, &topicName) == MAMA_STATUS_OK)
    {
        printf("Message of type %s received on topic \"%s\"\n", mamaMsgType_stringForMsg(message), topicName);
    }
    // extract from the message our own custom field
    const char * const MY_FIELD_NAME = "MdMyTimestamp";
    const int MY_FIELD_ID = 99, BUFFER_SIZE = 32;
    char buffer[BUFFER_SIZE];
    if (mamaMsg_getFieldAsString(message, MY_FIELD_NAME, MY_FIELD_ID, buffer, BUFFER_SIZE)
            == MAMA_STATUS_OK)
    {
        printf("This message has a field \"%s\" with value: %s", MY_FIELD_NAME, buffer);
    }
}
```

Now we can start receiving messages:

```cpp
mama_start(global.bridge);
```

This `mama_start` call blocks execution of the current thread until `mama_stop` is called, that is why we need to implement a handler for stopping our program by pressing `Ctrl-C` from console.

```cpp
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
```

The `stopAll` routine has `destroy` calls for the created transport and subscription.

Order of calls in that routine is very important and the very first one must be `mama_stop`:

```cpp
void stopAll()
{
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
```

## Sending Message

As usual, before everything else, we need to [initialize]({{ site.baseurl }}/hello-world/#initialize) the **Solace middleware bridge** and [create a transport]({{ site.baseurl }}/hello-world/#create-transport).

Now we can implement sending a message, and as you already know from the [Solace OpenMAMA “Hello World” tutorial]({{ site.baseurl }}/hello-world/#create-publisher), to publish a message we need to create a _publisher_.

A _publisher_ is created for a specific topic and refers to already created _transport_:

```c
global.publisher = NULL;
mamaPublisher_create(&global.publisher, global.transport, publishTopicName, NULL, NULL)
```

We want our program to publish messages periodically, until we stop it (by pressing `Ctrl-C` from console).

OpenMAMA provides a very convenient mechanism for a periodic event in a forms of _mamaTimer_. Such timers created with a reference to an _event queue_ and we can use the _default internal event queue_ for it:

```cpp
global.publishTimer = NULL;
mamaTimer_create(&global.publishTimer, defaultQueue, timerCallback, intervalSeconds, NULL);
```

The `timerCallback` parameter is a pointer to a function that will be invoked by OpenMAMA when the timer expires.

We’re going to implement this function to send one message with three different fields, one of them is a custom field with the message timestamp as a string:

```cpp
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
```

Now we can start sending messages:

```cpp
mama_start(global.bridge);
```

Because `mama_start` call blocks execution of the current thread until `mama_stop` is called, we’re going to use the handler mechanism for stopping our program by pressing `Ctrl-C` from console.

That handler calls `stopAll`. As you already know, order of calls in `stopAll` routine is very important and the very first one must be `mama_stop`:

```cpp
void stopAll()
{
    mama_stop(global.bridge);
    // order is important
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
```

## Summarizing

Our application consists of two executables: _topicSubscriber_ and _topicPublisher_.

If you combine the example source code shown above and split them into the two mentioned executables, it results in the source that is available for download:

<ul>
{% for item in page.links %}
<li><a href="{{ site.repository }}{{ item.link }}" target="_blank">{{ item.label }}</a></li>
{% endfor %}
</ul>

### Building

To build on **Linux**, assuming OpenMAMA installed into `/opt/openmama`:

```
$ gcc -o topicSubscriber topicSubscriber.c -I/opt/openmama/include -L/opt/openmama/lib -lmama
$ gcc -o topicPublisher topicPublisher.c -I/opt/openmama/include -L/opt/openmama/lib -lmama
```

### Running the Application

First, start the _topicSubscriber_:

```
$ ./topicSubscriber
Solace OpenMAMA tutorial.
Receiving messages with OpenMAMA.
2016-07-14 14:22:38:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-14 14:22:38: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Created subscription to topic "tutorial.topic"
```

Now in a separate console start the _topicPublisher_ that begins publishing messages periodically:

```
$ ./topicPublisher
Solace OpenMAMA tutorial.
Publishing messages with OpenMAMA.
2016-07-14 14:22:48:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-14 14:22:48: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Message published: Thu Jul 14 14:22:51 2016
Message published: Thu Jul 14 14:22:54 2016
Message published: Thu Jul 14 14:22:57 2016
Message published: Thu Jul 14 14:23:00 2016
```

You will see the _topicSubscriber_ receiving messages and logging them to the connsole:

```
$ ./topicSubscriber
Solace OpenMAMA tutorial.
Receiving messages with OpenMAMA.
2016-07-14 14:22:38:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-14 14:22:38: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Created subscription to topic "tutorial.topic"
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:51 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:54 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:57 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:23:00 2016
```

To stop the application press `Ctrl-C` and then `y` on each console. First, stop the publisher:

```
$ ./topicPublisher
Solace OpenMAMA tutorial.
Publishing messages with OpenMAMA.
2016-07-14 14:22:48: Failed to open properties file.

2016-07-14 14:22:48:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-14 14:22:48: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Message published: Thu Jul 14 14:22:51 2016
Message published: Thu Jul 14 14:22:54 2016
Message published: Thu Jul 14 14:22:57 2016
Message published: Thu Jul 14 14:23:00 2016
^C Do you want to stop the program? [y/n]: y
Closing Solace middleware bridge.
```

Now we can stop the subscriber:

```
$ ./topicSubscriber
Solace OpenMAMA tutorial.
Receiving messages with OpenMAMA.
2016-07-14 14:22:38:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-14 14:22:38: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Created subscription to topic "tutorial.topic"
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:51 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:54 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:22:57 2016
Message of type INITIAL received on topic "tutorial.topic"
This message has a field "MdMyTimestamp" with value: Thu Jul 14 14:23:00 2016
^C Do you want to stop the program? [y/n]: y
Closing Solace middleware bridge.
```

Congratulations! You have now successfully subscribed to a topic and exchanged messages using this topic on using OpenMAMA with the Solace middleware bridge.

If you have any issues with this program, check the [Solace community]({{ site.link-community }}){:target="_blank"} for answers to common issues.

## Resources

For more information about OpenMAMA:

*   The OpenMAMA website at: [http://www.openmama.org/](http://www.openmama.org/){:target="_blank"}.
*   The OpenMAMA code repository on GitHub [https://github.com/OpenMAMA/OpenMAMA](https://github.com/OpenMAMA/OpenMAMA){:target="_blank"}.
*   Chat with OpenMAMA developers and users at [Gitter OpenMAMA room](https://gitter.im/OpenMAMA/OpenMAMA){:target="_blank"}.

For more information about Solace technology:

*   The Solace Developer Portal website at: [{{ site.link-portal }}]({{ site.link-portal}}){:target="_top"}
*   Get a better understanding of [Solace technology]({{ site.link-tech }}){:target="_top"}.
*   Ask the [Solace community]({{ site.link-community }}){:target="_top"}.

Other tutorials and related links:

*   [OpenMAMA Quick Start Guide](http://www.openmama.org/content/quick-start-guide){:target="_blank"}
*   [OpenMAMA Wiki Quick Start Guide](https://github.com/OpenMAMA/OpenMAMA/wiki/Quick-Start-Guide){:target="_blank"}
*   [OpenMAMA Example Walk Through](http://www.openmama.org/example-walk-through){:target="_blank"}.
*   [OpenMAMA Code Examples](https://github.com/OpenMAMA/OpenMAMA/tree/master/mama/c_cpp/src/examples){:target="_blank"}
*   [OpenMAMA Wiki](https://github.com/OpenMAMA/OpenMAMA/wiki){:target="_blank"}
*   [OpenMAMA Documentation and Developers Guides](http://www.openmama.org/documentation){:target="_blank"}
*   [Solace’s Solution for OpenMAMA]({{ site.link-tech-openmama }}){:target="_top"}
