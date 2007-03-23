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
<title>IMPI </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	String action=request.getParameter("action");
	System.out.println("\n\nThe action is:" + action);
	int id = Integer.parseInt(request.getParameter("id"));
	System.out.println("\n\n impi.jsp ID is:" + id);
	
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(a) {
	switch(a){
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
		case 10:
			document.IMPI_Form.nextAction.value="ppr";
			document.IMPI_Form.submit();			
			break;
		case 11:	
			document.IMPI_Form.nextAction.value="rtr";
			document.IMPI_Form.submit();			
			break;
		case 12:
			document.IMPI_Form.nextAction.value="add_impu";
			document.IMPI_Form.submit();			
			break;
		case 13:
			document.IMPI_Form.nextAction.value="delete";
			document.IMPI_Form.submit();			
			
	}

}
function add_action_for_form2(action, id) {
 if (action == "delete"){
	switch(id){
		case 13:
			document.IMPI_Form.nextAction.value="delete2";
			document.IMPI_Form.submit();			
			break;
	}
}
}

function disable_other_boxes(){
	if (document.IMPI_Form.all.checked){
		document.IMPI_Form.aka1.disabled=true;
		document.IMPI_Form.aka2.disabled=true;
		document.IMPI_Form.md5.disabled=true;
		document.IMPI_Form.early.disabled=true;
	}
	else{
		document.IMPI_Form.aka1.disabled=false;
		document.IMPI_Form.aka2.disabled=false;
		document.IMPI_Form.md5.disabled=false;
		document.IMPI_Form.early.disabled=false;
	}
}
</script>

</head>

<body>

	<!-- Print errors, if any -->
	<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
	
	<html:form action="/IMPI_Submit">
			<html:hidden property="nextAction" value=""/>
			
		<center> 

		<table>
			<tr>
				<td> <br/><br/> <h1> Private User Identity </h1> <br/><br/> </td>
			</tr>
		</table>
			
		<table>
			<tr><td>
			<table>
			
			<tr>
				<td>ID: </td>
				<td><html:text property="id" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> </td>
			</tr>
			<tr>
				<td>IMSU:</td>
				<td><html:select property="id_imsu" name="IMPI_Form" styleClass="inputtext" size="1" style="width:325px;">
					<html:option value="-1">Select IMSU...</html:option>
					<html:optionsCollection name="IMPI_Form" property="select_imsu" label="name" value="id"/>
				</html:select></td>	
			</tr>

			<tr>
				<td>Identity: </td>
				<td><html:text property="identity" styleClass="inputtext" style="width:325px;"/> </td>
			</tr>
			<tr>
				<td>Secret Key: </td>
				<td><html:text property="secretKey" styleClass="inputtext" style="width:325px;"/> </td>
			</tr>
			
			<!-- The Authentication Schemes types -->
			<tr>
				<td><br/>Authentication Scheme: </td>
			</tr>
			<tr>
				<td>Digest-AKAv1: </td>
				<td><html:checkbox property="aka1" styleClass="inputbox"/> </td>
			</tr>
			<tr>
				<td>Digest-AKAv2: </td>
				<td><html:checkbox property="aka2" styleClass="inputbox"/> </td>
			</tr>
			<tr>
				<td>Digest-MD5: </td>
				<td><html:checkbox property="md5" styleClass="inputbox"/> </td>
			</tr>
			<tr>
				<td>Early-IMS: </td>
				<td><html:checkbox property="early" styleClass="inputbox"/> </td>
			</tr>
			<tr>
				<td>All: </td>
				<td><html:checkbox property="all" styleClass="inputbox" onclick="disable_other_boxes();"/> </td>
			</tr>
			<tr><td> <br/> </td></tr>
	
			<tr>
				<td>AMF: </td>
				<td><html:text property="amf" styleClass="inputtext" style="width:100px;"/> </td>
			</tr>
			<tr>
				<td>OP: </td>
				<td><html:text property="op" styleClass="inputtext" style="width:325px;"/> </td>
			</tr>
			<tr>
				<td>SQN: </td>
				<td><html:text property="sqn" styleClass="inputtext"/> </td>
			</tr>
			<tr>
				<td>IP: </td>
				<td><html:text property="ip" styleClass="inputtext"/> </td>
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
				<html:button property="rtr_button" value="RTR" onclick="add_action_for_form(11);"/>
				</td>
			</tr>	
			<%
				}
			%>
			
		</table> </td>

		<logic:notEqual value="-1" property="id" name="IMPI_Form">		
		<td><table>
			<tr>
				<td>	
					Associate an IMPU:
				</td>
				<td>
					<html:text property="impu_identity" styleClass="inputtext" />
				</td>
				<td>
					<html:button property="impu_add_button" value="Add" onclick="add_action_for_form(12);"/>
				</td>
			</tr>	
		</table>
		</logic:notEqual>		

			
		<logic:notEqual value="-1" property="id" name="IMPI_Form">		
		<table class="as" width="400">
			<tr class="header">
				<td class="header"> ID: </td>			
				<td class="header"> IMPU Identity: </td>
				<td class="header"> Delete: </td>
			</tr>
		
			<logic:iterate name="IMPI_Form" property="associated_impu_set" id="impu"
				type="de.fhg.fokus.hss.db.model.IMPU" indexId="ix">
				<tr class="<%= ix.intValue()%2 == 0 ? "even" : "odd" %>">
					<td>
						<bean:write name="impu" property="id" />
					</td>
				
					<td>
						<bean:write name="impu" property="identity" />
					</td>

					<td>					
<!--						<form method="post" action="/hss.web.console/IMPI_Delete.do?action=delete_associated_impu&id_impu=<bean:write name="impu" property="id" />" target="content" 
							style="text-align: center">
							<input type="hidden" name="id" value="<bean:write name="IMPI_Form" property="id" />"> 
							<input type="image" src="/hss.web.console/images/progress_rem.gif">
						</form>
-->
				<html:button property="delete" value="delete" onclick="add_action_for_form2('delete', 13);"/>												
					</td>
				</tr>
			</logic:iterate>						
		</table>
		</logic:notEqual>
	</html:form>		
		</td></tr>
	  </table>
	  </center> 		
</body>
</html>
