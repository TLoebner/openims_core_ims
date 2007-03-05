/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class RTR_PPR {
	private int id;
	private long hopbyhop;
	private long endtoend;
	private int id_impu;
	private int id_impi;
	private short type;
	private short subtype;
	private int grp;

	public RTR_PPR(){}

	public long getEndtoend() {
		return endtoend;
	}

	public void setEndtoend(long endtoend) {
		this.endtoend = endtoend;
	}

	public int getGrp() {
		return grp;
	}

	public void setGrp(int grp) {
		this.grp = grp;
	}

	public long getHopbyhop() {
		return hopbyhop;
	}

	public void setHopbyhop(long hopbyhop) {
		this.hopbyhop = hopbyhop;
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public int getId_impi() {
		return id_impi;
	}

	public void setId_impi(int id_impi) {
		this.id_impi = id_impi;
	}

	public int getId_impu() {
		return id_impu;
	}

	public void setId_impu(int id_impu) {
		this.id_impu = id_impu;
	}

	public short getSubtype() {
		return subtype;
	}

	public void setSubtype(short subtype) {
		this.subtype = subtype;
	}

	public short getType() {
		return type;
	}

	public void setType(short type) {
		this.type = type;
	}
	
}

