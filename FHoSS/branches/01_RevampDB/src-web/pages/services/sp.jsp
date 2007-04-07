<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
	
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions, java.util.*, de.fhg.fokus.hss.db.model.*, de.fhg.fokus.hss.db.op.*, 
	de.fhg.fokus.hss.db.hibernate.*, org.hibernate.Session" %>

<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> SP </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
<jsp:useBean id="attached_ifc_list" type="java.util.List" scope="request"></jsp:useBean>
<jsp:useBean id="attached_shared_ifc_list" type="java.util.List" scope="request"></jsp:useBean>

<%
	int id = Integer.parseInt(request.getParameter("id"));
	Session hibSession = HibernateUtil.getCurrentSession();
	HibernateUtil.beginTransaction();
	
%>

<script type="text/javascript" language="JavaScript">

function add_action_for_form(action, associated_ID) {

	switch(action){
		case 1:
			document.SP_Form.nextAction.value="save";
			document.SP_Form.submit();
			break;
		case 2:
			document.SP_Form.nextAction.value="refresh";
			document.SP_Form.submit();			
			break;
		case 3:
			document.SP_Form.nextAction.value="reset";
			document.SP_Form.reset();
			break;
		case 4:
			document.SP_Form.nextAction.value="delete";
			document.SP_Form.submit();			
			break;
		case 5:
			document.SP_Form.nextAction.value="detach_ifc";
			document.SP_Form.associated_ID.value = associated_ID;			
			document.SP_Form.submit();			
			break;
		case 6:
			document.SP_Form.nextAction.value="detach_shared_ifc";
			document.SP_Form.associated_ID.value = associated_ID;						
			document.SP_Form.submit();			
			break;

		case 12:
			document.SP_Form.nextAction.value="attach_ifc";
			document.SP_Form.submit();			
			break;

		case 13:
			document.SP_Form.nextAction.value="attach_shared_ifc";
			document.SP_Form.submit();			
			break;
			
	}
}

</script>
</head>

<body>
<html:form action="/SP_Submit">
	<html:hidden property="nextAction" value=""/>
	<html:hidden property="associated_ID" value=""/>
		
	<table align=center valign=middle height=100%>
	<!-- Print errors, if any -->
	<tr>
		<td>
			<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
		</td>
	</tr>	
	<tr>
		<td align="center"><h1> Service Profile -SP- </h1></td>
	</tr>
	<tr>
		<td>
	 		<table border="0" align="center" width="350" >						
 			<tr>
 				<td>
					<table border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">						
					<tr bgcolor="#FFCC66">
						<td> ID </td>
						<td><html:text property="id" readonly="true" styleClass="inputtext_readonly"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Name </td>
						<td><html:text property="name" styleClass="inputtext"/> </td>
					</tr>
					<tr bgcolor="#FFCC66">
						<td>Core Network Service Auth </td>
						<td><html:text property="cn_service_auth" styleClass="inputtext"/> </td>
					</tr>
					</table>
				</td>
			</tr>
 			<tr>
 				<td>
					<table align="center">			
					<tr>
						<td align=center> <br/>
							<html:button property="save_button" value="Save" onclick="add_action_for_form(1, -1);"/>				
							<html:button property="refresh_button" value="Refresh" onclick="add_action_for_form(2, -1);"/> 
							<% if (id == -1){ %>
								<html:button property="reset_button" value="Reset" onclick="add_action_for_form(3, -1);"/> 
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
	<%
		if (id != -1){
	%>
	<tr>
		<td>
			<table>
			<tr>
				<td>
					<table>
					<tr>
						<td>	
							<b>Attach IFC </b>
						</td>
					</tr>					
					<tr>
						
						<td>
							<html:select property="ifc_id" name="SP_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:option value="-1">Select IFC...</html:option>
								<html:optionsCollection name="SP_Form" property="select_ifc" label="name" value="id"/>
							</html:select>	
						</td>

						<td>	
							<b> &nbsp Priority </b>
						</td>

						<td>	
							<html:text property="sp_ifc_priority" value="1" styleClass="inputtext" style="width:100px;" />
						</td>
						
						<td>
							<html:button property="ifc_attach_button" value="Attach" onclick="add_action_for_form(12, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID </td>			
						<td class="header"> IFC Name </td>
						<td class="header"> Priority </td>
						<td class="header"> Detach </td>
					</tr>
					<%
						if (attached_ifc_list != null){
							Iterator it = attached_ifc_list.iterator();
							IFC ifc = null;
							int idx = 0;
							while (it.hasNext()){
								ifc = (IFC) it.next();
												
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  <%= ifc.getId() %></td>												
										
								<td>  
									<a href="/hss.web.console/IFC_Load.do?id=<%= ifc.getId() %>" > 
										<%= ifc.getName() %>
									</a>	
								</td>

								<td>  
									<%
										SP_IFC sp_ifc = SP_IFC_DAO.get_by_SP_and_IFC_ID(hibSession, id, ifc.getId());
										out.println(sp_ifc.getPriority());
									%>
								</td>
								
								<td> 
									<input type="button" name="detach_ifc" 
										"value="Detach" onclick="add_action_for_form(5, <%= ifc.getId() %>);"/>													
								</td>
							</tr>											
					<%			
								idx++;												
								}
							}
					%>
					</table>
				</td>
				
				<td>
					<br/>
				</td>
				
				<td>	
					<table>
					<tr>
						<td>	
							<b>Attach Shared-IFC-Set </b>
						</td>
						
					</tr>
					<tr>
						<td>
							<html:select property="shared_ifc_id" name="SP_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:option value="-1">Select Shared-iFC...</html:option>
								<html:optionsCollection name="SP_Form" property="select_shared_ifc" label="name" value="id_set"/>
							</html:select>	
						</td>	
						<td>
							<html:button property="ifc_attach_button" value="Attach" onclick="add_action_for_form(13, -1);"/>
							<br />
						</td>
					</tr>	
					</table>
					<table class="as" border="0" cellspacing="1" align="center" width="100%" style="border:2px solid #FF6600;">
					<tr class="header">
						<td class="header"> ID-Set </td>			
						<td class="header"> Name </td>
						<td class="header"> Detach </td>
					</tr>
					<%
						if (attached_shared_ifc_list != null){
							Iterator it = attached_shared_ifc_list.iterator();
							Shared_IFC_Set shared_ifc_set = null;
							int idx = 0;
							while (it.hasNext()){
								shared_ifc_set = (Shared_IFC_Set) it.next();
												
					%>
							<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">																			
								<td>  <%= shared_ifc_set.getId_set() %></td>												
										
								<td>  
									<a href="/hss.web.console/S_IFC_Load.do?id_set=<%= shared_ifc_set.getId_set() %>" > 
										<%= shared_ifc_set.getName() %>
									</a>	
								</td>

								<td> 
									<input type="button" name="detach_shared_ifc" 
										"value="Detach" onclick="add_action_for_form(6, <%= shared_ifc_set.getId_set() %>);"/>													
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
			</table>
		</td>
	</tr>
	<%
		}
	%>
	</table>
</html:form>
</body>
</html>
