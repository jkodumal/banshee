#!/usr/bin/python
import sys
import os
import string
import getopt

options='c:d:p:l:o:s:h'
long_options=['start-with=','end-with=','analysis=',"help"]

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
analysis = "../cparser/parser_ns.exe"
simfilename = "simulations/" + project + ".sim"

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
    global start_with_entry, end_with_entry, analysis 
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
	if (o == '--analysis'):
	    analysis = a

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
# modified in dir_b as compared to dir_a. The directory name will be
# the project name originally checked out, w/o _odd or _even
def get_modified_files(dir_a, dir_b, extension):
    result = []
    b_files = os.popen("find %s -name *%s" % (dir_b,extension))
    for filewline in b_files.readlines():
	file = filewline[:-1]
	if (os.system("bash -c \"diff %s %s >/dev/null\"" 
		      % (file, dir_a + file[len(dir_b):]))):
	    result.append(project + file[len(dir_b):])
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

def list_to_string_nolf(list):
    if (len(list) == 0):
	return ""
    result = list[0]
    for elt in list[1:]:
	result = result + " " + elt
    return result


# Take a string list (the output of running the analysis) and process
# it into two output lists
def process_andersen_output(current, output, prefix):
    found = False
    statefile = open(statefilename + str(current),"w")
    outfile = open(outfilename + str(current),"w")
    for entry in prefix:
	statefile.write("file: %s clock: %d\n" % entry)
    for line in output:
	if (line == '##################\n'):
	    found = True
	if found:
	    outfile.write(line)
	else:
	    statefile.write(line)
    statefile.close()
    outfile.close()

# Compute the rollback time from the previous analysis state, given
# that we are rolling back to file
def compute_time(file, state):
    time = 0
    for (nextfile,next_time) in state:
	if (file == nextfile):
	    return time
	time = next_time
    return time

# Compute the new list of files to analyze, given that we are rolling
# back to file
def compute_stack(modified, file, state, time):
    # First, take the prefix of the list up to file
    prefix = state[:state.index((file,time))]
    # Next, take the suffix of filelist starting with file, but filtering out
    # any modifieds
    suffix = [elem for elem,_ in state[state.index((file,time)):] if (not elem in modified)]
    return (suffix + modified),prefix

# Given the list of modified files, the new file list, and the old
# analysis state, compute a new filelist (stack) and the rollback time
def get_new_stack_and_time(modified, state):
    for file,nexttime in state:
	if file in modified:
	    time = compute_time(file, state)
	    stack,prefix = compute_stack(modified, file, state, nexttime)
	    return (time,stack, prefix)
    # TODO-- what if we just added a file, so it's not in state?
    print "Warning: failed to compute rollback time and new stack; presumably the commit just added some files..."
    return (state[-1][1], modified, state)

def write_simulation_data(modified, files, prefix,simfile):
    prefiles = []
    for (file,_) in prefix:
	prefiles.append(file)
    if (len(files) > 0):
	reanalysis_size = int(os.popen("du -sck %s | grep total" % list_to_string_nolf(files)).readlines()[0].split()[0])
    else:
	reanalysis_size = 0
    if (len(prefiles) > 0):
	preanalyzed_size = int(os.popen("du -sck %s | grep total" % list_to_string_nolf(prefiles)).readlines()[0].split()[0])
    else:
	preanalyzed_size = 0
    simfile.write("modified files: %s\n" % list_to_string_nolf(modified) )
    simfile.write("analyzed files: %s\n" % list_to_string_nolf(files) )
    simfile.write("analyzed: %d total: %d percent: %f\n" % (reanalysis_size,reanalysis_size + preanalyzed_size, float(reanalysis_size) / float(reanalysis_size + preanalyzed_size)))

# Entry point 
def main():
    parse_options()
    simfile = open(simfilename, "w")
    logfile = open(logfilename, "r")
    #skip the initial blank
    logfile.readline()
    project_prev = project + "_prev" 
    # skip the specified entries
    for _ in range(0,start_with_entry):
	next_log_entry(logfile)
    # run one entry to prime the pump
    os.system("rm -rf %s" % project)
    date,_ = next_log_entry(logfile)
    os.system("cvs -d %s co -D \"%s\" %s >/dev/null" % (repository, date, project))
    build_error = os.system("%s %s" % (compilescript, project))
    if (build_error):
	print "Build error"
	sys.exit(1)
    # run Andersen's analysis, save the state and output 
    files = list_to_string(get_filelist(project,".i"))
    cmd = "%s -fserialize-constraints %s 2>/dev/null" % (analysis,files)
    output = os.popen(cmd).readlines()
    process_andersen_output(start_with_entry,output,[])
    # move the analysis to the _prev directory
    os.system("rm -rf %s" % project_prev) 
    os.system("mv %s %s" % (project, project_prev))

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
	os.system("rm -rf %s" % project)
	statefile = open(statefilename + str(current-1), "r")
	banshee_state = get_banshee_state(statefile)
 	date,_ = next_log_entry(logfile)
	os.system("cvs -d %s co -D \"%s\" %s>/dev/null" % (repository, date, project))
	build_error = os.system("%s %s" % (compilescript, project))
	if (build_error):
	    print "Build error"
	    sys.exit(1)
	modified = get_modified_files(project_prev,project,".i")
	time,files,prefix = get_new_stack_and_time(modified,banshee_state)
	print "Backtracking to : %s" % time
	cmd = "%s -fserialize-constraints -fdeserialize-constraints -fback%s %s 2>/dev/null" % (analysis, time, list_to_string_nolf(files))
	print cmd
	output = os.popen(cmd).readlines()
	process_andersen_output(current,output,prefix)
	write_simulation_data(modified, files, prefix, simfile)
  	os.system("rm -rf %s" % project_prev)
	os.system("mv %s %s" % (project, project_prev))

    logfile.close()
    simfile.close()
    os.system("rm -rf %s" % project)
    os.system("rm -rf %s" % project_prev)
    os.system("rm -f andersen.out")

if __name__ == "__main__":
    main()
