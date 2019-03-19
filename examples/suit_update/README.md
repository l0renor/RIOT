# Overview

This example application shows how to integrate SUIT software updates into a
RIOT application.

# Prerequisites

TODO: python deps

gen_manifest.py: ed25519 pyasn1

TODO: explain how to get aiocoap from source

# publish

Currently, the build system assumes that it can publish files by simply copying
them to a configurable folder.

Manifests and image files will be copied to
$(SUIT_COAP_FSROOT)/$(SUIT_COAP_BASEPATH).

In examples/suit_update:

    $ BOARD=samr21-xpro SUIT_COAP_SERVER=<ip address> make suit/publish

# CoAP server

- start aiocoap-fileserver:

    $ path/to/aiocoap/contrib/aiocoap-fileserver ${RIOTBASE}/coaproot

