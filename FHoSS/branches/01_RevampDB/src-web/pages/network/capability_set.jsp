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
	de.fhg.fokus.hss.db.hibernate.*, org.hibernate.Session, de.fhg.fokus.hss.util.SecurityPermissions" %>
<jsp:useBean id="attached_cap" type="java.util.List" scope="request"></jsp:useBean>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Capability </title>
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
			document.CapS_Form.nextAction.value="save";
			document.CapS_Form.submit();
			break;
		case 2:
			document.CapS_Form.nextAction.value="reset";
			document.CapS_Form.reset();
			break;
		
		case 3:
			document.CapS_Form.nextAction.value="refresh";
			document.CapS_Form.submit();			
			break;

		case 4:
			document.CapS_Form.nextAction.value="delete";
			document.CapS_Form.submit();
			break;
		
		case 5:
			document.CapS_Form.nextAction.value="detach_cap";
			document.CapS_Form.associated_ID.value = associated_ID;
			document.CapS_Form.submit();
			break;
				
		case 12:
			document.CapS_Form.nextAction.value="attach_cap";
			document.CapS_Form.submit();
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
		
		<html:form action="/CapS_Submit">
			<html:hidden property="nextAction" value=""/>
			<html:hidden property="associated_ID" value=""/>		
			
			<tr>
				<td align="center"><h1>Capability Sets</h1></td>
			</tr>
			<tr>
				<td>
			 		<table border="0" align="center" width="350" >						
			 		<tr>
			 				<td>
						 		<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
								<tr bgcolor="#FFCC66">
									<td> ID-Set </td>
									<td>
										<html:text property="id_set" readonly="true" styleClass="inputtext_readonly" style="width:100px;"/> 
									</td>
								</tr>
								
								<tr bgcolor="#FFCC66">
									<td> Name </td>
									<td>
										<html:text property="name" styleClass="inputtext"/> 
									</td>
								</tr>

			<%					if (id_set == -1){
			%>			
								<tr bgcolor="#FFCC66">
									<td> Capability </td>							
									<td>
										<html:select property="id_cap" name="CapS_Form" styleClass="inputtext" size="1" style="width:250px;">
											<html:option value="-1">Select Capability...</html:option>
											<html:optionsCollection name="CapS_Form" property="select_cap" label="name" value="id"/>
										</html:select>	
									</td>
								</tr>
								
								<tr bgcolor="#FFCC66">
									<td>	
										Type
									</td>
									<td>	
										<html:select property="cap_type" styleClass="inputtext" size="1" style="width:250px;">
											<html:option value="-1">Select Type...</html:option>
											<html:optionsCollection name="CapS_Form" property="select_cap_type" label="name" value="code"/>
										</html:select>
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
									<td align=center> 
										<html:button property="save_button" value="Save" onclick="add_action_for_form(1, -1);"/>				
										<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(3, -1);"/> 
										<% if (id_set == -1){ %>
											<html:button property="reset_button" value="Reset" onclick="add_action_for_form(2, -1);"/> 
										<%}%>
						
										<% if (id_set != -1){ %>
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
			<tr>
			<%
					if (id_set != -1){		
			%>
				<td>	
					<table width="400">
					<tr>
						<td>	
							<b>Attach Capability </b>
						</td>
					</tr>					
					<tr>
						
						<td>
							<html:select property="id_cap" name="CapS_Form" styleClass="inputtext" size="1" style="width:150px;">
								<html:option value="-1">Select Capability...</html:option>
								<html:optionsCollection name="CapS_Form" property="select_cap" label="name" value="id"/>
							</html:select>	
						</td>

						<td>	
							<html:select property="cap_type" styleClass="inputtext" size="1" style="width:150px;">
								<html:option value="-1">Select Type...</html:option>
								<html:optionsCollection name="CapS_Form" property="select_cap_type" label="name" value="code"/>
							</html:select>
						</td>
						
						<td>
							<html:button property="cap_attach_button" value="Attach" onclick="add_action_for_form(12, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> Name </td>
						<td class="header"> Mandatory </td>
						<td class="header"> Detach </td>
					</tr>
					<%
						if (attached_cap != null){
							Iterator it = attached_cap.iterator();
							CapabilitiesSet cap_set = null;
							int idx = 0;
							Capability cap = null;
							while (it.hasNext()){
								cap_set = (CapabilitiesSet) it.next();
								cap = Capability_DAO.get_by_ID(hibSession, cap_set.getId_capability());												
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  <%= cap.getId() %></td>												
										
								<td>  
									<a href="/hss.web.console/Cap_Load.do?id=<%= cap.getId() %>" > 
										<%= cap.getName() %>
									</a>	
								</td>

								<td>  
									<%
										Tuple tuple = (Tuple) WebConstants.select_cap_type.get(cap_set.getIs_mandatory()); 
										out.println(tuple.getName()); 
									%>
								</td>
								
								<td> 
									<input type="button" name="detach_cap" 
										"value="Detach" onclick="add_action_for_form(5, <%= cap.getId() %>);"/>													
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
