SRP-JSON-shim
=============

A shim to interface SRP with JSON data.

*Disclaimer: If you do not have access to the [TWISNet](http://twisnet.eu/) source 
code (available under NDA), then this unfortunately is of limited use to you.*

This code provides a shim between JSON-based simulation data and the 
code to be run on embedded devices. It reads the data about the state of the network
and converts the JSON-description into the data-structures used inside TWISNet/SRP.
The route calculation will be done by the TWISNet code and the results will be written
back to a JSON file.

It is put here to enable outside discussion of
parts not affected by the above-mentioned NDA. This code is licensed under the AGPL (v3)
unless otherwise noted. This code uses the [Jansson library](http://www.digip.org/jansson/)
licensed under the [MIT license](http://www.opensource.org/licenses/mit-license.php).
