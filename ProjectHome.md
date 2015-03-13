ddcn is a distributed compiling tool comparable to distcc, but does not need any setup and infrastructure as it is decentral and builds upon spontaneous virtual networks (ariba, MCPO) for communication with the peers.

# Features #
ddcn...
  * supports compiling across multiple machine architectures and compiler versions.
  * can automatically build up connections to machines in your network (via mdns).
  * can transparently replace gcc so that it perfectly supports most build tools.
  * has a graphical control program to change settings and display the network status.
  * can mostly be configured from the command line as well via D-Bus.
  * supports C and C++.
  * **is selfhosting**!

# Documentation #

  * [Building ddcn](Building.md)
  * [Configuring the service](Configuration.md)
  * [Running ddcn\_gcc](Running.md)
  * [Troubleshooting](Troubleshooting.md)
  * [Screenshots](Screenshots.md)
  * [Hacking ddcn (source internals)](Hacking.md)
  * [Benchmarks](Benchmarks.md)
  * [Things which are still left to do](TODO.md)