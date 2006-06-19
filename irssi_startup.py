import sys, imp, __builtin__
import _irssi

class Output:
    def __init__(self, level):
        self.level = level
        self.buf = []
    def write(self, text):
        if not text:
            return
        self.buf.append(text)
        if '\n' == text[-1]:
            text = ''.join(self.buf)[:-1]
            for line in text.split('\\n'):
                _irssi.active_win().prnt(line, self.level)
            self.buf = []

#XXX: hardcode
#try:
sys.stdout = Output(level = 0x0080000)
sys.stderr = Output(level = 0x0100000)
#except Exception, e:
#    print 'Cant set output', e
#    sys.stdout = sys.__stdout__
#    sys.stderr = sys.__stderr__

"""
#Import stuff modified from <Python>/Demo/imputil/knee.py
#If a script instance is available, it's module dictionary
#is used in place of sys.modules
#XXX: needs testing
#XXX: copyright?

# Save the original hooks
original_import = __builtin__.__import__
original_reload = __builtin__.reload

# Replacement for __import__()
def import_hook(name, globals=None, locals=None, fromlist=None):
    print 'LOADING', name
    if name == 'sys':
        return sys
    #if name == '_irssi':
    #    return _irssi
    
    try:
        script = _irssi.get_script()
    except RuntimeError:
        print 'ORIG IMPORT', name
        return original_import(name, globals, locals, fromlist)

    parent = determine_parent(globals, script)
    q, tail = find_head_package(parent, name, script)
    m = load_tail(q, tail, script)
    if not fromlist:
        return q

    if hasattr(m, "__path__"):
        ensure_fromlist(m, fromlist, script)
    
    recur -= 1
    return m

def determine_parent(globals, script):
    if not globals or not globals.has_key("__name__"):
        print 'DP no __name__ in globals'
        return None
    pname = globals['__name__']
    print 'DP pname', pname
    if globals.has_key("__path__"):
        parent = script.modules[pname]
        assert globals is parent.__dict__
        return parent
    if '.' in pname:
        i = pname.rfind('.')
        pname = pname[:i]
        parent = script.modules[pname]
        assert parent.__name__ == pname
        return parent
    return None

def find_head_package(parent, name, script):
    if '.' in name:
        i = name.find('.')
        head = name[:i]
        tail = name[i+1:]
    else:
        head = name
        tail = ""
    if parent:
        qname = "%s.%s" % (parent.__name__, head)
    else:
        qname = head
    q = import_module(head, qname, parent, script)
    if q: 
        return q, tail
    else:
        print 'FHP (1) no q module for', name

    if parent:
        qname = head
        parent = None
        q = import_module(head, qname, parent, script)
        if q: 
            return q, tail
        else:
            print 'FHP (2) no q module for', name

    raise ImportError, "No module named " + qname

def load_tail(q, tail, script):
    m = q
    while tail:
        i = tail.find('.')
        if i < 0: i = len(tail)
        head, tail = tail[:i], tail[i+1:]
        mname = "%s.%s" % (m.__name__, head)
        m = import_module(head, mname, m, script)
        if not m:
            raise ImportError, "No module named " + mname
    return m

def ensure_fromlist(m, fromlist, script, recursive=0):
    for sub in fromlist:
        if sub == "*":
            if not recursive:
                try:
                    all = m.__all__
                except AttributeError:
                    pass
                else:
                    assert recursive == 0
                    ensure_fromlist(m, all, script, 1)
            continue
        if sub != "*" and not hasattr(m, sub):
            subname = "%s.%s" % (m.__name__, sub)
            submod = import_module(sub, subname, m, script)
            if not submod:
                raise ImportError, "No module named " + subname

def import_module(partname, fqname, parent, script):
    try:
        m = script.modules[fqname]
        #if hasattr(m, '_script'):
        #    assert m._script == script

        _irssi.prnt('LOADING CACHED %s script -> %s' % (fqname, repr(script)))

        return m
    except KeyError:
        print 'IM no cached moddule for', fqname
        pass

    try:
        fp, pathname, stuff = imp.find_module(partname,
                                              parent and parent.__path__)
    except ImportError, e:
        print 'IM import error', e
        return None

    try:
        m = imp.load_module(fqname, fp, pathname, stuff)
    finally:
        if fp: fp.close()
    if parent:
        setattr(parent, partname, m)

    #don't load script into builtins, extensions, or this wrapper
    if hasattr(m, '__file__') and fqname != 'irssi':
        m._script = script
    script.modules[fqname] = m

    #if hasattr(m, '__file__') and fqname not in ('__builtin__', 'sys', '__main__', '_irssi'):
    #    del sys.modules[fqname]

    _irssi.prnt('GOT -> %s, SCRIPT -> %s' % (m, repr(script)))

    return m


# Replacement for reload()
def reload_hook(module):
    _irssi.prnt('reloading ' + repr(module))
    try:
        script = _irssi.get_script()
    except RuntimeError:
        return original_reload(module)

    name = module.__name__
    if '.' not in name:
        return import_module(name, name, None, script)
    i = name.rfind('.')
    pname = name[:i]
    parent = script.modules[pname]
    return import_module(name[i+1:], name, parent, script)


# Now install our hooks
__builtin__.__import__ = import_hook
__builtin__.reload = reload_hook
"""

