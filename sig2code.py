import re
import sys

code_text = """
 Conversion codes notes
   I = int *arg IN/OUT 
   G = string GList **arg IN/OUT (list must be reconstructed)
   L = list of nicks

 Scalars
   s -> char *
   u -> ulong *
   I -> int *
   i -> int

 Lists of things (completion.c and massjoin.c) 
   G -> GList * of char* 
   L -> GSList of NICK_RECs

 Chat objects
   c -> CHATNET_REC
   S -> SERVER_REC
   C -> CHANNEL_REC
   q -> QUERY_REC
   n -> NICK_REC
   W -> WI_ITEM_REC

 Irssi objects
   d -> DCC_REC

 Other objects
   r -> RECONNECT_REC
   o -> COMMAND_REC
   l -> LOG_REC
   a -> RAWLOG_REC
   g -> IGNORE_REC
   ? -> MODULE_REC
   b -> BAN_REC
   N -> NETSPLIT_REC
   e -> NETSPLIT_SERVER_REC
   ? -> AUTOIGNORE_REC
   O -> NOTIFYLIST_REC
   ? -> THEME_REC
   ? -> KEYINFO_REC
   p -> PROCESS_REC
   t -> TEXT_DEST_REC
   w -> WINDOW_REC
"""

#generate body of transcode function from the list of codes above
def prepair():
    lines = code_text.split('\n')
    codes = [] 
    for ln in lines:
        m = re.match('^.*([a-zA-Z\?]) -> (.*)$', ln)
        if not m: continue
        code, match = m.groups()
        code = code.strip()
        match = match.strip()

        assert code == '?' or code not in codes, "dupe code, " + code
        codes.append(code)

        print "if arg.startswith('%s'): return '%s'" % (match, code)

def transcode(arg):
    if arg.startswith('char *'): return 's'
    if arg.startswith('ulong *'): return 'u'
    if arg.startswith('int *'): return 'I'
    if arg.startswith('int'): return 'i'
    if arg.startswith('GList * of char*'): return 'G'
    if arg.startswith('GSList of NICK_RECs'): return 'L'
    if arg.startswith('CHATNET_REC'): return 'c'
    if arg.startswith('SERVER_REC'): return 'S'
    if arg.startswith('RECONNECT_REC'): return 'r'
    if arg.startswith('CHANNEL_REC'): return 'C'
    if arg.startswith('QUERY_REC'): return 'q'
    if arg.startswith('COMMAND_REC'): return 'o'
    if arg.startswith('NICK_REC'): return 'n'
    if arg.startswith('LOG_REC'): return 'l'
    if arg.startswith('RAWLOG_REC'): return 'a'
    if arg.startswith('IGNORE_REC'): return 'g'
    if arg.startswith('MODULE_REC'): return '?'
    if arg.startswith('BAN_REC'): return 'b'
    if arg.startswith('NETSPLIT_REC'): return 'N'
    if arg.startswith('NETSPLIT_SERVER_REC'): return 'e'
    if arg.startswith('DCC_REC'): return 'd'
    if arg.startswith('AUTOIGNORE_REC'): return '?'
    if arg.startswith('NOTIFYLIST_REC'): return 'O'
    if arg.startswith('THEME_REC'): return '?'
    if arg.startswith('KEYINFO_REC'): return '?'
    if arg.startswith('PROCESS_REC'): return 'p'
    if arg.startswith('TEXT_DEST_REC'): return 't'
    if arg.startswith('WINDOW_REC'): return 'w'
    if arg.startswith('WI_ITEM_REC'): return 'W'
    return '?'

def main():

    print "/* Include in your C module */"
    print "static PY_SIGNAL_SPEC_REC py_sigmap[] = {"

    for ln in sys.stdin:
        ln = ln.strip()
        m = re.match('^\"([^\"]+)\",(.*)$', ln)
        if not m: continue

        signal, args = m.groups()
        
        if signal.startswith('script '): continue
        
        argv = [transcode(a.strip()) for a in args.split(',')]
        argv = ''.join(argv)

        print '  {"%s", "%s", 0, 0, 0},' % (signal, argv)
    
    print "  {NULL}"
    print "};"
    print
    print "#define py_sigmap_len() (sizeof(py_sigmap) / sizeof(py_sigmap[0]) - 1)"
    
if __name__ == '__main__': 
    main()

