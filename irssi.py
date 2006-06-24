"""
"""

import sys
import _irssi
from _irssi import *

#XXX: what if the values change? (better to compile in constants?)
#Level constants (coppied from levels.h valid Irssi 0.8.10)
MSGLEVEL_CRAP         = 0x0000001
MSGLEVEL_MSGS         = 0x0000002
MSGLEVEL_PUBLIC       = 0x0000004
MSGLEVEL_NOTICES      = 0x0000008
MSGLEVEL_SNOTES       = 0x0000010
MSGLEVEL_CTCPS        = 0x0000020
MSGLEVEL_ACTIONS      = 0x0000040
MSGLEVEL_JOINS        = 0x0000080
MSGLEVEL_PARTS        = 0x0000100
MSGLEVEL_QUITS        = 0x0000200
MSGLEVEL_KICKS        = 0x0000400
MSGLEVEL_MODES        = 0x0000800
MSGLEVEL_TOPICS       = 0x0001000
MSGLEVEL_WALLOPS      = 0x0002000
MSGLEVEL_INVITES      = 0x0004000
MSGLEVEL_NICKS        = 0x0008000
MSGLEVEL_DCC          = 0x0010000
MSGLEVEL_DCCMSGS      = 0x0020000
MSGLEVEL_CLIENTNOTICE = 0x0040000
MSGLEVEL_CLIENTCRAP   = 0x0080000
MSGLEVEL_CLIENTERROR  = 0x0100000
MSGLEVEL_HILIGHT      = 0x0200000
MSGLEVEL_ALL          = 0x03fffff
MSGLEVEL_NOHILIGHT    = 0x1000000 
MSGLEVEL_NO_ACT       = 0x2000000
MSGLEVEL_NEVER        = 0x4000000
MSGLEVEL_LASTLOG      = 0x8000000

def command_bind(*args, **kwargs):
    """ see Script.command_bind """
    get_script().command_bind(*args, **kwargs)

def signal_add(*args, **kwargs):
    """ see Script.signal_add """
    get_script().signal_add(*args, **kwargs)
