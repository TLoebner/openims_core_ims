<%@ page language="java" contentType="text/html; charset=ISO-8859-1"
	pageEncoding="ISO-8859-1" import="de.fhg.fokus.hss.path.TreeNode"%>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<%@ taglib uri="http://jakarta.apache.org/struts/tags-bean"
	prefix="bean"%>
<%@ taglib uri="http://jakarta.apache.org/struts/tags-html"
	prefix="html"%>
<%@ page import="de.fhg.fokus.hss.util.SecurityPermissions" %>

<html>
<head>
<link rel="stylesheet" type="text/css" 
	href="/hss.web.console/style/fokus_ngni.css">
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<title> Services Menu Page</title>
</head>

<body>
<table id="main" height="100%">
	<tr>
		<td id="bound_left">&nbsp;</td>
		<td valign="top" bgcolor="#FFFFFF">
			<h1> Services </h1>
			
			<ul>
			
			<!-- Service Profile -->
			<li> <b> Service Profiles </b> <br>			
			<a href="service_profile.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMPU)) { %>
				<a href="service_profile.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>

			<!-- Application Server -->
			<li> <b> Application Servers </b> <br>
			<a href="app_server.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMPI)) { %>
				<a href="app_server.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>

			<!-- Trigger Point -->
			<li> <b> Trigger Points </b> <br>
			<a href="tp.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMSU)) { %>
				<a href="tp.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>

			<!-- Initial Filter Criteria -->
			<li> <b> Initial Filter Criteria </b> <br>
			<a href="ifc.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMSU)) { %>
				<a href="ifc.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>

			<!-- Shared iFC -->
			<li> <b> Shared iFC </b> <br>
			<a href="sh_ifc.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMSU)) { %>
				<a href="sh_ifc.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>
			</ul>
			
			<h1> Public Service Identifiers </h1>
			
			<ul>
			
			<!-- PSI Template -->
			<li> <b> PSI Template </b> <br>			
			<a href="psi_template.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMPU)) { %>
				<a href="psi_template.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>

			<!-- PSI -->
			<li> <b> PSI </b> <br>			
			<a href="psi.jsp?action=search" target="content"> Search </a> <br>
			<% if(request.isUserInRole(SecurityPermissions.SP_IMPU)) { %>
				<a href="psi.jsp?action=create" target="content"> Create </a> <br>
			<% } %> <br>
			
			</ul/>
		</td>
	</tr>
</table>

</body>