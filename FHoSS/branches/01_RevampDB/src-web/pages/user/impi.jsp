<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions, java.util.*, de.fhg.fokus.hss.db.model.*" %>

<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title>IMPI </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<jsp:useBean id="associated_IMSU" class="de.fhg.fokus.hss.db.model.IMSU" scope="request"></jsp:useBean>
<jsp:useBean id="associated_IMPUs" type="java.util.List" scope="request"></jsp:useBean>

<%
	int id = Integer.parseInt(request.getParameter("id"));
	request.setAttribute("gigi", new Integer(12));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(action, associated_ID) {
	switch(action){
		case 1:
			document.IMPI_Form.nextAction.value="save";
			document.IMPI_Form.submit();
			break;
		case 2:
			document.IMPI_Form.nextAction.value="reset";
			document.IMPI_Form.reset();
			break;
		case 3:
			document.IMPI_Form.nextAction.value="refresh";
			document.IMPI_Form.submit();			
			break;
		case 4:
			document.IMPI_Form.nextAction.value="delete";
			document.IMPI_Form.submit();			
			break;		
		case 5:
			document.IMPI_Form.nextAction.value="delete_associated_IMPU";
		 	document.IMPI_Form.associated_ID.value=associated_ID;
		 	document.IMPI_Form.submit();
			break;

		case 6:
			document.IMPI_Form.nextAction.value="delete_associated_IMSU";
		 	document.IMPI_Form.submit();
			break;
					
		case 10:
			document.IMPI_Form.nextAction.value="ppr";
			document.IMPI_Form.submit();			
			break;
		case 11:	
			document.IMPI_Form.nextAction.value="rtr";
			document.IMPI_Form.submit();			
			break;
		case 12:
			document.IMPI_Form.nextAction.value="add_imsu";
			document.IMPI_Form.submit();			
			break;
		case 13:
			document.IMPI_Form.nextAction.value="add_impu";
			document.IMPI_Form.submit();			
			
	}

}

function disable_other_boxes(){
	if (document.IMPI_Form.all.checked){
		document.IMPI_Form.aka1.disabled=true;
		document.IMPI_Form.aka2.disabled=true;
		document.IMPI_Form.md5.disabled=true;
		document.IMPI_Form.digest.disabled=true;
		document.IMPI_Form.http_digest.disabled=true;		
		document.IMPI_Form.early.disabled=true;
		document.IMPI_Form.nass_bundle.disabled=true;		
		
	}
	else{
		document.IMPI_Form.aka1.disabled=false;
		document.IMPI_Form.aka2.disabled=false;
		document.IMPI_Form.md5.disabled=false;
		document.IMPI_Form.digest.disabled=false;
		document.IMPI_Form.http_digest.disabled=false;		
		document.IMPI_Form.early.disabled=false;
		document.IMPI_Form.nass_bundle.disabled=false;		
	}
}
</script>

</head>

<body>
	<table align=center valign=middle height=100%>
		<!-- Print errors, if any -->
		<tr>
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>	
		
		<html:form action="/IMPI_Submit">
			<html:hidden property="nextAction" value=""/>
			<html:hidden property="associated_ID" value=""/>			
			<html:hidden property="id_imsu" />			
			<html:hidden property="already_assigned_imsu_id" />						
			<tr>
				<td align="center"><h1> Private User Identity -IMPI- </h1></td>
			</tr>
			<tr>
				<td>
			 		<table border="0" align="center" width="100%" >						
			 			<tr>
			 				<td>
						 		<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
								    <tr bgcolor="#FFCC66">
										<td>ID </td>
										<td>
											<html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Identity 
										</td>
										<td>
											<html:text property="identity" styleClass="inputtext" style="width:325px;"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Secret Key
										</td>
										<td>
											<html:text property="secretKey" styleClass="inputtext" style="width:325px;"/> 
										</td>
									</tr>
			
									<!-- The Authentication Schemes types -->
									<tr bgcolor="#FFCC66">
										<td> 
											<br/> <b>Authentication Schemes </b>
										</td>
										<td></td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Digest-AKAv1
										</td>
										<td>
											<html:checkbox property="aka1" styleClass="inputbox"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Digest-AKAv2
										</td>
										<td>
											<html:checkbox property="aka2" styleClass="inputbox"/>
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Digest-MD5
										</td>
										<td>
											<html:checkbox property="md5" styleClass="inputbox"/> 
										</td>
									</tr>

									<tr bgcolor="#FFCC66">
										<td>
											Digest
										</td>
										<td>
											<html:checkbox property="digest" styleClass="inputbox"/> 
										</td>
									</tr>

									<tr bgcolor="#FFCC66">
										<td>
											HTTP Digest
										</td>
										<td>
											<html:checkbox property="http_digest" styleClass="inputbox"/> 
										</td>
									</tr>
									
									<tr bgcolor="#FFCC66">
										<td>
											Early-IMS 
										</td>
										<td>
											<html:checkbox property="early" styleClass="inputbox"/> 
										</td>
									</tr>

									<tr bgcolor="#FFCC66">
										<td>
											NASS Bundle
										</td>
										<td>
											<html:checkbox property="nass_bundle" styleClass="inputbox"/> 
										</td>
									</tr>
									
									<tr bgcolor="#FFCC66">
										<td>
											All
										</td>
										<td>
											<html:checkbox property="all" styleClass="inputbox" onclick="disable_other_boxes();"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											Default
										</td>
										<td>
											<html:select property="default_auth_scheme" name="IMPI_Form" styleClass="inputtext" size="1" style="width:250px;" > 
												<html:optionsCollection name="IMPI_Form" property="select_auth_scheme" label="name" value="code"/>
											</html:select>
										</td>	
									</tr>
									
									<tr bgcolor="#FFCC66">
										<td> <br/> </td>
										<td> </td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											AMF
										</td>
										<td>
											<html:text property="amf" styleClass="inputtext" size="4"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											OP
										</td>
										<td>
											<html:text property="op" styleClass="inputtext" size="34"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td> 
											SQN
										</td>
										<td>
											<html:text property="sqn" styleClass="inputtext" size="12"/>
										</td>
									</tr>

									<tr bgcolor="#FFCC66">
										<td> <br/> </td>
										<td> </td>
									</tr>
									
									<tr bgcolor="#FFCC66">
										<td>
											Early IMS IP
										</td>
										<td>
											<html:text property="ip" styleClass="inputtext"/> 
										</td>
									</tr>
									<tr bgcolor="#FFCC66">
										<td>
											DSL Line Identifier
										</td>
										<td>
											<html:text property="line_identifier" styleClass="inputtext"/> 
										</td>
									</tr>
							</table>		
						</td>
					</tr>
					
					<tr>
						<td>
							<table align="center">
								<tr>
									<td align=center> 
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
				</table>
			</td>
			<td>
				<table>
					<logic:notEqual value="-1" property="id" name="IMPI_Form">		
					<tr>
						<td>
							<table>
								<tr>
									<td>	
										<b>Associate an IMSU </b>
									</td>
									<td>
										<html:text property="imsu_name" value="" styleClass="inputtext" />
									</td>
									<td>
										<html:button property="imsu_add_button" value="Add/Change" onclick="add_action_for_form(12, -1);"/>
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
									<td class="header"> IMSU Identity </td>
									<td class="header"> Delete </td>
								</tr>
								<%
									if (associated_IMSU != null && associated_IMSU.getId() > 0){
								%>
										<tr class="even">																			
											<td>  <%= associated_IMSU.getId() %></td>
											<td>  
												<a href="/hss.web.console/IMSU_Load.do?id=<%= associated_IMSU.getId() %>" > 
													<%= associated_IMSU.getName() %>
												</a>	
											</td>
											<td> 
												<input type="button" name="delete_associated_imsu" 
													"value="Delete" onclick="add_action_for_form(6, <%= associated_IMSU.getId() %>);"/>													
											</td>
										</tr>											
								<%			
									}
								%>
							</table>
						</td>
					</tr>		

					<tr>
						<td>
							<br/>
						</td>	
					</tr>
					<tr>
						<td>
							<table>
								<tr>
									<td>	
										<b>Create & Bind new IMPU </b>
									</td>
									<td>
										<%
											out.println("<a href=\"/hss.web.console/IMPU_Load.do?id=-1&already_assigned_impi_id=" + id + "\" > ");
										%>
											<img src="/hss.web.console/images/add_obj.gif" /> 
										</a>
									</td>
								</tr>	
							</table>
						</td>
					</tr>	

					
					<tr>
						<td>
							<br/>
						</td>	
					</tr>	
					<tr>
						<td>
							<table>
								<tr>
									<td>	
										<b>Associate IMPUs </b>
									</td>
									<td>
										<html:text property="impu_identity" value="" styleClass="inputtext" />
									</td>
									<td>
										<html:button property="impu_add_button" value="Add" onclick="add_action_for_form(13);"/>
									</td>
								</tr>	
							</table>
						</td>
					</tr>	
						
					<tr>
						<td>
							<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
								<tr class="header">
									<td class="header"> ID: </td>			
									<td class="header"> IMPU Identity: </td>
									<td class="header"> Delete: </td>
								</tr>
								<%
									if (associated_IMPUs != null){
										Iterator it = associated_IMPUs.iterator();
										IMPU impu = null;
										int idx = 0;
										while (it.hasNext()){
											impu = (IMPU) it.next();
												
								%>
											<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
												<td>  <%= impu.getId() %></td>												
												
												<td>  
													<a href="/hss.web.console/IMPU_Load.do?id=<%= impu.getId() %>" > 
														<%= impu.getIdentity() %>
													</a>	
												</td>
												<td> 
													<input type="button" name="delete_associated_impu" 
														"value="Delete" onclick="add_action_for_form(5, <%= impu.getId() %>);"/>													
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
					</logic:notEqual>
				</table>
			</td>
		</tr>					     		
		</html:form>		
	</table>	
</body>
</html>
