#!/usr/bin/python
import sys
import os
import string

def is_c_file(nextarg):
    return (os.path.isfile(nextarg) and nextarg[-2:] == '.c' and not nextarg[0] == '-')

def list_to_string_nolf(list):
    if (len(list) == 0):
	return ""
    result = list[0]
    for elt in list[1:]:
	result = result + " " + elt
    return result

def modify_file(filename):
#    os.system("echo \"#ifndef CANON_IDENT\" >> %s" %filename)
#    os.system("echo \"#define CANON_IDENT\" >> %s" %filename)
    os.system("echo \"const char *CANON_IDENT = \\\"CANON_IDENT_%s\\\";\" >> %s" % (filename, filename))
#    os.system("echo \"#endif\" >> %s" %filename)

def main():
    for arg in sys.argv[1:]:
	if (is_c_file(arg)):
	    print ("%s is probably an input file... modifying" % arg)
	    modify_file(arg)
    print ("Invoking gcc as: gcc %s" % list_to_string_nolf(sys.argv[1:]))
    os.system("gcc %s" % list_to_string_nolf(sys.argv[1:]))

if __name__ == "__main__":
    main()
