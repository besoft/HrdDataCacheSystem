"""Miscellaneous modules

Contains useful modules that don't belong into any of the
other Crypto.* subpackages.

Crypto.Util.number        Number-theoretic functions (primality testing, etc.)
Crypto.Util.randpool      Random number generation
Crypto.Util.RFC1751       Converts between 128-bit keys and human-readable
                          strings of words.

"""

__all__ = ['randpool', 'RFC1751', 'number']

__revision__ = "$Id: __init__.py,v 1.1 2009-05-19 14:29:56 mucci Exp $"

