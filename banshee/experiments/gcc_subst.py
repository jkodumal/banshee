#!/usr/bin/python
import sys
import os
import string
import random

def is_c_file(nextarg):
    return (os.path.isfile(nextarg) and nextarg[-2:] == '.c' and not nextarg[0] == '-')

def list_to_string_nolf(list):
    if (len(list) == 0):
	return ""
    result = list[0]
    for elt in list[1:]:
	result = result + " " + elt
    return result

def get_canon_int(filename):
    output = os.popen("md5 -s \"%s\"" % filename)
    return output.readlines()[0].split()[3]
    
def already_added(appendline, filename):
    output = os.popen("tail -1 %s" % filename)
    lastline = output.readlines()[0][:-1]
    if (lastline == appendline):
	return True
    return False

def modify_file(filename):
#    os.system("echo \"#ifndef CANON_IDENT_%s\" >> %s" %(get_canon_int(filename),filename))
#    os.system("echo \"#define CANON_IDENT_%s\" >> %s" %(get_canon_int(filename),filename))
    appendline = "const char *CANON_IDENT_%s = \"CANON_IDENT_%s\";" % (get_canon_int(filename), filename)
    if (not already_added(appendline,filename)):
	os.system("echo \"const char *CANON_IDENT_%s = \\\"CANON_IDENT_%s\\\";\" >> %s" % (get_canon_int(filename), filename, filename))
#    os.system("echo \"#endif\" >> %s" %filename)
#    os.system("echo \"const char *CANON_IDENT_%d = \\\"CANON_IDENT_%s\\\";\" >> %s" % (random.randint(0,200000), filename, filename))


def main():
    for arg in sys.argv[1:]:
	if (is_c_file(arg)):
	    #print ("%s is probably an input file... modifying" % arg)
	    modify_file(arg)
	    #print ("Invoking gcc as: gcc %s" % list_to_string_nolf(sys.argv[1:]))
    os.system("gcc %s" % list_to_string_nolf(sys.argv[1:]))

if __name__ == "__main__":
    main()
