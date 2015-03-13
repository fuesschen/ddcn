# Problems #

## Compiling fails in ddcn\_service/main.cpp ##

Make sure your ariba is built with support for log4cxx or just comment that line out.

## mdns bootstrapping not working ##

Check whether you built ariba with mdns bootstrapping enabled in "./configure". This is only done if the headers needed for it have been installed. This is **not** done on the spovnet sdk virtualbox image!

## Compiling locally with ddcn\_gcc is much slower than without it ##

If you passed the correct number of threads to make when compiling, check that also ddcn\_service is configured to start the right number of processes in parallel. See the [ddcn\_service configuration page](Configuration.md) for instructions.

## Adding other peers to the network does not seem to speed up compilation at all ##

Three possible reasons:

First, there might not be any network connection at all if bootstrapping fails. Check the output of the ddcn\_control network overview to see whether there are connections, and check your bootstrapping settings if there are none.

Second, check how many processes your "make" starts in parallel. ddcn will always try to compile files locally and will only start to use other peers if there are more requests in the local queue than there are processors. Always use a much higher thread count when starting make than you would use usually, e.g. "make -j20".

Third, there might not be any peer in the network which supports the local toolchain version you are compiling with. A peer with a lower gcc version can not compile code for a peer with a higher gcc version, and a 32-bit peer can not compile code for a 64-bit peer. You need to build a crosscompiler for this.