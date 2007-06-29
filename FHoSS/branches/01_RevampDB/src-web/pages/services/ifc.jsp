<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.web.util.WebConstants " %>
<html>

<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> AS </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
<%
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(a) {
	switch(a){
		case 1:
			document.IFC_Form.nextAction.value="save";
			document.IFC_Form.submit();
			break;
		case 2:
			document.IFC_Form.nextAction.value="refresh";
			document.IFC_Form.submit();			
			break;
		case 3:
			document.IFC_Form.nextAction.value="reset";
			document.IFC_Form.reset();
			break;
		case 4:
			document.IFC_Form.nextAction.value="delete";
			document.IFC_Form.submit();
			break;			
	}
}
</script>
</head>

<body>
	<table id="title-table" align="center" weight="100%" >
	<tr>
		<td align="center">
			<h1> Initial Filter Criteria -iFC-</h1> 			
			<br/><br/>			
		</td>
	<tr>	
		<td align="left">
			<!-- Print errors, if any -->
			<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
		</td>
	</tr>
	</table> <!-- title-table -->

<html:form action="/IFC_Submit">
	<html:hidden property="nextAction" value=""/>
	<html:hidden property="associated_ID" value=""/>
		
	<table id="main-table" align="center" valign="middle" >
	<tr>
		<td>
	 		<table id="top-side-table" border="0" align="center" >						
 			<tr>
 				<td>
					<table id="ifc-table" border="0" cellspacing="1" align="center" width="400" style="border:2px solid #FF6600;">						
					<tr bgcolor="#FFCC66">
						<td> ID </td>
						<td><html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Name* </td>
						<td><html:text property="name" styleClass="inputtext" style="width:200px;"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>
							Trigger Point
						</td>	
						<td> 		
							<html:select property="id_tp" styleClass="inputtext" size="1" style="width:200px;">
								<html:option value="-1">Select TP...</html:option>					
								<html:optionsCollection property="select_tp" label="name" value="id"/>
							</html:select>
						</td>
					</tr>

					<tr bgcolor="#FFCC66">
						<td>
							Application Server*
						</td>	
						<td> 			
							<html:select property="id_application_server" styleClass="inputtext" size="1" style="width:200px;">
								<html:option value="-1">Select AS...</html:option>					
								<html:optionsCollection property="select_as" label="name" value="id"/>
							</html:select>
						</td>
					</tr>
						
					<tr bgcolor="#FFCC66">
						<td> 
							Profile Part Indicator 
						</td>
						<td>
							<html:select property="profile_part_ind" styleClass="inputtext" size="1" style="width:200px;">
								<html:optionsCollection property="select_profile_part_indicator" label="name" value="code"/>
							</html:select>
						</td>
					</tr>
					</table>
				</td>
			</tr>
 			<tr>
 				<td>
					<table id="buttons-table" align="center">			
					<tr>
						<td align="center"> 
							<b> Mandatory fields were marked with "*" </b>
						</td>
					</tr>						
					<tr>
						<td align=center> <br/>
							<%
								 if (request.isUserInRole(WebConstants.Security_Permission_ADMIN)){
							%>						
							<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
							<%
								}
							%>
							<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2);"/> 
							<% 
								if (request.isUserInRole(WebConstants.Security_Permission_ADMIN) && id == -1){ 
							%>
								<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3);"/> 
							<%
								}
							%>
							<% 
								if (request.isUserInRole(WebConstants.Security_Permission_ADMIN) && id != -1){ 
							%>
								<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4);" 
									disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
							<%
								}
							%>									
						</td>
					</tr>
					</table> <!-- buttons-table -->
				</td>
			</tr>							
			</table> <!-- top-side-table-->		
		</td>
	</tr>
	</table> <!-- main-table -->		
</html:form>
</body>
</html>
