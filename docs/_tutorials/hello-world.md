---
layout: tutorials
title: Solace Hello World
summary: This tutorial demonstrates the most basic messaging interaction using OpenMAMA with the Solace middleware bridge. 
icon: hello-world.png
---

## Assumptions

This tutorial assumes the following:

*   You are familiar with OpenMAMA [core concepts](https://sftp.solacesystems.com/Portal_Docs/OpenMAMA_User_Guide/01_Introduction.html){:target="_top"}.
    *   If not, see [this guide](http://www.openmama.org/content/quick-start-guide){:target="_blank"}.
*   You are familiar with Solace [core concepts](http://dev.solacesystems.com/docs/core-concepts/){:target="_top"}.
*   You have access to a properly installed OpenMAMA [release](https://github.com/OpenMAMA/OpenMAMA/releases){:target="_blank"}.
    *   Solace middleware bridge with its dependencies is also installed
*   You have access to a running Solace message router with the following configuration:
    *   Enabled Message VPN
    *   Enabled Client Username


## Goals

The goal of this tutorial is to demonstrate the most basic messaging interaction using OpenMAMA with the **Solace middleware bridge**. This tutorial is similar to the [OpenMAMA Quick Start Guide](http://www.openmama.org/content/quick-start-guide){:target="_blank"} and the [OpenMAMA Example Walk Through](http://www.openmama.org/example-walk-through){:target="_blank"}, but with a distinct focus on configuring OpenMAMA with **Solace message routers**. See the [Resources](#resources) section below for some further links to other OpenMAMA tutorials and examples.

This tutorial will show you how to publish a message with one string field to a specific topic on a Solace message router using OpenMAMA C API.

## Installation

Installation instructions for OpenMAMA can be found on [OpenMAMA Wiki](http://www.openmama.org/content/quick-start-guide#main){:target="_blank"}.

Simplified installation instructions for OpenMAMA with Solace middleware bridge [are available here]({{ site.baseurl }}/installation-linux).

For building OpenMAMA from source see [OpenMAMA Wiki](https://github.com/OpenMAMA/OpenMAMA/wiki/Build-Instructions){:target="_blank"}.

## Getting Access to Solace Message Router

There are two ways you can get hold of the **Solace message router**:

*   If your company has Solace message routers deployed, contact your middleware team to obtain the host name or IP address of a Solace message router to test against, a username and password to access it, and a VPN in which you can produce and consume messages.
*   If you do not have access to a Solace message router, you will need to use the [Solace Virtual Message Router](http://dev.solacesystems.com/tech/virtual-message-router/){:target="_top"}. Go through the [Set up a VMR](http://dev.solacesystems.com/get-started/vmr-setup-tutorials/setting-up-solace-vmr/){:target="_top"} tutorial to download and install it. By default the Solace VMR will run with the `“default”` message VPN configured and ready for messaging.

Going forward, this tutorial assumes that you are using the Solace VMR. If you are using a different Solace message router configuration, adapt the instructions to match your configuration.


## Solace Message Router Properties

In order to send or receive messages to a Solace message router, you need to know a few details of how to connect to the Solace message router. Specifically you need to know the following:

<table>
<thead>
<tr>
<th>Resource</th>
<th>Value</th>
<th>Description</th>
</tr>
</thead>

<tbody>
<tr>
<td>Host</td>
<td>String of the form `DNS name` or `IP:Port`</td>
<td>This is the address clients use when connecting to the Solace message router to send and receive messages. For a Solace VMR this there is only a single interface so the IP is the same as the management IP address. For Solace message router appliances this is the host address of the message-backbone.</td>
</tr>
<tr>
<td>Message VPN</td>
<td>String</td>
<td>The Solace message router Message VPN that this client should connect to. The simplest option is to use the `default` message-vpn which is present on all Solace message routers and fully enabled for message traffic on Solace VMRs.</td>
</tr>
<tr>
<td>Client Username</td>
<td>String</td>
<td>The client username. For the Solace VMR default message VPN, authentication is disabled by default, so this can be any value.</td>
</tr>
<tr>
<td>Client Password</td>
<td>String</td>
<td>The optional client password. For the Solace VMR default message VPN, authentication is disabled by default, so this can be any value or omitted.</td>
</tr>
</tbody>
</table>

Example of specifying these properties [see here]({{ site.repository }}/blob/master/src/helloworld/mama.properties){:target="_blank"} and detailed explanation of them is [below](#create-transport).

## Hello World

In our first program we’re going to publish one _“Hello World”_ message to a specific topic on a Solace message router using OpenMAMA with the Solace middleware bridge.

The program will consist of two major parts:

1.  [Initialize](#initialize) Solace middleware bridge
2.  [Publish Message](#publish-message)

### Initialize

Any OpenMAMA program begins with initialization that consists of loading a bridge and opening it, in this particular order:

*   load
*   open

This is how it is done.

Begin by declaring the bridge pointer:

```c
    mamaBridge bridge = NULL;
```

Then we need to load the bridge, referring to it by its name (**“solace”**), and open it by calling `mama_open()`:

```c
    mama_loadBridge(&bridge, "solace");
    mama_open();
```

Opening of the bridge must have a corresponding closing `mama_close()` call:

```c
    mama_close();
```

This is already a program that can be compiled and executed, let’s add to it some console messages that would help us to watch it running, and some rudimentary error handling.

```c
#include <stdio.h>
#include <mama/mama.h>

int main(int argc, const char** argv)
{
    printf("Solace OpenMAMA tutorial.\nPublishing one message with OpenMAMA.\n");

    mama_status status;
    mamaBridge bridge = NULL;
    // load Solace middleware bridge and open it
    if (((status = mama_loadBridge(&bridge, "solace")) == MAMA_STATUS_OK) &&
        ((status = mama_open()) == MAMA_STATUS_OK))
    {
        printf("Closing Solace middleware bridge.\n");
        mama_close();
        // normal exit
        exit(0);
    }
    printf("OpenMAMA error: %s\n", mamaStatus_stringForStatus(status));
    exit(status);
}
```

At this point our program needs to be linked with `libmama` (`libmamac` or `libmamacmdd`on Windows) and it requires OpenMAMA headers to compile.

On **Linux**, assuming OpenMAMA installed into `/opt/openmama`:

    $ gcc -o topicPublishOne topicPublishOne.c -I/opt/openmama/include -L/opt/openmama/lib -lmama

On **Windows**, assuming OpenMAMA is at `<openmama>` directory:

    $ cl topicPublishOne.c /I<openmama>\mama\c_cpp\src\c /I<openmama>\common\c_cpp\src\c\windows -I<openmama>\common\c_cpp\src\c <openmama>\Debug\libmamacmdd.lib

When we run this program, it seems like nothing happened:

```
$ ./topicPublishOne
Solace OpenMAMA tutorial.
Publishing one message with OpenMAMA.
2016-07-12 13:58:27: Failed to open properties file.
2016-07-12 13:58:27:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
Closing Solace middleware bridge.
```

But in fact OpenMAMA has successfully initialized with the **Solace middleware bridge** and is ready to publish a message.

### Publish Message

You have definitely noticed the `“Failed to open properties file”` log message (if it appears twice, ignore one of them, it is a well known OpenMAMA [pickle](https://github.com/OpenMAMA/OpenMAMA/issues/37)){:target="_blank"}.

It means that MAMA at run-time is looking for the **properties file** and we will create that file a bit later.

At this point we can successfully load the Solace middleware bridge and open it, which means a successful initialization of OpenMAMA. Now we can publish a message.

In order to publish a message we need to do the following **in this particular order**:

1.  [Create a transport](#create-transport) for the Solace middleware bridge
2.  [Create a publisher](#create-publisher) that uses this transport
3.  [Create a message](#create-message)
4.  Use the publisher to [send the message](#send-message)

#### Create transport

Creating of transport includes two steps in this particular order:

*   allocate
*   create

```c
mamaTransport transport = NULL;
mamaTransport_allocate(&transport);
mamaTransport_create(transport, "vmr", bridge);
```

Notice the name of the transport: `“vmr”`. This is the alias we will refer to from the **properties file**.

Don’t forget to destroy the transport before closing the bridge:

```c
mamaTransport_destroy(transport);
```

This is how our program looks now, let’s compile and run it.

```c
#include <stdio.h>
#include <mama/mama.h>

int main(int argc, const char** argv)
{
    printf("Solace OpenMAMA tutorial.\nPublishing one message with OpenMAMA.\n");

    mama_status status;
    mamaBridge bridge = NULL;
    // load Solace middleware bridge and open it
    if (((status = mama_loadBridge(&bridge, "solace")) == MAMA_STATUS_OK) &&
        ((status = mama_open()) == MAMA_STATUS_OK))
    {
        // create transport
        mamaTransport transport = NULL;
        if (((status = mamaTransport_allocate(&transport)) == MAMA_STATUS_OK) &&
            ((status = mamaTransport_create(transport, "vmr", bridge)) == MAMA_STATUS_OK))
        {
            printf("Closing Solace middleware bridge.\n");
            mamaTransport_destroy(transport);
            mama_close();
            // normal exit
            exit(0);
        }
    }
    printf("OpenMAMA error: %s\n", mamaStatus_stringForStatus(status));
    exit(status);
}
```

Now our program runs with a failure when creating of the Solace middleware bridge transport, the failure error code is `STATUS_PLATFORM`:

```
2016-06-28 14:08:11: Warn SOLACE-MW-Bridge: (mama/c_cpp/src/c/bridge/solace/logging.c:129): [API] solClientOS.c:3394 (7f1192289700) TCP connection failure for fd 7, error = Connection refused (111)
2016-06-28 14:08:11: Warn SOLACE-MW-Bridge: (mama/c_cpp/src/c/bridge/solace/logging.c:129): [API] solClient.c:11775 (7f1192289700) Session '(c0,s1)' error attempting transport connection, client name 'localhost.localdomain/13964/#00000001', peer address 'IP 127.0.0.1:55555', connection 'tcp_TxRx' local address 'IP 127.0.0.1:57885'
2016-06-28 14:08:11: Warn SOLACE-MW-Bridge: (mama/c_cpp/src/c/bridge/solace/logging.c:129): [API] solClient.c:11276 (7f1192289700) Protocol or communication error when attempting to login for session '(c0,s1)'; are session HOST and PORT correct? client name 'localhost.localdomain/13964/#00000001', peer address 'IP 127.0.0.1:55555', connection 'tcp_TxRx' local address 'IP 127.0.0.1:57885'
2016-06-28 14:08:11: Warn SOLACE-MW-Bridge: (mama/c_cpp/src/c/bridge/solace/transport.c:480): Error in solClient_session_connect() - ReturnCode='Not ready', SubCode='SOLCLIENT_SUBCODE_COMMUNICATION_ERROR', ResponseCode=0, Info='solClientOS.c:3394                   (7f1192289700) TCP connection failure for fd 7, error = Connection refused (111)'
OpenMAMA error: STATUS_PLATFORM
```

This means we cannot go on without configuring a transport for the Solace middleware bridge, and that transport is the **Solace message router**.

Configuring transport for the Solace middleware bridge means creating and editing a configuration file. The recommended name for this file is **mama.properties** and its location needs to be known to the bridge.

Create a text file named **mama.properties** and add to it a minimum set of properties for the **Solace message router**:

```
mama.solace.transport.vmr.session_host=192.168.1.75
mama.solace.transport.vmr.session_username=default
mama.solace.transport.vmr.session_vpn_name=default
mama.solace.transport.vmr.allow_recover_gaps=false
```

Notice how `solace` and `vmr` property token names are the same as in `mama_loadBridge(&bridge, "solace")` and `mamaTransport_create(transport, "vmr", bridge)` calls.

Each property corresponds to one of the [Solace Message Router Properties](#solace-message-router-properties):

*   `mama.solace.transport.vmr.session_host` is `Host` and usually has a value of the IP address of the **Solace message router**.
*   `mama.solace.transport.vmr.session_username` is `Client Username`
*   `mama.solace.transport.vmr.session_password` is optional `Client Password`
*   `mama.solace.transport.vmr.session_vpn_name` is `Message VPN`
*   `mama.solace.transport.vmr.allow_recover_gaps` doesn’t have a default value and specifies whether the Solace middleware bridge should override applications settings for recovering from sequence number gaps in subscriptions. When `allow_recover_gaps` is `true` applications are allowed to specify the gap recovery behaviour. When `allow_recover_gaps` is `false`, the gap recovery is disabled for all subscriptions, regardless of applications settings.

Now we need to modify our program to refer to this **properties file** by its name and location (in the current directory: `"."`):

```c
mama_openWithProperties(".","mama.properties");
```

This is how our program looks now, let’s compile and run it.

```
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
        // create transport
        mamaTransport transport = NULL;
        if (((status = mamaTransport_allocate(&transport)) == MAMA_STATUS_OK) &&
            ((status = mamaTransport_create(transport, "vmr", bridge)) == MAMA_STATUS_OK))
        {
            printf("Closing Solace middleware bridge.\n");
            mamaTransport_destroy(transport);
            mama_close();
            // normal exit
            exit(0);
        }
    }
    printf("OpenMAMA error: %s\n", mamaStatus_stringForStatus(status));
    exit(status);
}
```

Now our program runs without any errors and it successfully connects to the **Solace message router**. If you sleep the main thread before the `mamaTransport_destroy(transport)` call, you can see use **SolAdmin** to see this program as a client connected to the **Solace message router**.

#### Create publisher

The only thing we want to happen with this program is to publish the “Hello World” message and for that we need to create a publisher.

A publisher is created for a specific topic (named `“tutorial.topic”` in this tutorial) and the already created transport:

```c
mamaPublisher publisher = NULL;
mamaPublisher_create(&publisher, transport, "tutorial.topic", NULL, NULL);
```

It needs to have a corresponding `destroy` call:

```c
mamaPublisher_destroy(publisher);
```

The publisher is ready, let’s create the message we’re going to publish.

#### Create message

A message is created with one call, but it is created empty, so we need to add a field with the string `"Hello World"` in it.

```c
mamaMsg message = NULL;
mamaMsg_create(&message);
mamaMsg_addString(message, "MyGreetingField", 99, "Hello World");
```

Notice that our field has a _name_ (`"MyGreetingField"`) and a _field identifier_ a.k.a. _FID_ (`99`). Both are of arbitrary values in this tutorial, but if your program publishes market data, their values need to be assigned according to the **data dictionary**.

#### Send message

At this point the only things left are to send the message and to delete it afterwards to avoid memory leaks:

```c
mamaPublisher_send(publisher, message);
mamaMsg_destroy(message);
```

This is the final look of our program.

```c
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
        // 1\. create transport and 2\. create publisher
        mamaTransport transport = NULL;
        mamaPublisher publisher = NULL;
        if (((status = mamaTransport_allocate(&transport)) == MAMA_STATUS_OK) &&
            ((status = mamaTransport_create(transport, "vmr", bridge)) == MAMA_STATUS_OK) &&
            ((status = mamaPublisher_create(&publisher, transport, "tutorial.topic", NULL, NULL)) == MAMA_STATUS_OK))
        {
            // 3\. create message and add a string field to it
            mamaMsg message = NULL;
            if (((status = mamaMsg_create(&message)) == MAMA_STATUS_OK) &&
                ((status = mamaMsg_addString(message, "MyGreetingField", 99, "Hello World")) == MAMA_STATUS_OK))
            {
                // 4\. send the message
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
```

## Summarizing

Combining the example source code shown above results in the following source code files:

*   [mama.properties]({{ site.repository }}/blob/master/src/helloworld/mama.properties){:target="_blank"}
*   [topicPublishOne.c]({{ site.repository }}/blob/master/src/helloworld/topicPublishOne.c){:target="_blank"}

### Building

To build on **Linux**, assuming OpenMAMA installed into `/opt/openmama`:

    $ gcc -o topicPublishOne topicPublishOne.c -I/opt/openmama/include -L/opt/openmama/lib -lmama

To build on **Windows**, assuming OpenMAMA is at `<openmama>` directory:

    $ cl topicPublishOne.c /I<openmama>\mama\c_cpp\src\c /I<openmama>\common\c_cpp\src\c\windows -I<openmama>\common\c_cpp\src\c <openmama>\Debug\libmamacmdd.lib

## Running the application

On **Linux**:

    $ ./topicPublishOne

On **Windows**:

    $ topicPublishOne.exe

### Sample output

This is the program’s console output (on **Linux**):

```
$ ./topicPublishOne
Solace OpenMAMA tutorial.
Publishing one message with OpenMAMA.
2016-07-13 11:46:10:
********************************************************************************
Note: This build of the MAMA API is not enforcing entitlement checks.
Please see the Licensing file for details
**********************************************************************************
2016-07-13 11:46:10: mamaTransport_create(): No entitlement bridge specified for transport vmr. Defaulting to noop.
Message published, closing Solace middleware bridge.
```

You can see the message published by listening for it on the **Solace message router** with the [`sdkperf_c` utility](http://dev.solacesystems.com/docs/sdkperf-user-guide/):

```
$ ./sdkperf_c -cip=192.168.1.75 -cu=default@default -stl=">" -md

CPU mask currently set to: 0x0003\.  To modify use linux cmd: taskset
CPU Speed calculated (Hz): 2980249271
Client naming used:
    logging ID   = 000001
    username     = default
    vpn          = default
    client names = sdk generated.

APP NOTICE Wed Jul 13 11:45:59.762 2016 PerfClientCollection.cpp:95          (7f65461f9740) Master random seed used : 261933454
APP NOTICE Wed Jul 13 11:45:59.786 2016 AbstractClientCollection.cpp:1744    (7f65461f9740) Router capabilities: (PFG=1, SFG=1, TEMP=1, JNDI=1, Z=1)
Receiving messages.  Ctrl-c to exit.
^^^^^^^^^^^^^^^^^^ Start Message ^^^^^^^^^^^^^^^^^^^^^^^^^^^
Destination:                            Topic 'tutorial/topic'
Class Of Service:                       COS_1
DeliveryMode:                           DIRECT
User Property Map:
  Key 'U' (STRING) dfedorov
  Key 'H' (STRING) localhost.localdomain
  Key 'P' (INT32) 5086
Binary Attachment:                      len=44
  31 01 2f 00 00 00 2a 0c  04 00 63 18 13 65 4d 79      1./...*.   ..c..eMy
  47 72 65 65 74 69 6e 67  46 69 65 6c 64 00 1c 0e      Greeting   Field...
  48 65 6c 6c 6f 20 57 6f  72 6c 64 00                  Hello Wo   rld.

^^^^^^^^^^^^^^^^^^ End Message ^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

Between the _Start Message_ and _End Message_ console output you can see the published message topic `'tutorial/topic'` as **Destination** and the message with the string field **“Hello World”** as **Binary Attachment**.

Congratulations! You have now successfully published a message on a Solace message router using OpenMAMA with the Solace middleware bridge.

If you have any issues with this program, check the [Solace community](http://dev.solacesystems.com/community/){:target="_blank"} for answers to common issues.

## Resources

For more information about OpenMAMA:

*   The OpenMAMA website at: [http://www.openmama.org/](http://www.openmama.org/){:target="_blank"}.
*   The OpenMAMA code repository on GitHub [https://github.com/OpenMAMA/OpenMAMA](https://github.com/OpenMAMA/OpenMAMA){:target="_blank"}.
*   Chat with OpenMAMA developers and users at [Gitter OpenMAMA room](https://gitter.im/OpenMAMA/OpenMAMA){:target="_blank"}.

For more information about Solace technology:

*   The Solace Developer Portal website at: [http://dev.solacesystems.com](http://dev.solacesystems.com/){:target="_top"}
*   Get a better understanding of [Solace technology](http://dev.solacesystems.com/tech/){:target="_top"}.
*   Ask the [Solace community](http://dev.solacesystems.com/community/){:target="_top"}.

Other tutorials and samples:

*   [OpenMAMA Quick Start Guide](http://www.openmama.org/content/quick-start-guide){:target="_blank"}
*   [OpenMAMA Wiki Quick Start Guide](https://github.com/OpenMAMA/OpenMAMA/wiki/Quick-Start-Guide){:target="_blank"}
*   [OpenMAMA Example Walk Through](http://www.openmama.org/example-walk-through){:target="_blank"}.
*   [OpenMAMA Code Examples](https://github.com/OpenMAMA/OpenMAMA/tree/master/mama/c_cpp/src/examples){:target="_blank"}
*   [OpenMAMA Wiki](https://github.com/OpenMAMA/OpenMAMA/wiki){:target="_blank"}
*   [OpenMAMA Documentation and Developers Guides](http://www.openmama.org/documentation){:target="_blank"}
*   [Solace’s Solution for OpenMAMA](http://solacesystems2.staging.wpengine.com/tech/openmama/){:target="_blank"}