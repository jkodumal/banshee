package banshee.dyckcfl;

public class DyckCFL {
    private UnsafeDyckCFL cflEngine;

    public DyckCFL() {
	cflEngine = new UnsafeDyckCFL();
    }

    public static void printDyckConstraints(boolean value) {
	UnsafeDyckCFL.printDyckConstraints(value);
    }

    public DyckNode makeTaggedNode(String name) {
	long id = cflEngine.makeTaggedNode(name);
	return new DyckNode(name, id);
    }

    public DyckNode makeUntaggedNode(String name) {
	long id = cflEngine.makeUntaggedNode(name);
	return new DyckNode(name, id);
    }

    public void markNodeGlobal(DyckNode node) {
	cflEngine.markNodeGlobal(node.nodeID);
    }

    public void makeSubtypeEdge(DyckNode node1, DyckNode node2) {
	cflEngine.makeSubtypeEdge(node1.nodeID, node2.nodeID);
    }

    public void makeOpenEdge(DyckNode node1, DyckNode node2, int index) {
	cflEngine.makeOpenEdge(node1.nodeID, node2.nodeID, index);
    }

    public void makeCloseEdge(DyckNode node1, DyckNode node2, int index) {
	cflEngine.makeCloseEdge(node1.nodeID, node2.nodeID, index);
    }

    public void finishedAddingEdges() {
	cflEngine.finishedAddingEdges();
    }

    public boolean checkReaches(DyckNode node1, DyckNode node2) {
	return cflEngine.checkReaches(node1.nodeID, node2.nodeID);
    }
    
    public boolean checkPNReaches(DyckNode node1, DyckNode node2) {
	return cflEngine.checkPNReaches(node1.nodeID, node2.nodeID);
    }
    
}