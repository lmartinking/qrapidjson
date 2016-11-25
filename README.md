qrapidjson
==========

A faster JSON serialiser for kdb+/q (uses RapidJSON).

Behavior is similar to the builtin `.j.j`, differing in:

 * The serialisation of infinity (eg: `0w` or `0wj`) will become "Inf" or "-Inf" per relaxed JSON spec
 * The precision of serialisation of doubles and floats is left to RapidJSON (rather than `\P`)

# Build

    $ make {m32,m64,l32,l64}
    $ cp qrapidjson_{m32,m64,l32,l64}.so /path/to/q/bin

# Use

    q) tojson: (`$"qrapidjson_m64") 2:(`tojson;1); / change m64 to appropriate platform
    q) tojson `a`b`c!(1 2 3) / returns a string

NOTE: You might need to set `DYLD_LIBRARY_PATH` or `LD_LIBRARY_PATH` environment variables
(Mac and Linux respectively) to the directory where the `.so` lives before running `q`.

# Licence

LGPLv3. See `LICENSE` and `COPYING.LESSER`.

Copyright (c) 2016 Lucas Martin-King.

Other parts of this software (eg: RapidJSON) are covered by other licences.
