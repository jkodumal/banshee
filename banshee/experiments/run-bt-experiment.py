#!/usr/bin/python
import sys
import os
import string
import getopt

options='c:d:p:l:o:s:h'
long_options=['start-with=','end-with=',"help"]

# Default values for command line options
project = "cqual"
#repository = ":pserver:anonymous@cvs.sourceforge.net:/cvsroot/" + project
repository = "/Users/jkodumal/work/local_repositories/" + project
logfilename = "logs/" + project + ".log"
outfilename = "out/" + project 
statefilename = "state/" + project
start_with_entry = 0
end_with_entry = 1
compilescript = "./default_compile.sh"
parser_ns = "../cparser/parser_ns.exe"

# Print a usage message and exit
def usage():
    print("Usage: %s [options]\n\n\
options:\n\
  -c <script>:    run this script to compile the project\n\
  -d <cvsroot>:   specify CVS root\n\
  -p <project>:   name the repository (project) to analyze\n\
  -l <logfile>:   read the CVS log from logfile\n\
  -o <outfile>:   save output as outfile\n\
  -s <statefile>: read/write the analysis state from/to statefile\n\
  -h              show this message\n"
	  % sys.argv[0])

def convert_extension(filename):
    filename[:-2] + ".i"

# Parse command line options
def parse_options():
    global project, repository, logfilename, outfilename,statefilename
    global start_with_entry, end_with_entry
    try:
	opts, args = getopt.getopt(sys.argv[1:],options,long_options)
    except getopt.GetoptError:
	usage()
	sys.exit(2)
    for o,a in opts:
	if (o == '-c'):
	    compilescript = a
	if (o == '-d'):
	    repository = a
	if (o == '-p'):
	    project = a
	if (o == '-l'):
	    logfilename = a
	if (o == '-o'):
	    outfilename = a
	if (o == 's'):
	    statefilename = a
	if (o == '--start-with'):
	    start_with_entry = int(a)
	if (o == '--end-with'):
	    end_with_entry = int(a)
	if (o in ['-h','--help']):
	    usage()
	    sys.exit(0)

# Get the next log entry, exit if we are past the desired stopping
# point. Each entry is a pair consisting of a date string and a list
# of (type, name, version) triples
def next_log_entry(logfile):
    result = []
    date = logfile.readline()
    if (not date):
	sys.exit(0)
    while(True):
	entry = string.split(logfile.readline())
	if (not (len(entry) == 3)):
	    break
	else:
	    result.append(entry)
    return (date,result)

# Get a list of the files with a given extension that have been
# modified in dir_b as compared to dir_a
def get_modified_files(dir_a, dir_b, extension):
    result = []
    b_files = os.popen("find %s -name *%s" % (dir_b,extension))
    for filewline in b_files.readlines():
	file = filewline[:-1]
	if (os.system("bash -c \"diff %s %s >/dev/null\"" 
		      % (file, dir_a + file[len(dir_b):]))):
	    #		  % (file[:len(dir_b)],dir_a + file) )):
	    result.append(file)
    return result

# Read the list of banshee times 
def get_banshee_state(statefile):
    result = []
    while(True):
	line = statefile.readline()
	if (not line):
	    break
	temp = string.split(line)
	result.append((temp[1],int(temp[3])))
    return result

# Get the name of the directory to store the checkout in
def get_dirname(current):
    if ( (current % 2) == 1):
	dirname = project + "_odd"
    else:
	dirname = project + "_even"
    return dirname

def get_filelist(dirname,extension):
    files = os.popen("find %s -name *%s" % (dirname, extension))
    return files.readlines()

# Take a string list and return it as a space separated list, also
# chop off newlines
def list_to_string(list):
    if (len(list) == 0):
	return ""
    result = list[0][:-1]
    for elt in list[1:]:
	result = result + " " + elt[:-1]
    return result

# Take a string list (the output of running the analysis) and process
# it into two output lists
def process_andersen_output(current, output):
    found = False
    statefile = open(statefilename + str(current),"w")
    outfile = open(outfilename + str(current),"w")
    for line in output:
	if (line == '##################\n'):
	    found = True
	if found:
	    outfile.write(line)
	else:
	    statefile.write(line)

# Entry point 
def main():
    parse_options()
    logfile = open(logfilename, "r")
    #skip the initial blank
    logfile.readline()
    # skip the specified entries
    for _ in range(0,start_with_entry):
	next_log_entry(logfile)
    # run one entry to prime the pump
    date,_ = next_log_entry(logfile)
    dirname = get_dirname(start_with_entry)
    os.system("rm -rf %s" % dirname) 
    os.system("cvs -d %s co -D \"%s\" %s >/dev/null" % (repository, date, project))
    build_error = os.system("%s %s" % (compilescript, project))
    if (build_error):
	print "Build error"
	sys.exit(1)
    # run Andersen's analysis, save the state and output 
    os.system("mv %s %s" % (project, dirname))
    files = list_to_string(get_filelist(dirname,".i"))
    cmd = "%s -fserialize-constraints %s 2>/dev/null" % (parser_ns,files)
    output = os.popen(cmd).readlines()
    process_andersen_output(start_with_entry,output)

    # for each entry, do the following:
    # 1. run cvs co -d -D date
    # 2. mv to either project_odd or project_even
    # 3. compile it by running compilescript
    # 4. get the list of modified .i files with get_modified_files
    # 5. find the earliest modified file/time by comparing with banshee state
    # 6. compute the new list of files to analyze, putting recently modded
    #    files on the top of the stack
    # 7. run the alias analysis, rolling back to the specified time 
    # 8. save the new statefile and analysis output
    for current in range(start_with_entry+1,end_with_entry+1):
	statefile = open(statefilename + str(current-1), "r")
	banshee_state = get_banshee_state(statefile)
 	date,_ = next_log_entry(logfile)
	dirname = get_dirname(current)
	os.system("rm -rf %s" % dirname)
	os.system("cvs -d %s co -D \"%s\" %s>/dev/null" % (repository, date, project))
	build_error = os.system("%s %s" % (compilescript, project))
	if (build_error):
	    print "Build error"
	    sys.exit(1)
	os.system("mv %s %s" % (project, dirname))
	modified = get_modified_files(get_dirname(current-1),get_dirname(current),".i")
	print modified

if __name__ == "__main__":
    main()
