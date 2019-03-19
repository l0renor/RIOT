# Overview

This example application shows how to integrate SUIT software updates into a
RIOT application.


# Prerequisites

TODO: python deps

gen_manifest.py: ed25519 pyasn1

TODO: explain how to get aiocoap from source

- RIOT checked out into $RIOTBASE


# Setup network

In one shell:

    $ cd $RIOTBASE/dist/tools/ethos
    $ sudo ./start_network.sh /dev/ttyACM0 riot0 fd00::1/64

Keep this running.

Add a routable address to host:

    $ sudo ip address add fd01::1/128 dev riot0

Start aiocoap-fileserver:

    $ mkdir ${RIOTBASE}/coaproot
    $ path/to/aiocoap/contrib/aiocoap-fileserver ${RIOTBASE}/coaproot


# Initial flash

In order to get a SUIT capable firmware onto the node, do (with in the RIOT
checkout root folder):

    $ BOARD=samr21-xpro make -Cexamples/suit_update clean riotboot/flash -j4


# publish

Currently, the build system assumes that it can publish files by simply copying
them to a configurable folder. For this example, aiocoap-fileserver will then
serve the files via CoAP.

Manifests and image files will be copied to
$(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH).

In examples/suit_update:

    $ BOARD=samr21-xpro SUIT_COAP_SERVER=[fd01::1] make suit/publish

# notify node

If the network has been started as described above, the RIOT node should be
reachable via link-local "fe80::2" on the ethos interface.

    $ SUIT_COAP_SERVER='[fd01::1]' SUIT_CLIENT=[fe80::2%riot0] BOARD=samr21-xpro make suit/notify
