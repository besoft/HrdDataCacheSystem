
# Just use the MD5 module from the Python standard library

__revision__ = "$Id: MD5.py,v 1.1 2009-05-19 14:29:56 mucci Exp $"

from md5 import *

import md5
if hasattr(md5, 'digestsize'):
    digest_size = digestsize
    del digestsize
del md5

