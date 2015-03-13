# Setting up a local network node #

To launch the graphical control program which starts/configures the local network node, call the executable `ddcn_control`. The opening window should almost completely be grayed out, indicating that no network node is running yet. To start one, press the icon in the upper left corner.

The first tab in the window shows statistics of the running `ddcn_service` instance. Most important here is the key fingerprint and the node name on the left side - this is your identity in the network. Pressing the "Change" button will bring up a settings dialog that allows you to regenerate, import or export your key. The node name however can appear more than once in a network and exists to provide a readable description of the node in the network overview tab.

Also configurable is the number of local processes the program will try to start parallel. This roughly resembles the `-j` parameter of GNU make and should be about the number of physical CPUs in your system.

# Connecting to other peers #

To connect to other peers in the network, you first have to change the bootstrapping and endpoint settings in the configuration dialog. These are documented at http://www.ariba-underlay.org/wiki/Documentation/Configuration.
Per default `ddcn_service` listens at the local TCP port 5005. It searches for other peers via mDNS and broadcasting. Now, if you start another peer in the network as described above, the program should automatically connect to it. The list of peers found in the network can be viewed at the "Network Status" tab in the control window. This list does _not_ refresh automatically. So you have to press the button below it.
The peers will not exchange compiler jobs yet - per default, a peer does not trust any other peer because this would impose a huge security problem. This means that the you manually have to add single peers or groups of peers to the list of peers you trusts.

## Trusting a single peer ##

There are two ways if you want to trust a single peer:
  * **Trusting an online peer:** Either the peer is online, right click onto the peer in the "Network Status" list and select "Trust this peer...". The icon next to the peer will change to a green shield indicating that the service will now try to delegate compiler jobs to this peer.
  * **Trusting an offline peer:** If the peer is not online, you can import a PEM encoded public key file by pressing "Add trusted peer" in the "Trust settings" tab of the window.

## Joining a group of peers ##

Groups of peers are supported via a shared private key which every peer in the group has and which is used to verify that a peer is a member of a certain group.

Groups are managed in the "Group membership" tab in the window.
To create a new peer group there, press "Create new group". This will auto-generate a random private key which serves as the identity of the group, the name which was given to the group again is purely of a descriptive nature. To invite others to the group, press "Export private group key" and pass the key to the other computers which should join the group. To join the group on these machines, press "Join group" there and select the key file produced in the previous step. You can also import any other PEM encoded private key file if you want to use OpenSSL or a similar utility to create a group key.

It is important to know that being a member of a group does not mean you do automatically trust it. You explicitly have to mark the group trusted as shown in the next step.

## Trusting a group of peers ##

To trust a group of peers, there again are two ways, like with single peers - you can either right-click into the online group list and trust the group in the context menu of the list if there are peers online which are members of the group, or you can import the public key of the group by clicking "Add trusted group" in the "Trust settings" tab.

# Creating a network node without the GUI #

Internally, `ddcn_control` starts `ddcn_service`. Instead this process can also be run without the graphical interface if necessary (e.g. on headless servers). The two processes communicate via a D-Bus interface that also could be used by other front-ends. The D-Bus interface is reachable under the address `org.ddcn.service`. The documentation of the interface is only available as doxygen documentation of the adaptor classes within the `ddcn_service` sources. If you write a front-end and want to change settings, as it is done in the configuration dialog of `ddcn_control`, do not edit the INI file described below but rather call the appropriate functions via D-Bus. This ensures that the local network node will be informed about changes and reconfigures itself.

# Configuration files #

The configuration files for the local network node are, by default, stored in `~/.config/ddcn`. There you will find the private key of the local network node (`privkey.pem`) and an INI file holding the other configurable options available through `ddcn_control`.

# Toolchains #

If you compile in a heterogeneous network, you will get into the situation where different computers have different operating systems or even different hardware architectures. These cases are handled by identifying a computer using a combination of its gcc target triple and the first three (including the dot) letters of its version, for example `i686-linux-gnu/4.4`. The local default compilers at `/usr/bin/gcc` and `/usr/bin/g++` will automatically be recognized and added to the configuration if missing.

## Adding and removing toolchains ##

If you want to add other toolchains with a different gcc version or target architecture to be able to support machines with an incompatible architecture, you will need to manually notify the system of your toolchain. To do so, press the "Add" button on the "Toolchains" tab in the control window which will bring up a file-open dialog. Where you have to select the `gcc` executable of your compiler. Custom prefixes and suffices to the compiler name (e.g. `/usr/bin/arm-cross-gcc-4.6`) are supported so long as neither the suffix nor the prefix contain the word `gcc`.
You can always remove a toolchain by selecting it from the list and pressing the "Remove" button.

## Toolchain compatibility ##

There are some rules when a toolchain is compatible to another.
The easiest one is that they are compatible when their identifier (target triple and version, as described above) are identical. But even when they are not, they might work together well enough:
\smallskip

If the target triple is compatible or identical, computers with a higher gcc version can compile jobs for computers with a lower version. The reason is that new features of newer gcc versions might not be supported by older versions of gcc.

Compilers are incompatible if the target triple differs and the host architecture does not match (e.g. `arm` vs. `i686`). Nevertheless, `amd64` and `x86_64` are still compatible to 32-bit intel architectures. Even if the architecture is the same, the compiler might still use e.g. a different ABI if it was compiled for a different operating system - only target triples containing the word "linux" are considered to be compatible even if they would differ otherwise.