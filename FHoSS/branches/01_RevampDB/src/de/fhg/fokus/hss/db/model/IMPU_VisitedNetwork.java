/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */

public class IMPU_VisitedNetwork {
	private int id;
	private IMPU impu;
	private VisitedNetwork visited_network;
	
	public IMPU_VisitedNetwork(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public IMPU getImpu() {
		return impu;
	}

	public void setImpu(IMPU impu) {
		this.impu = impu;
	}

	public VisitedNetwork getVisited_network() {
		return visited_network;
	}

	public void setVisited_network(VisitedNetwork visited_network) {
		this.visited_network = visited_network;
	}


}

