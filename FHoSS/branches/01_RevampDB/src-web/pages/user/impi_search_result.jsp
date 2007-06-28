<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.db.model.*, java.util.*, de.fhg.fokus.hss.auth.HexCodec" %>

<jsp:useBean id="resultList" type="java.util.List" scope="request"></jsp:useBean>
<jsp:useBean id="maxPages" type="java.lang.String" scope="request"></jsp:useBean>
<jsp:useBean id="currentPage" type="java.lang.String" scope="request"></jsp:useBean>
<jsp:useBean id="rowPerPage" type="java.lang.String" scope="request"></jsp:useBean>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<link rel="stylesheet" type="text/css" href="/hss.web.console/style/fokus_ngni.css">
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
	<title><bean:message key="result.title" /></title>

<script type="text/javascript" language="JavaScript">
function submitForm(pageId){
	document.IMPI_SearchForm.crtPage.value = pageId;
	document.IMPI_SearchForm.submit();
}

function rowsPerPageChanged(){
	document.IMPI_SearchForm.crtPage.value = 1;
	document.IMPI_SearchForm.submit();
}
</script> 
</head>

<body>
	<table id="title-table" align="center" weight="100%" >
	<tr>
		<td align="center">
			<h1> Private User Identity - Search Results </h1> 
			<br/><br/>		
		</td>
	</tr>
	</table>

	<table id="main-table" align="center" valign="middle">
	<tr>
		<td>
	 		<table  id="result-table" class="as" border="0" cellspacing="1" align="center" width="600" style="border:2px solid #FF6600;">	
				<tr class="header">
					<td class="header"> ID </td>
					<td class="header"> Identity </td>
					<td class="header"> Auth. Scheme</td>
					<td class="header"> SQN </td>													
				</tr>

				<%
				if (resultList != null && resultList.size() > 0){
					IMPI impi;
					int idx = 0;
					Iterator it = resultList.iterator();
					
					while (it.hasNext()){
						impi = (IMPI) it.next();
				%>		
					<tr class="<%= idx % 2 == 0 ? "even" : "odd" %>">
						<td>
							<%= impi.getId() %>
						</td>
						<td> 
							<a href="/hss.web.console/IMPI_Load.do?id=<%= impi.getId() %>"> 
								<%= impi.getIdentity() %>
							</a>	
						</td>
						<td>
							<%= impi.getAuth_scheme() %>
						</td>
						<td>
							<%= impi.getSqn() %>
						</td>
					</tr>
				<%		
			
						idx++;		
					} //while
				} // if
				else{	
				%>
					<tr>
						<td>
							<bean:message key="result.emptyResultSet" />
						</td>
					</tr>						
				<%
				}
				%>	
				
			</table> <!-- result-table -->			
		</td>
	</tr>
	<tr>
		<td colspan="3" class="header">
			<html:form action="/IMPI_Search">
				<table align="center">
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
							} 
							else {
					%> 
									<font style="color:#FF0000;font-weight: 600;"> 
										<%=String.valueOf(iy)%>
									</font> 
					<% 
							}
						}
					%>
					</td>
					<td>
						<bean:message key="result.rowsPerPage" /><br>
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
						</html:select>
					</td>
				</tr>
				</table> 
			</html:form>
		</td>
	</tr>
	</table> <!-- main-table-->	
</body>
</html>
