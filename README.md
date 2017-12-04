# Getting Started Examples
## Solace Messaging with OpenMAMA

The "Getting Started" tutorials will get you up to speed and sending messages with Solace technology as quickly as possible. There are three ways you can get started:

- Follow [these instructions]({{ site.links-solaceCloud-setup }}){:target="_top"} to quickly spin up a cloud-based Solace messaging service for your applications.
- Follow [these instructions]({{ site.docs-vmr-setup }}){:target="_top"} to start the Solace VMR in leading Clouds, Container Platforms or Hypervisors. The tutorials outline where to download and how to install the Solace VMR.
- If your company has Solace message routers deployed, contact your middleware team to obtain the host name or IP address of a Solace message router to test against, a username and password to access it, and a VPN in which you can produce and consume messages.

## Contents

This repository contains code and matching tutorial walk through to get you started with OpenMAMA and the Solace transport bridge. For a nice introduction to the OpenMAMA with Solace, check out the [tutorials landing page](https://solacesamples.github.io/solace-samples-openmama/).

See the individual tutorials for details:

- [Linux Installation](https://solacesamples.github.io/solace-samples-openmama/installation-linux)
- [Hello World](https://solacesamples.github.io/solace-samples-openmama/hello-world)
- [Publish/Subscribe](https://solacesamples.github.io/solace-samples-openmama/publish-subscribe)

## Checking out and Building

To check out the project and build it, do the following:

  1. clone this GitHub repository
  2. `cd solace-samples-openmama`

### Hello World

  3. `cd helloworld`
 
To build on **Linux**, assuming OpenMAMA installed into `/opt/openmama`:
```
$ gcc -o topicPublishOne topicPublishOne.c -I/opt/openmama/include -L/opt/openmama/lib -lmama
```

To build on **Windows**, assuming OpenMAMA is at `<openmama>` directory:
```
$ cl topicPublishOne.c /I<openmama>\mama\c_cpp\src\c /I<openmama>\common\c_cpp\src\c\windows -I<openmama>\common\c_cpp\src\c <openmama>\Debug\libmamacmdd.lib
```

### Publish/Subscribe 

  3. `cd pubsub`
 
To build on **Linux**, assuming OpenMAMA installed into `/opt/openmama`:
```
$ gcc -o topicPublishOne topicPublishOne.c -I/opt/openmama/include -L/opt/openmama/lib -lmama
```

To build on **Windows**, assuming OpenMAMA is at `<openmama>` directory:
```
$ cl topicPublishOne.c /I<openmama>\mama\c_cpp\src\c /I<openmama>\common\c_cpp\src\c\windows -I<openmama>\common\c_cpp\src\c <openmama>\Debug\libmamacmdd.lib
```

## Running the Samples

To try individual samples, build the project from source and then run samples like the following:

On **Linux**:

```
$ ./topicPublishOne
```

On **Windows**:

```
$ topicPublishOne.exe
```

The individual tutorials linked above provide full details which can walk you through the samples, what they do, and how to correctly run them to explore Solace messaging.

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

See the list of [contributors](https://github.com/SolaceSamples/solace-samples-openmama/contributors) who participated in this project.

## License

This project is licensed under the Apache License, Version 2.0. - See the [LICENSE](LICENSE) file for details.

## Resources

For more information about OpenMAMA:

- The OpenMAMA website at: [http://www.openmama.org/](http://www.openmama.org/).
- The OpenMAMA code repository on GitHub [https://github.com/OpenMAMA/OpenMAMA](https://github.com/OpenMAMA/OpenMAMA).
- Chat with OpenMAMA developers and users at [Gitter OpenMAMA room](https://gitter.im/OpenMAMA/OpenMAMA).

For more information about Solace technology:

- The Solace Developer Portal website at: http://dev.solace.com
- Get a better understanding of [Solace technology](http://dev.solace.com/tech/).
- Check out the [Solace blog](http://dev.solace.com/blog/) for other interesting discussions around Solace technology
- Ask the [Solace community.](http://dev.solace.com/community/)
