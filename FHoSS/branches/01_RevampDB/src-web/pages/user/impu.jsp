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
<title>IMPU </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	String action=request.getParameter("action");
	System.out.println("\n\nThe action is:" + action);
	int id = Integer.parseInt(request.getParameter("id"));
	System.out.println("\n\n impu.jsp ID is:" + id);
	
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(a) {
	switch(a){
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
		case 10:
			document.IMPU_Form.nextAction.value="ppr";
			document.IMPU_Form.submit();			
			break;
		case 11:	
			document.IMPU_Form.nextAction.value="rtr";
			document.IMPU_Form.submit();			
			break;
	}

}

</script>
</head>

<body>

	<!-- Print errors, if any -->
	<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
	
	<html:form action="/IMPU_Submit">
			<html:hidden property="nextAction" value=""/>
			
		<center> 

		<table>
			<tr>
				<td> <br/><br/> <h1> Public User Identity </h1> <br/><br/> </td>
			</tr>
		</table>
			
		<table>
			<tr>
				<td>ID: </td>
				<td><html:text property="id" readonly="true" styleClass="inputtext_readonly"/> </td>
			</tr>
			<tr>
				<td>Identity: </td>
				<td><html:text property="identity" styleClass="inputtext"/> </td>
			</tr>
			<tr>
				<td>Barring:</td>
				<td><html:checkbox property="barring" styleClass="inputbox" /></td>
			</tr>
			
			<tr>
				<td>Service Profile:</td>
				<td><html:select property="id_sp" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;">
					<html:option value="-1">Select Profile...</html:option>
					<html:optionsCollection name="IMPU_Form" property="select_sp" label="name" value="id"/>
				</html:select></td>	
			</tr>
			
			<tr>
				<td>Implicit Set:</td>
				<td>
			<%
				if (id == -1){
			%>
					<html:text property="id_impu_implicitset" readonly="true" styleClass="inputtext_readonly"/>
			<%
				}
				else{
			%>
					<html:text property="id_impu_implicitset" styleClass="inputtext"/>
			<%	
				}
			%>			
					
				</td>
			</tr>
			<tr>	
				<td>Charging-Info Set:</td>
				<td><html:select property="id_charging_info" name="IMPU_Form" styleClass="inputtext" size="1" style="width:250px;">
					<html:option value="-1">Select Charging-Info...</html:option>
					<html:optionsCollection name="IMPU_Form" property="select_charging_info" label="name" value="id"/>
				</html:select>	
				</td>
			</tr>
			
			<tr>
				<td>User-Status:</td>
				<html:hidden property="user_state" />

				<logic:equal value="1" property="user_state" name="IMPU_Form">
					<td bgcolor="#00FF00"> REGISTERED </td>
				</logic:equal>
				
				<logic:equal value="0" property="user_state" name="IMPU_Form">
					<td bgcolor="#FF0000"> NOT-REGISTERED </td>
				</logic:equal>
				
				<logic:equal value="-1" property="user_state" name="IMPU_Form">
					<td bgcolor="#0000FF"> UN-REGISTERED </td>
				</logic:equal>
				
				<logic:equal value="2" property="user_state" name="IMPU_Form">
					<td bgcolor="#FFFF00"> PENDING </td>
				</logic:equal>
			</tr>
		</table>		
			
		<table>			
			<tr><td align=center> <br/>
				<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
				<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3);"/> 
				<% if (id == -1){ %>
					<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2);"/> 
				<%}%>
				
			</td></tr>
			
			<%
				if (id != -1){
			%>
			
			<tr><td><br/> Cx Operations:</td><td>&nbsp;</td></tr>
			<tr>
				<td align="center">
				<html:button property="ppr_button" value="PPR" onclick="add_action_for_form(10);"/>
				<html:button property="rtr_button" value="RTR" onclick="add_action_for_form(11);"/>
				</td>
			</tr>	
			<%
				}
			%>
			
		</table>			
	  </center> 		

	</html:form>
	
	
</body>
</html>
