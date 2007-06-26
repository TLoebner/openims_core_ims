<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="java.util.*, de.fhg.fokus.hss.db.model.*, de.fhg.fokus.hss.db.op.*, de.fhg.fokus.hss.web.util.*,
	de.fhg.fokus.hss.db.hibernate.*, org.hibernate.Session " %>

<html>
<head>
<jsp:useBean id="attached_ifc" type="java.util.List" scope="request"></jsp:useBean>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Shared iFC </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">

<%
	int id_set = Integer.parseInt(request.getParameter("id_set"));
	Session hibSession = HibernateUtil.getCurrentSession();
	HibernateUtil.beginTransaction();
	
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(action, associated_ID) {
	switch(action){
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
		case 5:
			document.S_IFC_Form.nextAction.value="detach_ifc";
			document.S_IFC_Form.associated_ID.value = associated_ID;
			document.S_IFC_Form.submit();
			break;
				
		case 12:
			document.S_IFC_Form.nextAction.value="attach_ifc";
			document.S_IFC_Form.submit();
			break;
	}
}
</script>
</head>

<body>
	<html:form action="/S_IFC_Submit">
		<html:hidden property="nextAction" value=""/>

		<html:hidden property="associated_ID" value=""/>
		
		<table align=center valign=middle height=100%>
		
		<tr>
			<!-- Print errors, if any -->
			<td>
				<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
			</td>
		</tr>	
		<tr>
			<td align="center"><h1>  Shared iFC Sets -Sh-iFC- </h1></td>
		</tr>
		<tr>
			<td>
	 			<table border="0" align="center" width="450" >						
	 			<tr>
 					<td>
						<table border="0" cellspacing="1" align="center" width="70%" style="border:2px solid #FF6600;">						
						<tr bgcolor="#FFCC66">
							<td> ID-Set </td>
							<td><html:text property="id_set" readonly="true" styleClass="inputtext_readonly"/> </td>
						</tr>
						<tr bgcolor="#FFCC66">
							<td>Name </td>
							<td><html:text property="name" styleClass="inputtext"/> </td>
						</tr>
						
						
			<%			if (id_set == -1){
			%>			
						<tr bgcolor="#FFCC66">
							<td> iFC </td>							
							<td>
								<html:select property="id_ifc" name="S_IFC_Form" styleClass="inputtext" size="1" style="width:250px;">
									<html:option value="-1">Select iFC...</html:option>
									<html:optionsCollection name="S_IFC_Form" property="select_ifc" label="name" value="id"/>
								</html:select>	
							</td>
						</tr>
						<tr bgcolor="#FFCC66">
							<td>
								Priority	
							</td>
							<td>	
								<html:text property="priority" styleClass="inputtext" />
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
						<table align="center">			
						<tr>
							<td align=center> <br/>
								<html:button property="save_button" value="Save" onclick="add_action_for_form(1);"/>				
								<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2);"/> 
								<% if (id_set == -1){ %>
									<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3);"/> 
								<%}%>
								<% if (id_set != -1){ %>
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
			<tr>
			<%
					if (id_set != -1){		
			%>
				<td>	
					<table width="400">
					<tr>
						<td>	
							<b>Attach iFC </b>
						</td>
					</tr>			
							
					<tr>
						
						<td>
							<html:select property="id_ifc" name="S_IFC_Form" styleClass="inputtext" size="1" style="width:150px;" value="">
								<html:option value="-1">Select iFC...</html:option>
								<html:optionsCollection name="S_IFC_Form" property="select_ifc" label="name" value="id"/>
							</html:select>	
						</td>

						<td>
							Priority	
						</td>
						<td>	
							<html:text property="priority" styleClass="inputtext" />
						</td>
						
						<td>
							<html:button property="cap_attach_button" value="Attach" onclick="add_action_for_form(12, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					<table>
					<tr><td>
						<font color="#FF0000">
							Warning: Priority values defined here can overwrite priority values defined in SP-iFC setup! 
						</font>						
					</td></tr>
					
					</table>
					
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> Name </td>
						<td class="header"> Priority </td>
						<td class="header"> Detach </td>
					</tr>
					<%
						if (attached_ifc != null){
							Iterator it = attached_ifc.iterator();
							Shared_IFC_Set s_ifc_set = null;
							int idx = 0;
							IFC ifc = null;
							while (it.hasNext()){
								s_ifc_set = (Shared_IFC_Set) it.next();
								ifc = IFC_DAO.get_by_ID(hibSession, s_ifc_set.getId_ifc());												
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  <%= ifc.getId() %></td>												
										
								<td>  
									<a href="/hss.web.console/IFC_Load.do?id=<%= ifc.getId() %>" > 
										<%= ifc.getName() %>
									</a>	
								</td>

								<td>  
									<%=s_ifc_set.getPriority()%>
								</td>
								
								<td> 
									
									<%
										if (((String)request.getAttribute("detachDeactivation")).equals("true")){
									%>
											<input type="button" name="detach_ifc" "value="Detach" onclick="add_action_for_form(5, <%= ifc.getId() %>);" disabled/>	
									<%
										}
										else{
									%>
											<input type="button" name="detach_ifc" "value="Detach" onclick="add_action_for_form(5, <%= ifc.getId() %>);" />										
									<%
										}
									%>												
								</td>
							</tr>											
					<%			
								idx++;												
								}
							}
					%>
					</table>
					
				</td>	
			<%
				}
			%>	
		</tr>
		</table>		
	</html:form>
</body>
</html>
