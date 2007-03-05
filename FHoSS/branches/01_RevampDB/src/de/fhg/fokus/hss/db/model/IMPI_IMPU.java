package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IMPI_IMPU {
	private int id;
	private IMPI impi;
	private IMPU impu;
	private short user_state;
	
	public IMPI_IMPU(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public short getUser_state() {
		return user_state;
	}

	public void setUser_state(short user_state) {
		this.user_state = user_state;
	}

	public IMPI getImpi() {
		return impi;
	}

	public void setImpi(IMPI impi) {
		this.impi = impi;
	}

	public IMPU getImpu() {
		return impu;
	}

	public void setImpu(IMPU impu) {
		this.impu = impu;
	}
}
