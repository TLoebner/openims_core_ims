<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>
<jsp:useBean id="resultList" type="java.util.List" scope="request"></jsp:useBean>
<jsp:useBean id="maxPages" type="java.lang.String" scope="request"></jsp:useBean>
<jsp:useBean id="currentPage" type="java.lang.String" scope="request"></jsp:useBean>
<jsp:useBean id="rowPerPage" type="java.lang.String" scope="request"></jsp:useBean>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link rel="stylesheet" type="text/css"
	href="/hss.web.console/style/fokus_ngni.css">

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title><bean:message key="result.title" /></title>

<script type="text/javascript" language="JavaScript">

function submitForm(pageId){
	document.IFC_SearchForm.crtPage.value = pageId;
	document.IFC_SearchForm.submit();
}

function rowsPerPageChanged(){
	document.IFC_SearchForm.crtPage.value = 1;
	document.IFC_SearchForm.submit();
}
</script> 

</head>
<body>

<center>
<table>
<tr>
	<td><br/><br/><h1>Initial Filter Criteria - Search Results </h1><br/><br/></td>
</tr>
</table>
</center>

<center>
	<table class="as" width="500">
		<logic:notEmpty name="resultList">
			<tr class="header">
				<td class="header"> ID </td>
				<td class="header"> Name </td>
				<td class="header"> Triggering Point </td>									
				<td class="header"> Application Server </td>									
				<td class="header"> Profile Part Indicator </td>													
			</tr>
				
			<logic:iterate name="resultList" id="ifc"
				type="de.fhg.fokus.hss.db.model.IFC" indexId="ix">
				<tr class="<%= ix.intValue()%2 == 0 ? "even" : "odd" %>">

					<td>
						<bean:write name="ifc" property="id" />
					</td>

					<td> 
						<a href="/hss.web.console/IFC_Load.do?id=<bean:write name="ifc" property="id" />"> 
							<bean:write name="ifc" property="name" />
						</a>	
					</td>

					<td> 
						<bean:write name="ifc" property="id_tp" />
					</td>

					<td> 
						<bean:write name="ifc" property="id_application_server" />
					</td>

					<td> 
						<bean:write name="ifc" property="profile_part_ind" />
					</td>

				</tr>
			</logic:iterate>
			
			<tr>
				<td colspan="3" class="header">
				
				<html:form action="/IFC_Search">
					<table>
						<tr>
							<td>
								<%
									int length = Integer.parseInt(maxPages) + 1;
									int cPage = Integer.parseInt(currentPage) + 1;
									for (int iy = 1; iy < length; iy++) {
										if (cPage != iy) {
									%>
									<a href="javascript:submitForm(<%=String.valueOf(iy)%>);"><%=iy%></a>
								<%
									} else {
									%> 
									<font style="color:#FF0000;font-weight: 600;"> <%=String.valueOf(iy)%>
									</font> 
								<% }
							}
							%>
							</td>
							
							<td><bean:message key="result.rowsPerPage" /><br>
							<html:hidden property="crtPage"></html:hidden> 
							<html:select property="rowsPerPage" onchange="javascript:rowsPerPageChanged();">

							<option value="20"
								<%= rowPerPage.equals("20") ? "selected" : "" %> >20 </option>
							<option value="30"
								<%= rowPerPage.equals("30") ? "selected" : "" %> >30 </option>
							<option value="50"
								<%= rowPerPage.equals("50") ? "selected" : "" %> >50</option>
							<option value="100"
								<%= rowPerPage.equals("100") ? "selected" : "" %> >100</option>
							</html:select></td>
						</tr>
					</table>
				</html:form></td>
			</tr>
		</logic:notEmpty> 
		
		<tr><td>
		<logic:empty name="resultList">
			<bean:message key="result.emptryResultSet" />
		</logic:empty></td>
		</td></tr>
		</table>		
</center>

</body>
</html>
