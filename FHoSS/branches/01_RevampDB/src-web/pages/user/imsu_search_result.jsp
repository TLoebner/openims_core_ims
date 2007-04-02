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

</head>
<body>
<center><h1><br/><br/>IMS Subscription - Search Results </h1></center>

<table align=center valign=middle height=100%>
	<tr><td>
	 <table class="as" border="0" cellspacing="1" align="center" style="border:2px solid #FF6600;">	
		<logic:notEmpty name="resultList">
			<tr class="header">
				<td class="header"> ID </td>
				<td class="header"> Name </td>
				<td class="header"> S-CSCF Name </td>
				<td class="header"> Diameter Name </td>
			</tr>
				
			<logic:iterate name="resultList" id="imsu"
				type="de.fhg.fokus.hss.db.model.IMSU" indexId="ix">
				<tr class="<%= ix.intValue()%2 == 0 ? "even" : "odd" %>">
					<td>
						<bean:write name="imsu" property="id" />
					</td>
					<td> 
						<a href="/hss.web.console/IMSU_Load.do?id=<bean:write name="imsu" property="id" />"> 
							<bean:write name="imsu" property="name" />
						</a>	
					</td>
					<td>
						<bean:write name="imsu" property="scscf_name" />
					</td>
					<td>
						<bean:write name="imsu" property="diameter_name" />
					</td>
				</tr>
			</logic:iterate>
			
			<%if (Integer.parseInt(maxPages) > 1) {

			%>
			<tr>
				<td colspan="3" class="header">
				<script type="text/javascript"
					language="JavaScript">
					function submitForm(pageId){
						document.IMSU_SearchForm.page.value = pageId;
						document.IMSU_SearchForm.submit();
					}
				</script> 
				
				<html:form action="/IMSU_Search">
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
							<html:hidden property="page"></html:hidden> 
							<html:select property="rowsPerPage" onchange="javascript:document.IMSU_SearchForm.submit();">

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
			<%}

		%>
		</logic:notEmpty> 
		
		<tr><td>
		<logic:empty name="resultList">
			<bean:message key="result.emptryResultSet" />
		</logic:empty></td>
		</td></tr>
		</table>		
	</td></tr>
</table>
</body>
</html>
