package banshee.dyckcfl;

class DyckNode {
    String name;
    transient long nodeID;

    DyckNode(String name, long nodeID) {
	this.name = name;
	this.nodeID = nodeID;
    }

    public String getName() {
	return name;
    }
}