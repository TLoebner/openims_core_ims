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
<title> Visited Networks </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(a) {
	switch(a){
		case 1:
			document.VN_Form.nextAction.value="save";
			document.VN_Form.submit();
			break;
		case 2:
			document.VN_Form.nextAction.value="reset";
			document.VN_Form.reset();
			break;
		case 3:
			document.VN_Form.nextAction.value="refresh";
			document.VN_Form.submit();			
			break;
		case 4:
			document.VN_Form.nextAction.value="delete";
			document.VN_Form.submit();
			break;
	}
}
</script>
</head>

<body>

	<table id="title-table" align="center" weight="100%" >
	<tr>
		<td align="center">
			<h1> Visited Networks Identifiers </h1> 			
			<br/><br/>			
		</td>
	<tr>	
		<td align="left">
			<!-- Print errors, if any -->
			<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
		</td>
	</tr>
	</table> <!-- title-table -->

	<html:form action="/VN_Submit">
		<html:hidden property="nextAction" value=""/>

			<table id="main-table" align="center" valign="middle">
			<tr>
				<td>
			 		<table id="vn-table" border="0" align="center" width="400" >						
			 		<tr>
			 				<td>
						 		<table id="fields-table" class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
								<tr bgcolor="#FFCC66">
									<td> ID </td>
									<td>
										<html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> 
									</td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td>Identity* </td>
									<td>
										<html:text property="identity" styleClass="inputtext" style="width:200px;"/> 
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
									<td align=center> 
										<br />
										<%
											 if (request.isUserInRole(WebConstants.Security_Permission_ADMIN)){
										%>									
												<html:button property="save_button" value="Save" onclick="add_action_for_form(1, -1);"/>					
										<%
											}
										%>									
										<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3, -1);"/> 
										<% 
											if (request.isUserInRole(WebConstants.Security_Permission_ADMIN) && id == -1){ 
										%>
											<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2, -1);"/> 
										<%
											}
										%>
						
										<% 
											if (request.isUserInRole(WebConstants.Security_Permission_ADMIN) && id != -1){ 
										%>
										<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4, -1);" 
											disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
										<%
											}
										%>												
									</td>
								</tr>
							</table> <!-- button-tables -->	
						</td>
					</tr>
				</table> <!-- vn-table -->
			</td>
		</tr>
		</table>	<!-- main-table-->		
	</html:form>
</body>
</html>
