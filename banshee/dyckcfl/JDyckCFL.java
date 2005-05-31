// A wrapper around the DyckCFL Banshee API. For efficiency, this code
// assumes that longs are big enough to store C pointers. Then each
// long (really, a C address), is a unique ID for a node. This
// minimizes native->java conversion, but is hardly safe.

class JDyckCFL {

    // Initializer block ensures the dyckCFL library is initialized
    // when this class is loaded
    static {
	initialize();
    }

    private static native void initialize();

    public static native void printDyckConstraints(boolean value);

    public native long makeTaggedNode(String name);

    public native long makeUntaggedNode(String name);

    public native void markNodeGlobal(long node);

    public native void makeSubtypeEdge(long node1, long node2);

    public native void makeOpenEdge(long node1, long node2, int index);

    public native void makeCloseEdge(long node1, long node2, int index);

    public native void finishedAddingEdges();

    public native boolean checkReaches(long node1, long node2);
    
    public native boolean checkPNReaches(long node1, long node2);
}