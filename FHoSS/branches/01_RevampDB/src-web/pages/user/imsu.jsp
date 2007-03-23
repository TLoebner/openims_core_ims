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
<title>IMSU </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	String action=request.getParameter("action");
	int id = Integer.parseInt(request.getParameter("id"));
	
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(a) {
	switch(a){
		case 1:
			document.IMSU_Form.nextAction.value="save";
			document.IMSU_Form.submit();
			break;
		case 2:
			document.IMSU_Form.nextAction.value="reset";
			document.IMSU_Form.reset();
			break;
		case 3:
			document.IMSU_Form.nextAction.value="refresh";
			document.IMSU_Form.submit();			
			break;
		case 4:
			document.IMSU_Form.nextAction.value="delete";
			document.IMSU_Form.submit();			
			break;
	}

}

</script>

</head>

<body>

	<!-- Print errors, if any -->
	<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
	
	<html:form action="/IMSU_Submit">
			<html:hidden property="nextAction" value=""/>
			
		<center> 

		<table>
			<tr>
				<td> <br/><br/> <h1> IMS Subscription </h1> <br/><br/> </td>
			</tr>
		</table>
			
		<table>
			<tr>
				<td>ID: </td>
				<td><html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> </td>
			</tr>
			
			<tr>
				<td>Name: </td>
				<td><html:text property="name" styleClass="inputtext" style="width:325px;"/> </td>
			</tr>
						
			<tr>
				<td>Capabilities Set:</td>
				<td><html:select property="id_capabilities_set" name="IMSU_Form" styleClass="inputtext" size="1" style="width:325px;">
					<html:option value="-1">none</html:option>
					<html:optionsCollection name="IMSU_Form" property="select_capabilities_set" label="name" value="id"/>
				</html:select></td>	
			</tr>

			<tr>
				<td>Preferred S-CSCF:</td>
				<td><html:select property="id_preferred_scscf" name="IMSU_Form" styleClass="inputtext" size="1" style="width:325px;">
					<html:option value="-1">none</html:option>
					<html:optionsCollection name="IMSU_Form" property="select_preferred_scscf" label="name" value="id"/>
				</html:select></td>	
			</tr>
			
			<tr>
				<td>S-CSCF Name: </td>
				<td><html:text property="scscf_name" readonly="true" styleClass="inputtext_readonly" style="width:325px;"/> </td>
			</tr>
			
			<tr>
				<td>Diameter Name: </td>
				<td><html:text property="diameter_name" readonly="true" styleClass="inputtext_readonly" style="width:325px;"/> </td>
			</tr>
			
		</table>		
			
		<table>			
			<tr><td align=center> <br/>
				<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
				<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3);"/> 
				
				<% if (id == -1){ %>
					<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2);"/> 
				<%}%>

				<% if (id != -1){ %>
				<html:button property="delete_button" value="Delete" onclick="add_action_for_form(4);" 
					disabled="<%=Boolean.parseBoolean((String)request.getAttribute("deleteDeactivation")) %>"/>				
				<%}%>					
				
			</td></tr>
		</table>			
	  </center> 		

	</html:form>
	
</body>
</html>
