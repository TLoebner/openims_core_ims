/**
 * 
 */
package de.fhg.fokus.hss.db.model;

/**
 * @author adp dot fokus dot fraunhofer dot de 
 * Adrian Popescu / FOKUS Fraunhofer Institute
 */
public class IFC {
	private int id;
	private String name;
	private ApplicationServer application_server;
	private TP tp;
	private int profile_part_ind;
	
	//associations
	private ApplicationServer applicationServer;
	
	public IFC(){}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public ApplicationServer getApplication_server() {
		return application_server;
	}

	public void setApplication_server(ApplicationServer application_server) {
		this.application_server = application_server;
	}

	public TP getTp() {
		return tp;
	}

	public void setTp(TP tp) {
		this.tp = tp;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public int getProfile_part_ind() {
		return profile_part_ind;
	}

	public void setProfile_part_ind(int profile_part_ind) {
		this.profile_part_ind = profile_part_ind;
	}

	public ApplicationServer getApplicationServer() {
		return applicationServer;
	}

	public void setApplicationServer(ApplicationServer applicationServer) {
		this.applicationServer = applicationServer;
	}
	
}

