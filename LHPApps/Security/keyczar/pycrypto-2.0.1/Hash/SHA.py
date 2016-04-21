
# Just use the SHA module from the Python standard library

__revision__ = "$Id: SHA.py,v 1.1 2009-05-19 14:29:56 mucci Exp $"

from sha import *
import sha
if hasattr(sha, 'digestsize'):
    digest_size = digestsize
    del digestsize
del sha
