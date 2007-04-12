<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Capability </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(a) {
	switch(a){
		case 1:
			document.Cap_Form.nextAction.value="save";
			document.Cap_Form.submit();
			break;
		case 2:
			document.Cap_Form.nextAction.value="refresh";
			document.Cap_Form.submit();			
			break;
		case 3:
			document.Cap_Form.nextAction.value="reset";
			document.Cap_Form.reset();
			break;
		case 4:
			document.Cap_Form.nextAction.value="delete";
			document.Cap_Form.submit();
			break;
			
	}
}
</script>
</head>

<body>
	<table align=center valign=middle height=80%>
		<!-- Print errors, if any -->
		<tr>
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>
		
		<html:form action="/Cap_Submit">
			<html:hidden property="nextAction" value=""/>
			<tr>
				<td align="center"><h1>Capabilities</h1></td>
			</tr>
			<tr>
				<td>
			 		<table border="0" align="center" width="100%" >						
			 		<tr>
			 				<td>
						 		<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
								<tr bgcolor="#FFCC66">
									<td> ID </td>
									<td>
										<html:text property="id" styleClass="inputtext"/> 
									</td>
								</tr>
								<tr bgcolor="#FFCC66">
									<td> Name </td>
									<td>
										<html:text property="name" styleClass="inputtext"/> 
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
		</tr>
		</table>			
	</html:form>
</body>
</html>
