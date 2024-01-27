# MQTTNox

MQTT Client for Embedded Devices

## Why another MQTT Client?

While there's been several mqtt clients out there, especially the eclipse project PAHO,
those clients are relatively large and complicated to integrate in a real system.

MQTTNox is small, efficient and most importantly easy to use.

Our experience with other MQTT brokers is that they hide a lot of things behind layers which
means it's difficult to use in real products where errors can occur for a variety of reasons.

MQTTNox is designed to provide full access to the underlying system and help build robust MQTT
devices

## Integrating MQTTNox

* Add files in src/lib to your project
* Import header files
    #include "mqttnox.h"
    #include "mqttnox_tal.h"
* Use an existing TCP Abstraction Layer (TAL) file or create your own. This file provides
  standard TCP socket connectivity functions. See `mqttnox_tal.h` for files needed.
* Call library functions
