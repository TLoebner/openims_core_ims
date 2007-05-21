<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-nested"
	prefix="nested"%>

<%@ page import="java.util.*, de.fhg.fokus.hss.db.model.*, de.fhg.fokus.hss.cx.CxConstants,
	de.fhg.fokus.hss.web.form.*, de.fhg.fokus.hss.util.SecurityPermissions" %>
<html>
<head>

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> AS </title>
<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
<jsp:useBean id="attached_ifc_list" type="java.util.List" scope="request"></jsp:useBean>

<%
	String action=request.getParameter("action");
	int id = Integer.parseInt(request.getParameter("id"));
%>

<script type="text/javascript" language="JavaScript">
function add_action_for_form(action, associated_ID) {
	switch(action){
		case 1:
			document.TP_Form.nextAction.value="save";
			document.TP_Form.submit();
			break;
		case 2:
			document.TP_Form.nextAction.value="refresh";
			document.TP_Form.submit();			
			break;
		case 3:
			document.TP_Form.nextAction.value="reset";
			document.TP_Form.reset();
			break;
		case 4:
			document.TP_Form.nextAction.value="delete";
			document.TP_Form.submit();			
			break;
		case 5:
			document.TP_Form.nextAction.value="detach_ifc";
			document.TP_Form.associated_ID.value = associated_ID;			
			document.TP_Form.submit();			
			break;
		case 6:
			document.TP_Form.nextAction.value="delete_spt";
			document.TP_Form.associated_ID.value = associated_ID;			
			document.TP_Form.submit();			
			break;

		case 12:
			document.TP_Form.nextAction.value="attach_ifc";
			document.TP_Form.submit();			
			break;

		case 13:
			document.TP_Form.nextAction.value="save_spt";
			document.TP_Form.submit();
			break;
			
					
	}
}

function addSpt(groupId, is_spt_list_empty){
	document.TP_Form.nextAction.value = "attach_spt";
	if (is_spt_list_empty == 1){
		document.TP_Form.type.value = document.TP_Form.typeSelect.value;
	}
	else{
		document.TP_Form.type.value = document.TP_Form.typeSelect[groupId].value;
	}
	document.TP_Form.group.value = groupId;
	document.TP_Form.submit();
}
</script>
</head>

<body>

<html:form action="/TP_Submit">
	<html:hidden property="nextAction" value=""/>
	<html:hidden property="associated_ID" value=""/>
	
	<html:hidden property="group" />
	<html:hidden property="type" />
	
	<table align=center valign=middle height=100%>
	<!-- Print errors, if any -->
	<tr>
		<td>
			<jsp:include page="/pages/tiles/error.jsp"></jsp:include>
		</td>
	</tr>	
	<tr>
		<td align="center"><h1> Trigger Point -TP- </h1></td>
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
						<td>Condition Type CNF </td>
						<td>
							<html:select property="condition_type_cnf" name="TP_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:optionsCollection name="TP_Form" property="select_condition_type" label="name" value="code"/>
							</html:select>
						</td>
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
			
	<%
				if (id != -1){
	%>
				<td>
					<table>
					<tr>
						<td>	
							<b>Attach IFC </b>
						</td>
					</tr>					
					<tr>
						
						<td>
							<html:select property="ifc_id" value="-1" name="TP_Form" styleClass="inputtext" size="1" style="width:250px;">
								<html:option value="-1">Select IFC...</html:option>
								<html:optionsCollection name="TP_Form" property="select_ifc" label="name" value="id"/>
							</html:select>	
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
				<td> <br/> </td>
			</tr>	
			</table>		
		</td>
		<td> <br/> </td>
	</tr>
	<tr>		
		<td>
			<table>
			<tr>
				<b> Add SPTs to Trigger Point </b>
			</tr>
			<tr>
				<td>				
					<table width="730" class="as" cellspacing="0" cellpadding="0">
					<tr class="header">
						<td><br>
			<% 
							int lastGroupId = 0;
							int groupCnt = 0;							
			%>
							<table class="as" cellpadding="0" cellspacing="0" border="0" width="700">

								<nested:iterate property="spts" name="TP_Form"
									id="spt" type="SPT_Form" indexId="ix">
					<%

								if (lastGroupId != spt.getGroup() && groupCnt > 0){
					%>
								<tr class="<%= lastGroupId%2 == 0 ? "even" : "odd" %>">
									<td style="text-align:center;" colspan="12">
										<select name="typeSelect">
											<option value="<%= CxConstants.SPT_Type_RequestURI %>">
												<%= CxConstants.SPT_Type_RequestURI_Name %>
											</option>
											<option value="<%= CxConstants.SPT_Type_Method %>">
												<%= CxConstants.SPT_Type_Method_Name %>
											</option>
											<option value="<%= CxConstants.SPT_Type_SIPHeader %>">
												<%= CxConstants.SPT_Type_SIPHeader_Name %>
											</option>
											<option value="<%= CxConstants.SPT_Type_SessionCase %>">
												<%= CxConstants.SPT_Type_SessionCase_Name %>
											</option>
											<option value="<%= CxConstants.SPT_Type_SessionDescription %>">
												<%= CxConstants.SPT_Type_SessionDescription_Name %>
											</option>
										</select> 
					
										<a href="javascript:addSpt(<%=(lastGroupId)%>, 0);">
											<img src="/hss.web.console/images/add_obj.gif" width="16"
												height="16" alt="Add" />
										</a>
									</td>
								</tr>		
								
								<tr class="header">
									<td style="text-align:center;" colspan="12">
										<logic:equal name="TP_Form" property="condition_type_cnf" value="1">AND</logic:equal>
										<logic:equal name="TP_Form" property="condition_type_cnf" value="0">OR</logic:equal>
									</td>
								</tr>

					<%							
									lastGroupId = spt.getGroup();		
									groupCnt = 0;	
								}
					%>
								
								<tr class="<%= lastGroupId%2 == 0 ? "even" : "odd" %>">
								
									<td nowrap="nowrap" width="120">
										<nested:hidden property="sptId"> </nested:hidden> 
										<nested:hidden property="group"></nested:hidden> 
										<nested:hidden property="type"></nested:hidden> 
									
			<%							if (spt.getType() == CxConstants.SPT_Type_RequestURI) {
			
			%> 								<bean:message key="spt.head.requestUri" />
									</td>								
									<td class="tgpFormular">
										<nested:text property="requestUri" styleClass="inputtext" 
											style="width:280px;" maxlength="255" onchange="add_action_for_form(13, -1);" />
									</td>
	
									<td>								
			<%					
										} else if (spt.getType() == CxConstants.SPT_Type_Method) {
			%>
										<bean:message key="spt.head.sipMethod" />
									</td>
									<td class="tgpFormular">
										<nested:select property="sipMethod" onchange="add_action_for_form(13, -1);">
											<nested:optionsCollection property="sipMethodList" label="label" value="value"/>
										</nested:select>
									</td>
								
									<td>
			<%
										} else if (spt.getType() == CxConstants.SPT_Type_SessionCase) {
			%>
										<bean:message key="spt.head.sessionCase" />
									</td>
									<td class="tgpFormular">
										<nested:select property="sessionCase" value="0" styleClass="inputtext" style="width:280px;" 
											onchange="add_action_for_form(13, -1);">
											<nested:optionsCollection property="directionOfRequestList" label="name" value="code"/>
										</nested:select>
									</td>
									
									<td>									
			<% 
									} else if (spt.getType() == CxConstants.SPT_Type_SessionDescription) {
			%>
										<bean:message key="spt.head.sessionDesc" />
									</td>
									<td>
										<table>
										<tr>
											<td class="tgpFormular">
												<bean:message key="spt.head.sessionDescLine" />
											</td>
											<td class="tgpFormular" nowrap="nowrap" colspan="3">
												<nested:text property="sessionDescLine" size="10" styleClass="inputtext"
													style="width:200px;" onchange="add_action_for_form(13, -1);"/>
											</td>
										</tr>
										<tr>
											<td class="tgpFormular" nowrap="nowrap">
												<bean:message key="spt.head.sessionDescContent" />
											</td>
											<td class="tgpFormular" colspan="2">
												<nested:text property="sessionDescContent" size="10" styleClass="inputtext" style="width:200px;" 
													onchange="add_action_for_form(13, -1);"/>
											</td>
										</tr>
										</table>
									</td>

									<td>
			<%				
									} else if (spt.getType() == CxConstants.SPT_Type_SIPHeader) {

			%>
									<bean:message key="spt.head.sipHeader" />
									</td>
									
									<td>
										<table>
										<tr>
											<td class="tgpFormular">
												<bean:message key="spt.head.sipHeader" />
											</td>
											<td class="tgpFormular" colspan="2">
												<nested:text property="sipHeader" size="10" styleClass="inputtext"
														style="width:200px;" onchange="add_action_for_form(13, -1);"/>
											</td>
										</tr>
										<tr>
											<td class="tgpFormular" nowrap="nowrap">
												<bean:message key="spt.head.sipHeaderContent" />
											</td>
											<td class="tgpFormular" colspan="2">
												<nested:text property="sipHeaderContent" size="10"
														styleClass="inputtext" style="width:200px;" onchange="add_action_for_form(13, -1);"/>
											</td>
										</tr>
										</table>
									</td>
			<%					
									}

			%>
								<!-- Condition Negated -->
								<td nowrap="nowrap">
									<bean:message key="spt.head.neg" />
								</td>
								<td class="tgpFormular">
									<nested:checkbox property="neg" styleClass="inputtext" onclick="add_action_for_form(13, -1);"/>
								</td>

								<!-- Registration Type: Registration -->
								<td nowrap="nowrap">
									Reg
								</td>
								<td class="tgpFormular">
									<nested:radio property="rtype" value="rtype_reg" styleClass="inputtext" onchange="add_action_for_form(13, -1);"/>
								</td>

								<!-- Registration Type: Re-Registration -->
								<td nowrap="nowrap">
									ReReg
								</td>
								<td class="tgpFormular">
									<nested:radio property="rtype" value="rtype_re_reg" styleClass="inputtext" onchange="add_action_for_form(13, -1);"/>
								</td>

								<!-- Registration Type: UnRegistration -->
								<td nowrap="nowrap">
									DeReg
								</td>
								<td class="tgpFormular">
									<nested:radio property="rtype" value="rtype_de_reg" styleClass="inputtext" onchange="add_action_for_form(13, -1);"/>
								</td>
								
								<td>
									<input type="button" name="delete_button" value="Delete" 
										onclick="add_action_for_form(6, <%= spt.getSptId() %>);" />		
								</td>	

							</tr>
							<tr class="<%= lastGroupId%2 == 1 ? "even" : "odd" %>">
								<td style="text-align:center;" colspan="12">
									<logic:equal name="TP_Form" property="condition_type_cnf" value="0">
										AND
									</logic:equal>
									<logic:equal name="TP_Form" property="condition_type_cnf" value="1">
										OR
									</logic:equal>
								</td>
							</tr>
					
						<%
							lastGroupId = spt.getGroup();
							groupCnt++;
						%>
						</nested:iterate>

						<!-- End of the last Group -->
						<logic:notEmpty property="spts" name="TP_Form">
							<tr class="<%= lastGroupId%2 == 0 ? "even" : "odd" %>">
								<td style="text-align:center;" colspan="12">
									<select name="typeSelect">
										<option value="<%= CxConstants.SPT_Type_RequestURI %>">
											<%= CxConstants.SPT_Type_RequestURI_Name %>
										</option>
										<option value="<%= CxConstants.SPT_Type_Method %>">
											<%= CxConstants.SPT_Type_Method_Name %>
										</option>
										<option value="<%= CxConstants.SPT_Type_SIPHeader %>">
											<%= CxConstants.SPT_Type_SIPHeader_Name %>
										</option>
										<option value="<%= CxConstants.SPT_Type_SessionCase %>">
											<%= CxConstants.SPT_Type_SessionCase_Name %>
										</option>
										<option value="<%= CxConstants.SPT_Type_SessionDescription %>">
											<%= CxConstants.SPT_Type_SessionDescription_Name %>
										</option>
									</select> 
			
									<a href="javascript:addSpt(<%=lastGroupId%>, 0);">
										<img src="/hss.web.console/images/add_obj.gif" width="16"
											height="16" alt="Add" />
									</a>	
								</td>
							</tr>
							<tr class="header">
								<td style="text-align:center;" colspan="12">
									<logic:equal name="TP_Form" property="condition_type_cnf" value="1">AND</logic:equal>
									<logic:equal name="TP_Form" property="condition_type_cnf" value="0">OR</logic:equal>
								</td>
							</tr>
							
						<%
							lastGroupId++;
						%>
						</logic:notEmpty>						
						
						<!-- Root Group -->
						<tr class="<%= lastGroupId%2 == 0 ? "even" : "odd" %>">
							<td style="text-align:center;" colspan="12">
								<select name="typeSelect">
									<option value="<%= CxConstants.SPT_Type_RequestURI %>">
										<%= CxConstants.SPT_Type_RequestURI_Name %>
									</option>
									<option value="<%= CxConstants.SPT_Type_Method %>">
										<%= CxConstants.SPT_Type_Method_Name %>
									</option>
									<option value="<%= CxConstants.SPT_Type_SIPHeader %>">
										<%= CxConstants.SPT_Type_SIPHeader_Name %>
									</option>
									<option value="<%= CxConstants.SPT_Type_SessionCase %>">
										<%= CxConstants.SPT_Type_SessionCase_Name %>
									</option>
									<option value="<%= CxConstants.SPT_Type_SessionDescription %>">
										<%= CxConstants.SPT_Type_SessionDescription_Name %>
									</option>
								</select> 
								
			<%
							if (lastGroupId == 0){					
			%>					
								<a href="javascript:addSpt(<%=lastGroupId%>, 1);">
									<img src="/hss.web.console/images/add_obj.gif" width="16"
										height="16" alt="Add" />
								</a>
								
			<%
							}
							else{
			%>						
								<a href="javascript:addSpt(<%=lastGroupId%>, 0);">
									<img src="/hss.web.console/images/add_obj.gif" width="16"
										height="16" alt="Add" />
								</a>
			<%				
							}
			%>					
							</td>
						</tr>
						</table>
					<br></td>
				</tr>
				</table>
			</td>
		</tr>
		</table>
	
		</td>
	</tr>
	</table>
	<%
			}
			else{
	%>
			</table>
		</td>
	</tr>
	</table>				
	<%
			}
	%>
</html:form>

</body>
</html>
