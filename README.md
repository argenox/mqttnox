# MQTTNox - MQTT Client for Embedded Devices

MQTTNox is a complete MQTT client that's cross platform and enables optimized simple access
to MQTT brokers.

## Why another MQTT Client?

While there's are several mqtt clients out there, these clients tend to be relatively large 
and complicated to integrate in a real embedded system.

MQTTNox is small, efficient, and most importantly easy to use.

Our experience with other MQTT clients is that they hide a lot of things behind the scenes which
makes them difficult to use in real products where errors can occur for a variety of reasons.

Many of these clients are also vendor/platform specific, making reusing of code difficult.
Some of these have terrible code, wierd behaviors and side effects that make creating a reliable 


## Integrating MQTTNox

* Add files in src/lib to your project
* Import header files
    #include "mqttnox.h"
    #include "mqttnox_tal.h"

* Use an existing TCP Abstraction Layer (TAL) file or create your own. This file provides
  standard TCP socket connectivity functions. See `mqttnox_tal.h` for files needed.

* Call library functions