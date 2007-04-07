<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="java.util.*, de.fhg.fokus.hss.util.SecurityPermissions, de.fhg.fokus.hss.db.model.* " %>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title>IMPU </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
<jsp:useBean id="associated_IMPIs" class="java.util.ArrayList" scope="request"></jsp:useBean>
<jsp:useBean id="implicitset_IMPUs" class="java.util.ArrayList" scope="request"></jsp:useBean>
<jsp:useBean id="visitedNetworks" class="java.util.ArrayList" scope="request"></jsp:useBean>

<%
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(action, associated_ID) {

	switch(action){
		case 1:
			document.IMPU_Form.nextAction.value="save";
			document.IMPU_Form.submit();
			break;
		case 2:
			document.IMPU_Form.nextAction.value="reset";
			document.IMPU_Form.reset();
			break;
		case 3:
			document.IMPU_Form.nextAction.value="refresh";
			document.IMPU_Form.submit();			
			break;
		case 4:
			document.IMPU_Form.nextAction.value="delete";
			document.IMPU_Form.submit();			
			break;
		case 5:
			document.IMPU_Form.nextAction.value="delete_associated_impi";
			document.IMPU_Form.associated_ID.value = associated_ID;
			document.IMPU_Form.submit();			
			break;
		case 6:
			document.IMPU_Form.nextAction.value="delete_impu_from_implicitset";
			document.IMPU_Form.associated_ID.value = associated_ID;			
			document.IMPU_Form.submit();			
			break;
		case 7:
			document.IMPU_Form.nextAction.value="delete_visited_network";
			document.IMPU_Form.associated_ID.value = associated_ID;			
			document.IMPU_Form.submit();			
			break;
								
		case 10:
			document.IMPU_Form.nextAction.value="ppr";
			document.IMPU_Form.submit();			
			break;
		case 11:	
			document.IMPU_Form.nextAction.value="rtr";
			document.IMPU_Form.submit();			
			break;
			
		case 12:
			document.IMPU_Form.nextAction.value="add_impi";
			document.IMPU_Form.submit();			
			break;
		case 13:
			document.IMPU_Form.nextAction.value="add_impu_to_implicitset";
			document.IMPU_Form.submit();			
			break;
		case 14:
			document.IMPU_Form.nextAction.value="add_vn";
			document.IMPU_Form.submit();			
			break;
			
	}

}

</script>
</head>

<body>
	<table align="center" valign="middle" height="100%">
		<!-- Print errors, if any -->
		<tr>
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>	

		<tr>
			<td align="center"> <h1> Public User Identity -IMPU-</h1> </td>
		</tr>
		
		<html:form action="/IMPU_Submit">
			<html:hidden property="nextAction" value=""/>
			<html:hidden property="associated_ID" />
			<html:hidden property="already_assigned_impi_id" />			
			
			<tr>
				<td>
		 			<table border="0" align="center" width="100%" >						
		 			<tr>
 						<td>
			 				<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">										
							<tr bgcolor="#FFCC66">
								<td> ID </td>
								<td><html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> </td>
							</tr>
		
							<tr bgcolor="#FFCC66">
								<td>Identity </td>
								<td><html:text property="identity" styleClass="inputtext" style="width:250px;"/> </td>
							</tr>
							<tr bgcolor="#FFCC66">
								<td>Barring</td>
								<td><html:checkbox property="barring" styleClass="inputbox" /></td>
							</tr>
			
							<tr bgcolor="#FFCC66">
								<td>Service Profile</td>
								<td>
									<html:select property="id_sp" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;">
										<html:option value="-1">Select Profile...</html:option>
										<html:optionsCollection name="IMPU_Form" property="select_sp" label="name" value="id"/>
									</html:select>
								</td>	
							</tr>
			
							<tr bgcolor="#FFCC66">
								<td>Implicit Set:</td>
								<td>
									<html:text property="id_impu_implicitset" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/>
								</td>
							</tr>			
							<tr bgcolor="#FFCC66">
								<td>Charging-Info Set:</td>
						
								<td>
									<html:select property="id_charging_info" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;">
										<html:option value="-1">Select Charging-Info...</html:option>
										<html:optionsCollection name="IMPU_Form" property="select_charging_info" label="name" value="id"/>
									</html:select>	
								</td>
							</tr>
			
							<tr bgcolor="#FFCC66">
								<td> Can Register </td>
								<td><html:checkbox property="can_register" styleClass="inputbox"/> </td>
							</tr>			

							<tr bgcolor="#FFCC66">
								<td> IMPU Type </td>
								<td>
									<html:select property="type" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;" > 
										<html:option value="-1">Select type...</html:option>
										<html:optionsCollection name="IMPU_Form" property="select_identity_type" label="name" value="code"/>
									</html:select>
								</td>	
							</tr>			
			
							<tr bgcolor="#FFCC66">
								<td> Wildcard PSI </td>
								<td><html:text property="wildcard_psi" styleClass="inputtext" style="width:250px;"/> </td>				
							</tr>			
			
							<tr bgcolor="#FFCC66">
								<td> PSI Activation </td>
								<td><html:checkbox property="psi_activation" styleClass="inputbox"/> </td>				
							</tr>			
			
							<tr bgcolor="#FFCC66">
								<td> Display Name </td>
								<td><html:text property="display_name" styleClass="inputtext" styleClass="inputtext" style="width:250px;"/> </td>							
							</tr>			
			
							<tr bgcolor="#FFCC66">
								<td> User-Status </td>
								<html:hidden property="user_state" />

								<logic:equal value="0" property="user_state" name="IMPU_Form">
									<td bgcolor="#FF0000"> NOT-REGISTERED </td>
								</logic:equal>

								<logic:equal value="1" property="user_state" name="IMPU_Form">
									<td bgcolor="#00FF00"> REGISTERED </td>
								</logic:equal>
				
								<logic:equal value="2" property="user_state" name="IMPU_Form">
									<td bgcolor="#0000FF"> UN-REGISTERED </td>
								</logic:equal>
				
								<logic:equal value="3" property="user_state" name="IMPU_Form">
									<td bgcolor="#FFFF00"> AUTH-PENDING </td>
								</logic:equal>
							</tr>
							</table>		
					</td>
				</tr>				
				<tr>
					<td>
						<table align="center">			
						<tr>
							<td align=center> <br/>
								<html:button property="save_button" value="Save" onclick="add_action_for_form(1, -1);"/>				
								<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3, -1);"/> 
								<% if (id == -1){ %>
									<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2, -1);"/> 
								<%}%>
									<% if (id != -1){ %>
									<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4, -1);" 
										disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
									<%}%>												
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
						<br/>
				 		<table align="center" width="100%" >										
						<tr>
							<td>
								<table>
								<tr>
									<td>	
										<b>Add IMPUs to Implicit-Set </b>
									</td>
									<td>
										<html:text property="impu_implicitset_identity" value="" styleClass="inputtext" />
									</td>
									<td>
										<html:button property="add_impu_to_implicitset" value="Add" onclick="add_action_for_form(13, -1);"/>
									</td>
								</tr>	
								</table>
							</td>
						</tr>	
						<tr>
							<td>
								<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
								<tr class="header">
									<td class="header"> ID </td>			
									<td class="header"> IMPU Identity </td>
									<td class="header"> Delete </td>
								</tr>
							<%
								if (implicitset_IMPUs != null && implicitset_IMPUs.size() > 0){
									Iterator it = implicitset_IMPUs.iterator();
									IMPU crt_IMPU = null;
								
									while (it.hasNext()){									
										crt_IMPU = (IMPU) it.next();
							%>
										<tr class="even">																			
											<td>  <%= crt_IMPU.getId() %></td>
											
											<td>  
												<a href="/hss.web.console/IMPU_Load.do?id=<%= crt_IMPU.getId() %>" > 
													<%= crt_IMPU.getIdentity() %>
												</a>	
											</td>
											
							<%
							
											if (crt_IMPU.getId() != id){
							%>				
												<td> 
													<input type="button" name="delete_impu_from_implicitset" 
														"value="Delete" onclick="add_action_for_form(6, <%= crt_IMPU.getId() %>);" />													
												</td>
							<%
											} else {
							%>				
												<td> </td> 
							<%
											}
							%>											
										</tr>											
							<%			
									}
								}
							%>
								</table>
							</td>
						</tr>							
						</table>		
				</td>
			</tr>
			</table>		
		</td>	
		
		<td>	

		
			<table align="center" valign="middle" width="100%" >										
			<tr>
				<td>
					<table>
					<tr>
						<td>	
							<b>Add Visited-Networks </b>
						</td>

						<td>
							<html:select property="vn_id" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:option value="-1">Select Visited-Network...</html:option>
								<html:optionsCollection name="IMPU_Form" property="select_vn" label="identity" value="id"/>
							</html:select>	
						</td>
						
						<td>
							<html:button property="add_vn" value="Add" onclick="add_action_for_form(14, -1);"/>
						</td>
					</tr>	
					</table>
				</td>
			</tr>	
			<tr>
				<td>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> Identity </td>
						<td class="header"> Delete </td>
					</tr>
					<%
						if (visitedNetworks != null && visitedNetworks.size() > 0){
							Iterator it = visitedNetworks.iterator();
							VisitedNetwork crt_VisitedNetwork = null;
										
							while (it.hasNext()){									
								crt_VisitedNetwork = (VisitedNetwork) it.next();
					%>
								<tr class="even">																			
									<td>  <%= crt_VisitedNetwork.getId() %></td>

									<td> 
								    	<a href="/hss.web.console/VN_Load.do?id=<%= crt_VisitedNetwork.getId() %>" > 
											 <%= crt_VisitedNetwork.getIdentity() %></td>
										</a>
									</td>	
									<td> 
										<input type="button" name="delete_visited_network" 
											"value="Delete" onclick="add_action_for_form(7, <%= crt_VisitedNetwork.getId() %>);"/>													
									</td>
								</tr>											
					<%			
							}
						}
					%>
					</table>
				</td>
			</tr>		
		
			<table align="center" valign="middle" width="100%" >										
			<tr>
				<td>
					<table>
					<tr>
						<td>
							<br/>	
							<b>Associate IMPIs </b>
						</td>
						<td>
							<html:text property="impi_identity" value="" styleClass="inputtext" />
						</td>
						<td>
							<html:button property="add_impi" value="Add" onclick="add_action_for_form(12, -1);"/>
						</td>
					</tr>	
					</table>
				</td>
			</tr>	
			<tr>
				<td>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> IMPI Identity </td>
						<td class="header"> Delete </td>
					</tr>
					<%
						if (associated_IMPIs != null && associated_IMPIs.size() > 0){
							Iterator it = associated_IMPIs.iterator();
							IMPI crt_IMPI = null;
										
							while (it.hasNext()){									
								crt_IMPI = (IMPI) it.next();
					%>
								<tr class="even">																			
									<td>  <%= crt_IMPI.getId() %></td>
									<td>
								    	<a href="/hss.web.console/IMPI_Load.do?id=<%= crt_IMPI.getId() %>" > 
  											<%= crt_IMPI.getIdentity() %>
  										</a>  
									</td>

									<td> 
										<input type="button" name="delete_associated_impi" 
											"value="Delete" onclick="add_action_for_form(5, <%= crt_IMPI.getId() %>);"/>													
									</td>
								</tr>											
					<%			
							}
						}
					%>
					</table>
				</td>
			</tr>		
					
			<tr>
				<td>
					<br>
				</td>
			</tr>
	
			<tr>
				<td>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr align="center">
						<td align="center"> <b> Push Cx Operation </b></td>
					</tr>
					<tr align="center">
						<td align="center">
							<html:button property="ppr_button" value="PPR" onclick="add_action_for_form(10, -1);"/>
							<html:button property="rtr_button" value="RTR" onclick="add_action_for_form(11, -1);"/>
						</td>
					</tr>	
					</table>				
				</td>
			</tr>
							
			<%
				}
			%>
			</table>			
		</td>
	</tr>
	</html:form>
</table>		
</body>
</html>
