#!/usr/bin/env python3

import base64
import calendar
import logging
import time

import asyncio

import cbor
import aiocoap.resource as resource
import aiocoap

import ed25519
from pyasn1.type import univ
from pyasn1.type.namedtype import NamedType, NamedTypes
from pyasn1.codec.der.decoder import decode as der_decoder

FIRMWARE = 'firmware-slot{}.bin'
KEYFILE = '/home/hydrazine/dev/suit-manifest-generator/test-ed25519.der'


class Identifier(univ.Sequence):
    componentType = NamedTypes(
        NamedType('type', univ.ObjectIdentifier())
    )


class PrivKeyContainer(univ.OctetString):
    pass


class EddsaPrivateKey(univ.Sequence):
    componentType = NamedTypes(
        NamedType('Version', univ.Integer()),
        NamedType('Type', Identifier()),
        NamedType('PrivateKey', univ.OctetString()),
    )


def _get_skey(skey_container):
    container = skey_container['PrivateKey']
    private_key, _ = der_decoder(container, asn1Spec=PrivKeyContainer)
    return bytes(private_key)


def _parse_privkey(skey):
    if skey.startswith(b"-----BEGIN PRIVATE KEY-----"):
        pem_input = skey.decode('ascii')
        skey = base64.b64decode(''.join(pem_input.splitlines()[1:-1]))
    keys_input, _ = der_decoder(skey, asn1Spec=EddsaPrivateKey())
    return ed25519.SigningKey(_get_skey(keys_input))


def _format_sign1(payload, protected, unprotected, signature):
    return [
            cbor.dumps(protected),
            {},
            payload,
            signature
            ]


def _gen_signature(key, payload, protected):
    sig = [
            "Signature1",
            cbor.dumps(protected),
            b'',
            payload
            ]
    return key.sign(cbor.dumps(sig))


def _sign1(payload, kid, key):
    protected = {4: kid, 1: -8}
    signature = _gen_signature(key, payload, protected)
    return _format_sign1(payload, protected, {}, signature)


class FirmwareResource(resource.Resource):
    def __init__(self, num):
        super().__init__()
        self.content = None
        self.num = num
        with open(FIRMWARE.format(num), 'rb') as f:
            self.content = f.read()

    def get_link_description(self):
        # Publish additional data in .well-known/core
        return dict(**super().get_link_description(), title="FW resource")

    async def render_get(self, request):
        with open(FIRMWARE.format(self.num), 'rb') as f:
            self.content = f.read()
        print("FIRMWARE_SIZE: {}".format(len(self.content)))
        return aiocoap.Message(payload=self.content)


class TimeResource(resource.Resource):
    """Example resource that can be observed. The `notify` method keeps
    scheduling itself, and calles `update_state` to trigger sending
    notifications."""

    def __init__(self):
        super().__init__()

    async def render_get(self, request):
        epoch = calendar.timegm(time.gmtime())
        fmt = cbor.dumps(cbor.Tag(1, epoch))
        print(fmt)
        with open(KEYFILE, 'rb') as f:
            keydata = f.read()
            skey = _parse_privkey(keydata)
            payload = _sign1(fmt, "test", skey)
            return aiocoap.Message(payload=cbor.dumps(payload))


# logging setup
logging.basicConfig(level=logging.INFO)
logging.getLogger("coap-server").setLevel(logging.DEBUG)


def main():
    # Resource tree creation
    root = resource.Site()

    root.add_resource(('.well-known', 'core'),
                      resource.WKCResource(root.get_resources_as_linkheader))
    root.add_resource(('t',), TimeResource())
    root.add_resource(('fw', '1'), FirmwareResource(1))
    root.add_resource(('fw', '2'), FirmwareResource(2))

    asyncio.Task(aiocoap.Context.create_server_context(root))
    asyncio.get_event_loop().run_forever()


if __name__ == "__main__":
    main()
