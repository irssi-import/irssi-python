"""
"""

import sys
import _irssi
from _irssi import *

def command_bind(*args, **kwargs):
    """ see Script.command_bind """
    get_script().command_bind(*args, **kwargs)

def command_unbind(*args, **kwargs):
    """ see Script.command_unbind """
    get_script().command_unbind(*args, **kwargs)

def signal_add(*args, **kwargs):
    """ see Script.signal_add """
    get_script().signal_add(*args, **kwargs)

def signal_remove(*args, **kwargs):
    """ see Script.signal_remove """
    get_script().signal_remove(*args, **kwargs)

def timeout_add(*args, **kwargs):
    """ see Script.timeout_add """
    get_script().timeout_add(*args, **kwargs)

def io_add_watch(*args, **kwargs):
    """ see Script.io_add_watch """
    get_script().io_add_watch(*args, **kwargs)

