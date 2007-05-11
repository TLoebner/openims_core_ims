<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="java.util.*, de.fhg.fokus.hss.db.model.*, de.fhg.fokus.hss.util.SecurityPermissions" %>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> AS </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
<jsp:useBean id="attached_ifc_list" type="java.util.List" scope="request"></jsp:useBean>

<%
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(action, associated_ID) {

	switch(action){

		case 1:
			document.AS_Form.nextAction.value="save";
			document.AS_Form.submit();
			break;
		case 2:
			document.AS_Form.nextAction.value="refresh";
			document.AS_Form.submit();			
			break;
		case 3:
			document.AS_Form.nextAction.value="reset";
			document.AS_Form.reset();
			break;
		case 4:
			document.AS_Form.nextAction.value="delete";
			document.AS_Form.submit();
			break;
		case 5:
			document.AS_Form.nextAction.value="detach_ifc";
			document.AS_Form.associated_ID.value = associated_ID;			
			document.AS_Form.submit();			
			break;
		case 12:
			document.AS_Form.nextAction.value="attach_ifc";
			document.AS_Form.submit();			
			break;
	}
}

</script>
</head>

<body>
<html:form action="/AS_Submit">
	<html:hidden property="nextAction" value=""/>
	<html:hidden property="associated_ID" value=""/>
		
	<table align=center valign=middle height=100%>
	<!-- Print errors, if any -->
	<tr>
		<td>
			<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
		</td>
	</tr>	
	<tr>
		<td align="center"><h1>  Application Server -AS- </h1></td>
	</tr>
	<tr>
		<td>
	 		<table border="0" align="center" width="800" >						
 			<tr>
 				<td>
					<table border="0" cellspacing="1" align="center" width="70%" style="border:2px solid #FF6600;">						
					<tr bgcolor="#FFCC66">
						<td> ID </td>
						<td><html:text property="id" readonly="true" styleClass="inputtext_readonly"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Name </td>
						<td><html:text property="name" styleClass="inputtext" style="width:250px;"/> </td>
					</tr>

					<tr bgcolor="#FFCC66">
						<td>Server Name </td>
						<td><html:text property="server_name" styleClass="inputtext" style="width:250px;"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Default Handling</td>
						<td>
							<html:select property="default_handling" name="AS_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:optionsCollection name="AS_Form" property="select_default_handling" label="name" value="code"/>
							</html:select>
						</td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Service Info</td>
						<td><html:text property="service_info" styleClass="inputtext" style="width:250px;"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Diameter Peer</td>
						<td><html:text property="diameter_address" styleClass="inputtext_readonly" readonly="true" style="width:250px;"/> </td>
					</tr>

					<tr bgcolor="#FFCC66">
						<td> Rep-Data Limit </td>				
						<td><html:text property="rep_data_size_limit" styleClass="inputtext" style="width:100px;"/> </td>
					</tr>
					
					</table>		
				</td>
				<td>
					<b>Sh Interface - Permissions</b>	
					<table class="as" border="0" cellspacing="1" align="center" style="border:2px solid #FF6600;">								
					<tr class="header">
						<td width="60%"> Permission for </td>
						<td class="header"> UDR </td>
						<td class="header"> PUR </td>
						<td class="header"> SNR </td>
					</tr>

					<tr bgcolor="#FFCC66">
						<td> Allowed Request </td>				
						<td> <html:checkbox property="udr" styleClass="inputbox" />	 </td>										
						<td> <html:checkbox property="pur" styleClass="inputbox" />	 </td>										
						<td> <html:checkbox property="snr" styleClass="inputbox" />	 </td>										
					</tr>
					
					<tr bgcolor="#FFCC66">
						<td> Repository-Data </td>				
						<td><html:checkbox property="udr_rep_data" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="pur_rep_data" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="snr_rep_data" styleClass="inputbox" />	</td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> IMPU </td>				
						<td><html:checkbox property="udr_impu" styleClass="inputbox" />	</td>										
						<td></td>										
						<td><html:checkbox property="snr_impu" styleClass="inputbox" />	</td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> IMS User State </td>				
						<td><html:checkbox property="udr_ims_user_state" styleClass="inputbox" />	</td>										
						<td> </td>										
						<td> </td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> S-CSCF Name </td>				
						<td><html:checkbox property="udr_scscf_name" styleClass="inputbox" />	</td>										
						<td></td>										
						<td><html:checkbox property="snr_scscf_name" styleClass="inputbox" />	</td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> iFC </td>				
						<td><html:checkbox property="udr_ifc" styleClass="inputbox" />	</td>										
						<td></td>										
						<td><html:checkbox property="snr_ifc" styleClass="inputbox" />	</td>										
					</tr>
					
					<tr bgcolor="#FFCC66">
						<td> Location </td>				
						<td><html:checkbox property="udr_location" styleClass="inputbox" />	</td>										
						<td></td>										
						<td></td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> User-State </td>				
						<td><html:checkbox property="udr_user_state" styleClass="inputbox" /> </td>										
						<td></td>										
						<td></td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> Charging-Info </td>				
						<td><html:checkbox property="udr_charging_info" styleClass="inputbox" /> </td>										
						<td></td>										
						<td></td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> MS-ISDN </td>				
						<td><html:checkbox property="udr_msisdn" styleClass="inputbox" /> </td>										
						<td></td>										
						<td></td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> PSI Activation </td>				
						<td><html:checkbox property="udr_psi_activation" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="pur_psi_activation" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="snr_psi_activation" styleClass="inputbox" />	</td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> DSAI </td>				
						<td><html:checkbox property="udr_dsai" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="pur_dsai" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="snr_dsai" styleClass="inputbox" />	</td>										
					</tr>
					<tr bgcolor="#FFCC66">
						<td> Aliases Rep Data </td>				
						<td><html:checkbox property="udr_aliases_rep_data" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="pur_aliases_rep_data" styleClass="inputbox" />	</td>										
						<td><html:checkbox property="snr_aliases_rep_data" styleClass="inputbox" />	</td>										
					</tr>
					</table>		
				</td>					
			</tr>
 			<tr>
 				<td>
					<table align="center">			
					<tr>
						<td align=center> <br/>
							<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
							<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2);"/> 
							<% if (id == -1){ %>
								<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3);"/> 
							<%}%>
							<% if (id != -1){ %>
								<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4);" 
									disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
							<%}%>									
						</td>
					</tr>
					</table>
				</td>
			</tr>		
			</table>		
		</td>
	</tr>
	<%
		if (id != -1){
	%>
	<tr>
		<td>
			<table>
			<tr>
				<td>
					<table>
					<tr>
						<td>	
							<b>Attach IFC </b>
						</td>
					</tr>					
					<tr>
						
						<td>
							<html:select property="ifc_id" name="AS_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:option value="-1">Select IFC...</html:option>
								<html:optionsCollection name="AS_Form" property="select_ifc" label="name" value="id"/>
							</html:select>	
						</td>

						<td>
							<html:button property="ifc_attach_button" value="Attach" onclick="add_action_for_form(12, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> IFC Name </td>
						<td class="header"> Detach </td>
					</tr>
					<%
						if (attached_ifc_list != null){
							Iterator it = attached_ifc_list.iterator();
							IFC ifc = null;
							int idx = 0;
							while (it.hasNext()){
								ifc = (IFC) it.next();
												
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  <%= ifc.getId() %></td>												
										
								<td>  
									<a href="/hss.web.console/IFC_Load.do?id=<%= ifc.getId() %>" > 
										<%= ifc.getName() %>
									</a>	
								</td>

								<td> 
									<input type="button" name="detach_ifc" 
										"value="Detach" onclick="add_action_for_form(5, <%= ifc.getId() %>);"/>													
								</td>
							</tr>											
					<%			
								idx++;												
								}
							}
					%>
					</table>
				</td>
			</tr>
		</td>
	</tr>
	<%
		}
	%>				
	</table>		
</html:form>
</body>
</html>
