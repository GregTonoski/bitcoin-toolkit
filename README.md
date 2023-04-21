![Version](https://img.shields.io/badge/Version-3.0.0-green.svg)

Like this project? Consider sponsoring me: [https://github.com/sponsors/bartobri](https://github.com/sponsors/bartobri)

Bitcoin Toolkit
===============

Bitcoin Toolkit is a collection of command line tools for working with the Bitcoin network and blockchain. The capabilities of the tools include creating and manipulating public or private keys, generating addresses, querying state info from remote bitcoin nodes, and mining blockchain data.

Bitcoin Toolkit supports JSON-formatted input and output making it useful for developers who want to build custom Bitcoin applications or integrate Bitcoin functionality into existing applications.

See the [Usage](#usage) section for a quick introduction on the basics of Bitcoin Toolkit.

Once installed, there is a comprehensive set of man pages you can accress for more detailed information. You can run `btk help` or `btk help <command>` to access them.

Table of Contents
-----------------

1. [Download and Install](#download-and-install)
2. [Usage](#usage)
3. [License](#license)

Download and Install
--------------------
The following instructions are for unix and linux systems. Windows 10 users can install this project from within the linux subsystem.

Bitcoin Toolkit was built to be installed without any dependencies. However, there are speed and functionality advantages to having certain libraries installed. The following libraries are optional but recommended.

1. libgmp
2. libgcrypt
3. libleveldb

To install them on debian systems:
```
sudo apt-get install libgmp-dev
sudo apt-get install libgcrypt20-dev
sudo apt-get install libleveldb-dev
```

You will also need basic build tools like `gcc` and `make`. You can install them with the following command:

```
sudo apt-get install build-essential
```

Next, follow these instructions:

#### Install:
```
$ git clone https://github.com/bartobri/bitcoin-toolkit.git
$ cd ./bitcoin-toolkit
$ make
$ sudo make install
```

#### Uninstall:

```
$ sudo make uninstall
```

Usage
-----

#### Synopsis

`btk <command> [<command_opts>] [<input_item> ...]`

#### Help and Man Pages

Use the 'help' command to access the man pages.

See `btk help` to learn about available comands and basic usage.

See `btk help <command>` to learn more about a specific command as well as the options that are available.

#### Basic Examples

Bitcoin Toolkit can do a lot of useful and complex operations, but to get a very basic sense of how it works and what it can do, here are some common use cases:

Create a new private key:
```
$ btk privkey --create
[
  "L58TtRui29iURKjVnXWko33VQPco3UTe5gPyhUmG7b6yR7FspDJo"
]
```

Generate a private key from a passphrase:
```
$ btk privkey --in-type=string "Secret Passphrase"
[
  "L1Cf21MBhiZX9QFTAhN3PGJkyvQzN4CuHwhasHsdV9tkEfiiB8Ug"
]
```

Generate a private key from binary data:
```
$ cat photo.jpg | btk privkey --in-type=binary
[
  "KxRgZMj7dY3eFhJR8zd4JR8jCLFqUvdhj5DiNZVe8DyMDbap464K"
]
```

Change the compression of a private key:
```
$ btk privkey --compressed=false KxRgZMj7dY3eFhJR8zd4JR8jCLFqUvdhj5DiNZVe8DyMDbap464K
[
  "5J694FfYQgfD4nP57LrQQRxDjSfFcKwzFyEkp2ULHbCrttPfJPX"
]
```

Generate the public key from a private key:
```
$ btk pubkey KxRgZMj7dY3eFhJR8zd4JR8jCLFqUvdhj5DiNZVe8DyMDbap464K
[
  "0237b921b35555af8d6c27017ec2a4c524eac53011d0a45dba227f14937a17cf3b"
]
```

Generate an address from a public key:
```
$ btk address --bech32 0237b921b35555af8d6c27017ec2a4c524eac53011d0a45dba227f14937a17cf3b
[
  "bc1qs84fqal6f9pfpcxxtvw3wcjeduuuqwfplctcrp"
]
```

Generate an address from a public key in qrcode format:
```
$ btk address --bech32 -Q 0237b921b35555af8d6c27017ec2a4c524eac53011d0a45dba227f14937a17cf3b

█▀▀▀▀▀█ ▄ █▀▀ █  █    █▀▀▀▀▀█
█ ███ █ █ ▀ █▄ ▀ ▄▄▀▀ █ ███ █
█ ▀▀▀ █  ▀▀▄▀▄  ▀▄▄▀▀ █ ▀▀▀ █
▀▀▀▀▀▀▀ █▄▀ █ █▄█ ▀ █ ▀▀▀▀▀▀▀
█ ▄ ▄▄▀ ██ ▄▄▀▀█ ▀ ▄▀▀▀▄▄▀██
▀▄ █▄█▀▀▄▀▀▄  ▀██▀    ▄▄▄█ ▄
▀▄█ ▄ ▀▀▄▀ ▀▄  █▄ ▄▄▄██ ▄  ██
 ██▄▄ ▀▀ █▀  ▀ ▀█▀▀ ▄██▄▀▀██
█▄ ▀ ▀▀ ▄ ▄██▀████▄██▄▄▄▄▀█▀▄
█▀ ▄  ▀█▀▄█▀▄ ▀▀▄ ██ ▀▀ █▀▀ █
▀ ▀  ▀▀▀▄ ▄▀ ▀▀█▀▀▄ █▀▀▀███▀
█▀▀▀▀▀█  ▀▄ █  ▀█  ██ ▀ █▄  ▄
█ ███ █  █▄ █▀▀█▄ ▄▀▀█▀▀▀█ ▀▄
█ ▀▀▀ █  █▄ ▀█▄▀▀ ▄▄ ████▄█▀▄
▀▀▀▀▀▀▀ ▀      ▀▀▀ ▀ ▀▀ ▀▀▀
```

Create key/address pairs:
```
$ btk privkey --create | btk address --bech32 --trace
{
  "Kwbc1yF7G9g8QA3W49ejubBNuU7XkfEyzbPzJSRAiLYZyRJrvym4": [
    "bc1qkknu6wp24au9smjjwazq4wgmlk36rqx2x4ukwy"
  ]
}
```

Generate a vanity address:
```
$ btk privkey --create --stream | btk address --grep="^1bri"
{
  "L3UqmMR9F3pTRpJxFw6k2jCsoQkiRAkLGc7enEiThxjpKVo6ZeGA": [
    "1BRiFm93hjdkEz4GPMTfyxQcbR4dPBTJcz"
  ]
}
```

Query an address balance:
```
$ btk balance 13A1W4jLPP75pzvn2qJ5KyyqG3qPSpb9jM
[
  "5000000000"
]
```

Query the state of any node on the bitcoin network:
```
$ btk node --hostname=seed.bitcoin.sipa.be
{
  "version": 70016,
  "services": {
    "bit 0": "NODE_NETWORK",
    "bit 3": "NODE_WITNESS",
    "bit 10": "NODE_NETWORK_LIMITED"
  },
  "timestamp": 1682030946,
  "addr_recv_services": {
  },
  "addr_recv_ip_address": "00000000000000000000ffffd9b4d894",
  "addr_recv_port": 52614,
  "addr_trans_services": {
    "bit 0": "NODE_NETWORK",
    "bit 3": "NODE_WITNESS",
    "bit 10": "NODE_NETWORK_LIMITED"
  },
  "addr_trans_ip_address": "00000000000000000000000000000000",
  "addr_trans_port": 0,
  "nonce": 1.2171476425208355e+19,
  "user_agent": "/Satoshi:24.0.1/",
  "start_height": 786299,
  "relay": true
}
```

License
-------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License. See [LICENSE](LICENSE) for
more details.
