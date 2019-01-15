News for mts
============

This file lists the major changes between versions. For a more detailed list of
every change, see the Git log.

Latest
------
* tbd

4.2.0
-----
* Minor: Added Packetizer.
* Patch: Fixed PES packet issue with it's size being limited to``uint16_t``.

4.1.0
-----
* Minor: All parsable objects now have two versions of parse:
  ``parse(stream_reader)``, and
  ``parse(data, size, error)``.

4.0.0
-----
* Major: Upgrade bnb to version 5.

3.0.0
-----
* Major: Upgrade bnb to version 4.

2.0.0
-----
* Major: Upgrade bnb to version 3.

1.0.0
-----
* Major: Initial release.
