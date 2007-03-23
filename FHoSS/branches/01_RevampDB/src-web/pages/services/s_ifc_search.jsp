<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-logic"
	prefix="logic"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<link rel="stylesheet" type="text/css"
	href="/hss.web.console/style/fokus_ngni.css">

<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title><bean:message key="result.title" /></title>
</head>
<body>

<center>
<table>
<tr>
	<td><br/><br/><h1>Shared iFC - Search </h1><br/><br/></td>
</tr>
</table>
</center>

<html:form action="S_IFC_Search">
<center>
		<table class="as">
			<tr>
				<td></td>
				<td><h2>Enter Search Parameters:</h2></td>
			</tr>

			<tr class="header">
				<td class="tgpFormular">ID</td>
				<td class="tgpFormular"><html:text property="id_s_ifc" value=""/></td>
			</tr>
			<tr class="header">
				<td class="tgpFormular">Name</td>
				<td class="tgpFormular"><html:text property="name" value=""/></td>
			</tr>
			<tr class="header">
				<td class="tgpFormular">Set-ID</td>
				<td class="tgpFormular"><html:text property="id_set" value=""/></td>
			</tr>
			<tr class="header">
				<td class="tgpFormular">IFC-Name</td>
				<td class="tgpFormular"><html:text property="name_ifc" value=""/></td>
			</tr>
		</table>
		<table>		
			<tr>
				<td><br/><html:submit property="search" value="Search" /></td>
			</tr>				
		</table>		
</center>
</html:form>
</body>
</html>
