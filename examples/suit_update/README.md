# Overview

This example application shows how to integrate SUIT software updates into a
RIOT application.

## Prerequisites

Dependencies:
    
    Python3 : - ed25519 > 1.4
              - pyasn1  > 0.4.5
              - cbor    > 1.0.0 
              - aiocoap > 0.4

When this was implemented aiocoap > 0.4 must be built from source you can follow 
instalation instructions here https://aiocoap.readthedocs.io/en/latest/installation.html. 
If you don't choose to clone the repo locally you still need to download "aiocoap-filesever"
 from https://github.com/chrysn/aiocoap/blob/master/contrib/aiocoap-fileserver.

- RIOT repository checked out into $RIOTBASE

(*) cbor is installed as a dependy of aiocoap but is needed on its own if another 
server is used.

# Setup

### Initial flash

In order to get a SUIT capable firmware onto the node, do (with in the RIOT
checkout root folder):

    $ BOARD=samr21-xpro make -C examples/suit_update clean riotboot/flash -j4

### Setup network

First, you need to compile `ethos`.
Go to `/dist/tools/ethos` and type:

    $ make clean all

Then, you need to compile UHCP.
This tool is found in `/dist/tools/uhcpd`. So, as for `ethos`:

    $ make clean all

In one shell and with the board already flashed and connected to /dev/ttyACM0:

    $ cd $RIOTBASE/dist/tools/ethos
    $ sudo ./start_network.sh /dev/ttyACM0 riot0 fd00::1/64

Keep this running (don't close the shell).

Add a routable address to host:

    $ sudo ip address add fd01::1/128 dev riot0

Start aiocoap-fileserver:

    $ mkdir ${RIOTBASE}/coaproot
    $ <PATH>/aiocoap-fileserver ${RIOTBASE}/coaproot

If aiocoap was cloned and built from source aiocoap-filserver will be located
at <AIOCOAP_BASE_DIR>/aiocoap/contrib.

### Key Generation

To sign the manifest a key must be generated.

In examples/suit_update:

    $ BOARD=samr21-xpro make suit/genkey

### Publish

Currently, the build system assumes that it can publish files by simply copying
them to a configurable folder. For this example, aiocoap-fileserver will then
serve the files via CoAP.

Manifests and image files will be copied to
$(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH).

In examples/suit_update:

    $ BOARD=samr21-xpro SUIT_COAP_SERVER=[fd01::1] make suit/publish

This wil publish into the server new firmware for a samr21-xpro board.

### Notify node

If the network has been started as described above, the RIOT node should be
reachable via link-local "fe80::2" on the ethos interface.

    $ SUIT_COAP_SERVER='[fd01::1]' SUIT_CLIENT=[fe80::2%riot0] BOARD=samr21-xpro make suit/notify

This will notify the node of new availale maifest and it will fetch it.

# In depth explanation

## Node

## Network

For connecting the device with the internet we are using ethos (a simple
etehernet over serial driver).

When executing $RIOTBASE/dist/tools/ethos:
    $ sudo ./start_network.sh /dev/ttyACM0 riot0 fd00::1/64
A tap interface called riot0 is setup and configures fd00::1/64 as a prefix
for the

## Sever and File System Variables

The following variables are defined in makefiles/suit.inc.mk:

    SUIT_COAP_BASEPATH ?= firmware/$(APPLICATION)/$(BOARD)
    SUIT_COAP_SERVER ?= localhost
    SUIT_COAP_ROOT ?= coap://$(SUIT_COAP_SERVER)/$(SUIT_COAP_BASEPATH)
    SUIT_COAP_FSROOT ?= $(RIOTBASE)/coaproot

The following convention is used when naming a manifest

    SLOTx_SUIT_MANIFEST ?= $(BINDIR_APP)-slotx.riot.suit.$(APP_VER).bin
    SLOTx_SUIT_MANIFEST_LATEST ?= $(BINDIR_APP)-slotx.riot.suit.latest.bin

The following default values are using for generating the manifest:

    SUIT_VENDOR ?= RIOT
    SUIT_VERSION ?= $(APP_VER)
    SUIT_DEVICE_ID ?= $(BOARD)
    SUIT_KEY ?= secret.key

All files (both slot binaries, both manifests, copies of manifests with
"latest" instead of $APP_VER in riotboot build) are copied into the folder
$(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH). The manifests contain URLs to
$(SUIT_COAP_ROOT)/* and are signed that way.

The whole tree under $(SUIT_COAP_FSROOT) is expected to be served via CoAP
under $(SUIT_COAP_ROOT). This can be done by e.g., "aiocoap-fileserver $(SUIT_COAP_FSROOT)".

## Makefile recipes

The following recipes are defined in makfiles/suit.inc.mk

suit/manifest: creates manifest for all slots and "latest" tag of each,
    uses following parameters:

    - $(SUIT_KEY): key to sign the manifest
	- $(SUIT_COAP_ROOT): coap root address
	- $(SUIT_DEVICE_ID)
	- $(SUIT_VERSION)
	- $(SUIT_VENDOR)

suit/publish: makes the suit manifest, slotx bin and publishes it to the
    aiocoap-fileserver

    1.- builds slot0 and slot1 bin's
    2.- builds manifest
    3.- creates $(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH) directory
    4.- copy's binaries to $(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH)
    - $(SUIT_COAP_ROOT): root url for the coap resources

suit/notify: triggers a device update, it sends two requests:

    1.- COAP get to check which slot is active on the device
    2.- COAP POST with the url where to fetch the latest manifest for
    the active slot

    - $(SUIT_CLIENT): define the client ipv6 address
    - $(SUIT_COAP_ROOT): root url for the coap resources
    - $(SLOTx_SUIT_MANIFEST_LATEST): name of the resource where the slotx
    manifest is exposed.

suit/genkey: this recipe generates a ed25519 key to sign the manifest

**NOTE: to plugin a new server you would only have to change the suit/publish
recipe, respecting or adjusting to the naming conventions.**

# Todo

* Parse the Manifest
* Fetch and Store Firmware
* Reboot and boot from new firmware
