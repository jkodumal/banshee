package banshee.dyckcfl;

// A wrapper around the DyckCFL Banshee API. For efficiency, this code
// assumes that longs are big enough to store C pointers. Then each
// long (really, a C address), is a unique ID for a node. This
// minimizes native->java conversion, but is hardly safe.

public class UnsafeDyckCFL {

    // Initializer block ensures the dyckCFL library is initialized
    // when this class is loaded
    static {
	System.loadLibrary("jdyckcfl");
	initialize();
    }

    public UnsafeDyckCFL() {
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

    public static void main(String args[]) {
	UnsafeDyckCFL cflEngine = new UnsafeDyckCFL();
	long n1 = cflEngine.makeTaggedNode("foo");
	long n2 = cflEngine.makeTaggedNode("bar");
	long n3 = cflEngine.makeTaggedNode("baz");
	cflEngine.makeSubtypeEdge(n1,n2);
	cflEngine.finishedAddingEdges();
	System.out.println("Checking reachability (should be true): " + cflEngine.checkReaches(n1,n2));
	System.out.println("Checking reachability (should be false): " + cflEngine.checkReaches(n1,n3));
    }

}