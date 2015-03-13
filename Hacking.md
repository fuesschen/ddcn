# Coarse structure #

The project is divided into three distinct programs:
  * ddcn\_service: The main local service which does all the networking interaction (delegating/receiving compiler jobs) and which calls gcc locally if files have to be compiled.
  * ddcn\_gcc: A gcc replacement which does not call gcc itself but rather connects to ddcn\_service and submits the parameters. It does not read files itself but only submits its working path and parameters for minimal overhead. It is supposed to exactly behave like the local gcc for each and every possible combination of parameters.
  * ddcn\_control: A graphical configuration and overview program which also connects to the local ddcn\_service, exposes an interface for the service settings and is able to display the current network status.

All these components are connected via D-Bus. The service registers a D-Bus interface at org.ddcn.service and can mostly be viewed via command-line utilities like qdbus, although these usually fail to display functions with user-defined return/parameter types defined within ddcn.

Additionally, there is a helper library ddcn\_crypto which wraps around OpenSSL and provides some easy-to-use classes for encryption.

# Detailed structure #

ddcn\_gcc and ddcn\_control are quite simple, the main complexity of the project lies in ddcn\_service. Some interesting points in the sources of this part:

  * ddcn\_service/main.cpp: Contains main(), creates CompilerService and CompilerNetwork classes and exposes them to D-Bus.
  * CompilerService: This class calls gcc for the jobs which are compiled locally. It also delegated jobs to CompilerNetwork and receives jobs from it. It maintains several queues for the jobs which have to be compiled and selects whether to accept incoming jobs.
  * CompilerNetwork: This class implements the whole network protocol, it accepts remote jobs and sends jobs into the network to those peers which accepted a former job request. It also initiates preprocessing, but does not call the compiler for jobs from the network, instead CompilerService fetches those.
  * NetworkInterface: This class connects the two completely different multhtreading models from ariba and Qt - as both want to run their own event loop, information is passed into the ariba thread via ariba events, and pushed out via signals connected to slots of objects in the qt thread.
  * Other than that, there are no additional threads created in ddcn\_service, instead, the program uses an event-based programming model using Qt signals/slots.

# Decisions #

A number of architectural decisions were made during the project. This section tries to list some of these to ease understanding of the code.

## Preprocessing ##

Preprocessing is done locally even if jobs are delegated to other peers in the network, only compiling can be done by other peers. The opposite would be to let the other peer execute the whole compiler job, doing both preprocessing and compiling there.

Preprocessing locally has the disadvantage that it puts more pressure onto the network connection as all included files have to be transferred. Also not as much load is taken away from the local peer as it has to do preprocessing for **every** file it wants to compile, not only those which are not delegated but rather executed locally.

The huge advantage though is that other peers can go with a very minimal gcc installation and do not have to have all headers installed which the other peers need. As the main target of ddcn is to make the whole problem of distributed compiling possible without large setup costs, this was the chosen option.

TODO: How do others do this? distcc?

## Encryption ##

All source files and compiling results are transmitted 100% secure using TLS encrypted data streams between all clients. The other options would be to only digitally sign the transferred files to ensure integrity, or not to encrypt or sign anything.

Not encrypting can instantly be dismissed as ddcn is supposed to operate within potentially unsafe networks (e.g. over the internet) where a malicious peer could modify either the source code or the resulting compiled code.

Now full encryption might have some measurable overhead, but it is the only option where not only data integrity but also privacy is preferred - for most closed-source projects leaking the sources to everybody while compiling them is not an option.

Also the costs for encryption are held low by not establishing the connection once per compiler job, but only once per peer by the ddcn

## Cryptography library ##

Why didn't we reuse an existing C++ cryptography library and instead hacked our own OpenSSL wrapper together?

Actually I tried pretty hard not to have to do this. I tried to integrate Qt SSL sockets, the Qt Crypto Architecture (QCA) and Botan, but failed with all three of them. The Qt SSL sockets do not offer any way to manually implement the socket portion of them, which is necessary for integration of ariba. QCA does not implement a full client authenticated handshake, and I felt like it was too dangerous if I tried to manually implement authentication, given the little experience I had in that area. Lastly, Botan, while being very feature-rich, only implements blocking input/output - which was a no-go because it does not interact well with the event driven programming style advised by both ariba and Qt.

Also, OpenSSL is not exactly difficult to use - while on the other side you get quite nice control over the performance of your classes and get exactly the data you need.