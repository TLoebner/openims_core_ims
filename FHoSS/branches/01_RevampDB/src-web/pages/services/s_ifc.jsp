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
<title> Shared iFC </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	String action=request.getParameter("action");
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(a) {
	switch(a){
		case 1:
			document.S_IFC_Form.nextAction.value="save";
			document.S_IFC_Form.submit();
			break;
		case 2:
			document.S_IFC_Form.nextAction.value="refresh";
			document.S_IFC_Form.submit();			
			break;
		case 3:
			document.S_IFC_Form.nextAction.value="reset";
			document.S_IFC_Form.reset();
			break;
		case 4:
			document.S_IFC_Form.nextAction.value="delete";
			document.S_IFC_Form.submit();			
			break;
			
	}
}
</script>
</head>

<body>

	<!-- Print errors, if any -->
	<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
	
	<html:form action="/S_IFC_Submit">
			<html:hidden property="nextAction" value=""/>
		<center> 

		<table>
			<tr>
				<td> <br/><br/> <h1> Shared iFC Sets </h1> <br/><br/> </td>
			</tr>
		</table>
			
		<table>
			<tr>
				<td> ID </td>
				<td><html:text property="id" readonly="true" styleClass="inputtext_readonly"/> </td>
			</tr>
			<tr>
				<td>Name </td>
				<td><html:text property="name" styleClass="inputtext"/> </td>
			</tr>

			<tr>
				<td>Priority </td>
				<td><html:text property="priority" styleClass="inputtext"/> </td>
			</tr>

		</table>		
			
		<table>			
			<tr><td align=center> <br/>
				<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
				<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2);"/> 
				<% if (id == -1){ %>
					<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3);"/> 
				<%}%>
				<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4);" 
					disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
				
			</td></tr>
		</table>			
	  </center> 		
	</html:form>
</body>
</html>
