-- MySQL dump 10.9
--
-- Host: localhost    Database: sip2ims
-- ------------------------------------------------------
-- Server version	4.1.20-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `sip2ims`
--

/*!40000 DROP DATABASE IF EXISTS `sip2ims`*/;

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `sip2ims` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `sip2ims`;

--
-- Table structure for table `acc`
--

DROP TABLE IF EXISTS `acc`;
CREATE TABLE `acc` (
  `id` int(11) NOT NULL auto_increment,
  `from_uid` varchar(64) default NULL,
  `to_uid` varchar(64) default NULL,
  `to_did` varchar(64) default NULL,
  `from_did` varchar(64) default NULL,
  `sip_from` varchar(255) default NULL,
  `sip_to` varchar(255) default NULL,
  `sip_status` varchar(128) default NULL,
  `sip_method` varchar(16) default NULL,
  `in_ruri` varchar(255) default NULL,
  `out_ruri` varchar(255) default NULL,
  `from_uri` varchar(255) default NULL,
  `to_uri` varchar(255) default NULL,
  `sip_callid` varchar(255) default NULL,
  `sip_cseq` int(11) default NULL,
  `digest_username` varchar(64) default NULL,
  `digest_realm` varchar(255) default NULL,
  `from_tag` varchar(128) default NULL,
  `to_tag` varchar(128) default NULL,
  `src_ip` int(10) unsigned default NULL,
  `src_port` smallint(5) unsigned default NULL,
  `request_timestamp` datetime NOT NULL default '0000-00-00 00:00:00',
  `response_timestamp` datetime NOT NULL default '0000-00-00 00:00:00',
  `flags` int(10) unsigned NOT NULL default '0',
  `attrs` varchar(255) default NULL,
  UNIQUE KEY `acc_id_key` (`id`),
  KEY `acc_cid_key` (`sip_callid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `attr_types`
--

DROP TABLE IF EXISTS `attr_types`;
CREATE TABLE `attr_types` (
  `name` varchar(32) NOT NULL default '',
  `rich_type` varchar(32) NOT NULL default 'string',
  `raw_type` int(11) NOT NULL default '2',
  `type_spec` varchar(255) default NULL,
  `description` varchar(255) default NULL,
  `default_flags` int(11) NOT NULL default '0',
  `flags` int(11) NOT NULL default '0',
  `priority` int(11) NOT NULL default '0',
  `access` int(11) NOT NULL default '0',
  `ordering` int(11) NOT NULL default '0',
  UNIQUE KEY `upt_idx1` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `cpl`
--

DROP TABLE IF EXISTS `cpl`;
CREATE TABLE `cpl` (
  `uid` varchar(64) NOT NULL default '',
  `cpl_xml` blob,
  `cpl_bin` blob,
  UNIQUE KEY `cpl_key` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `credentials`
--

DROP TABLE IF EXISTS `credentials`;
CREATE TABLE `credentials` (
  `auth_username` varchar(64) NOT NULL default '',
  `did` varchar(64) NOT NULL default '_none',
  `realm` varchar(64) NOT NULL default '',
  `password` varchar(28) NOT NULL default '',
  `flags` int(11) NOT NULL default '0',
  `ha1` varchar(32) NOT NULL default '',
  `ha1b` varchar(32) NOT NULL default '',
  `uid` varchar(64) NOT NULL default '',
  KEY `cred_idx` (`auth_username`,`did`),
  KEY `uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `customers`
--

DROP TABLE IF EXISTS `customers`;
CREATE TABLE `customers` (
  `cid` int(11) NOT NULL auto_increment,
  `name` varchar(128) NOT NULL default '',
  `address` varchar(255) default NULL,
  `phone` varchar(64) default NULL,
  `email` varchar(255) default NULL,
  UNIQUE KEY `cu_idx` (`cid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `domain`
--

DROP TABLE IF EXISTS `domain`;
CREATE TABLE `domain` (
  `did` varchar(64) NOT NULL default '',
  `domain` varchar(128) NOT NULL default '',
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `domain_idx` (`domain`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `domain_attrs`
--

DROP TABLE IF EXISTS `domain_attrs`;
CREATE TABLE `domain_attrs` (
  `did` varchar(64) default NULL,
  `name` varchar(32) NOT NULL default '',
  `type` int(11) NOT NULL default '0',
  `value` varchar(225) default NULL,
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `domain_attr_idx` (`did`,`name`,`value`),
  KEY `domain_did` (`did`,`flags`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `domain_settings`
--

DROP TABLE IF EXISTS `domain_settings`;
CREATE TABLE `domain_settings` (
  `did` varchar(64) NOT NULL default '',
  `filename` varchar(225) NOT NULL default '',
  `version` int(10) unsigned NOT NULL default '0',
  `timestamp` int(10) unsigned default NULL,
  `content` blob,
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `ds_id` (`did`,`filename`,`version`),
  KEY `ds_df` (`did`,`filename`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `global_attrs`
--

DROP TABLE IF EXISTS `global_attrs`;
CREATE TABLE `global_attrs` (
  `name` varchar(32) NOT NULL default '',
  `type` int(11) NOT NULL default '0',
  `value` varchar(255) default NULL,
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `global_attrs_idx` (`name`,`value`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `grp`
--

DROP TABLE IF EXISTS `grp`;
CREATE TABLE `grp` (
  `uid` varchar(64) NOT NULL default '',
  `grp` varchar(64) NOT NULL default '',
  `last_modified` datetime NOT NULL default '1970-01-01 00:00:00',
  KEY `grp_idx` (`uid`,`grp`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `gw`
--

DROP TABLE IF EXISTS `gw`;
CREATE TABLE `gw` (
  `gw_name` varchar(128) NOT NULL default '',
  `ip_addr` int(10) unsigned NOT NULL default '0',
  `port` smallint(5) unsigned default NULL,
  `uri_scheme` tinyint(3) unsigned default NULL,
  `transport` smallint(5) unsigned default NULL,
  `grp_id` int(11) NOT NULL default '0',
  UNIQUE KEY `gw_idx1` (`gw_name`),
  KEY `gw_idx2` (`grp_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `gw_grp`
--

DROP TABLE IF EXISTS `gw_grp`;
CREATE TABLE `gw_grp` (
  `grp_id` int(11) NOT NULL auto_increment,
  `grp_name` varchar(64) NOT NULL default '',
  UNIQUE KEY `gwgrp_idx` (`grp_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `i18n`
--

DROP TABLE IF EXISTS `i18n`;
CREATE TABLE `i18n` (
  `code` int(11) NOT NULL default '0',
  `reason_re` varchar(255) default NULL,
  `lang` varchar(32) NOT NULL default '',
  `new_reason` varchar(255) default NULL,
  UNIQUE KEY `i18n_uniq_idx` (`code`,`lang`),
  KEY `i18n_idx` (`code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `ipmatch`
--

DROP TABLE IF EXISTS `ipmatch`;
CREATE TABLE `ipmatch` (
  `ip` varchar(50) NOT NULL default '',
  `avp_val` varchar(30) default NULL,
  `mark` int(10) unsigned NOT NULL default '1',
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `ipmatch_idx` (`ip`,`mark`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `lcr`
--

DROP TABLE IF EXISTS `lcr`;
CREATE TABLE `lcr` (
  `prefix` varchar(16) NOT NULL default '',
  `from_uri` varchar(255) NOT NULL default '%',
  `grp_id` int(11) default NULL,
  `priority` int(11) default NULL,
  KEY `lcr_idx1` (`prefix`),
  KEY `lcr_idx2` (`from_uri`),
  KEY `lcr_idx3` (`grp_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `location`
--

DROP TABLE IF EXISTS `location`;
CREATE TABLE `location` (
  `uid` varchar(64) NOT NULL default '',
  `aor` varchar(255) NOT NULL default '',
  `contact` varchar(255) NOT NULL default '',
  `received` varchar(255) default NULL,
  `expires` datetime NOT NULL default '1970-01-01 00:00:00',
  `q` float NOT NULL default '1',
  `callid` varchar(255) default NULL,
  `cseq` int(10) unsigned default NULL,
  `flags` int(10) unsigned NOT NULL default '0',
  `user_agent` varchar(64) default NULL,
  `instance` varchar(255) default NULL,
  UNIQUE KEY `location_key` (`uid`,`contact`),
  KEY `location_contact` (`contact`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `missed_calls`
--

DROP TABLE IF EXISTS `missed_calls`;
CREATE TABLE `missed_calls` (
  `id` int(11) NOT NULL auto_increment,
  `from_uid` varchar(64) default NULL,
  `to_uid` varchar(64) default NULL,
  `to_did` varchar(64) default NULL,
  `from_did` varchar(64) default NULL,
  `sip_from` varchar(255) default NULL,
  `sip_to` varchar(255) default NULL,
  `sip_status` varchar(128) default NULL,
  `sip_method` varchar(16) default NULL,
  `in_ruri` varchar(255) default NULL,
  `out_ruri` varchar(255) default NULL,
  `from_uri` varchar(255) default NULL,
  `to_uri` varchar(255) default NULL,
  `sip_callid` varchar(255) default NULL,
  `sip_cseq` int(11) default NULL,
  `digest_username` varchar(64) default NULL,
  `digest_realm` varchar(255) default NULL,
  `from_tag` varchar(128) default NULL,
  `to_tag` varchar(128) default NULL,
  `src_ip` int(10) unsigned default NULL,
  `src_port` smallint(5) unsigned default NULL,
  `request_timestamp` datetime NOT NULL default '0000-00-00 00:00:00',
  `response_timestamp` datetime NOT NULL default '0000-00-00 00:00:00',
  `flags` int(10) unsigned NOT NULL default '0',
  `attrs` varchar(255) default NULL,
  UNIQUE KEY `mc_id_key` (`id`),
  KEY `mc_cid_key` (`sip_callid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `offline_winfo`
--

DROP TABLE IF EXISTS `offline_winfo`;
CREATE TABLE `offline_winfo` (
  `uid` varchar(64) NOT NULL default '',
  `watcher` varchar(255) NOT NULL default '',
  `events` varchar(64) NOT NULL default '',
  `domain` varchar(128) default NULL,
  `status` varchar(32) default NULL,
  `created_on` datetime NOT NULL default '2006-01-31 13:13:13',
  `expires_on` datetime NOT NULL default '2006-01-31 13:13:13',
  `dbid` int(10) unsigned NOT NULL auto_increment,
  KEY `offline_winfo_key` (`dbid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `pdt`
--

DROP TABLE IF EXISTS `pdt`;
CREATE TABLE `pdt` (
  `prefix` varchar(32) NOT NULL default '',
  `domain` varchar(255) NOT NULL default '',
  UNIQUE KEY `pdt_idx` (`prefix`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `phonebook`
--

DROP TABLE IF EXISTS `phonebook`;
CREATE TABLE `phonebook` (
  `id` int(11) NOT NULL auto_increment,
  `uid` varchar(64) NOT NULL default '',
  `fname` varchar(32) default NULL,
  `lname` varchar(32) default NULL,
  `sip_uri` varchar(255) NOT NULL default '',
  UNIQUE KEY `pb_idx` (`id`),
  KEY `pb_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `presentity`
--

DROP TABLE IF EXISTS `presentity`;
CREATE TABLE `presentity` (
  `presid` int(10) unsigned NOT NULL auto_increment,
  `uri` varchar(255) NOT NULL default '',
  `uid` varchar(64) NOT NULL default '',
  `pdomain` varchar(128) NOT NULL default '',
  UNIQUE KEY `presentity_key` (`presid`),
  KEY `presentity_key2` (`uri`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `presentity_contact`
--

DROP TABLE IF EXISTS `presentity_contact`;
CREATE TABLE `presentity_contact` (
  `contactid` int(10) unsigned NOT NULL auto_increment,
  `presid` int(10) unsigned NOT NULL default '0',
  `basic` varchar(32) NOT NULL default 'offline',
  `status` varchar(32) NOT NULL default '',
  `location` varchar(128) NOT NULL default '',
  `expires` datetime NOT NULL default '2004-05-28 21:32:15',
  `placeid` int(10) default NULL,
  `priority` float NOT NULL default '0.5',
  `contact` varchar(255) default NULL,
  `tupleid` varchar(64) NOT NULL default '',
  `prescaps` int(10) NOT NULL default '0',
  `etag` varchar(64) NOT NULL default '',
  `published_id` varchar(64) NOT NULL default '',
  UNIQUE KEY `pc_idx1` (`contactid`),
  KEY `presid_index` (`presid`),
  KEY `location_index` (`location`),
  KEY `placeid_index` (`placeid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `presentity_notes`
--

DROP TABLE IF EXISTS `presentity_notes`;
CREATE TABLE `presentity_notes` (
  `dbid` varchar(64) NOT NULL default '',
  `presid` int(10) unsigned NOT NULL default '0',
  `etag` varchar(64) NOT NULL default '',
  `note` varchar(128) NOT NULL default '',
  `lang` varchar(64) NOT NULL default '',
  `expires` datetime NOT NULL default '2005-12-07 08:13:15',
  UNIQUE KEY `pnotes_idx1` (`dbid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `presentity_persons`
--

DROP TABLE IF EXISTS `presentity_persons`;
CREATE TABLE `presentity_persons` (
  `dbid` varchar(64) NOT NULL default '',
  `presid` int(10) unsigned NOT NULL default '0',
  `etag` varchar(64) NOT NULL default '',
  `person_element` blob NOT NULL,
  `id` varchar(128) NOT NULL default '',
  `expires` datetime NOT NULL default '2005-12-07 08:13:15',
  UNIQUE KEY `prespersons_idx1` (`dbid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `rls_subscription`
--

DROP TABLE IF EXISTS `rls_subscription`;
CREATE TABLE `rls_subscription` (
  `id` varchar(48) NOT NULL default '',
  `doc_version` int(11) NOT NULL default '0',
  `dialog` blob NOT NULL,
  `expires` datetime NOT NULL default '2005-12-02 09:00:13',
  `status` int(11) NOT NULL default '0',
  `contact` varchar(255) NOT NULL default '',
  `uri` varchar(255) NOT NULL default '',
  `package` varchar(128) NOT NULL default '',
  `w_uri` varchar(255) NOT NULL default '',
  `xcap_params` blob NOT NULL,
  UNIQUE KEY `rls_subscription_key` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `rls_vs`
--

DROP TABLE IF EXISTS `rls_vs`;
CREATE TABLE `rls_vs` (
  `id` varchar(48) NOT NULL default '',
  `rls_id` varchar(48) NOT NULL default '',
  `uri` varchar(255) NOT NULL default '',
  UNIQUE KEY `rls_vs_key` (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `rls_vs_names`
--

DROP TABLE IF EXISTS `rls_vs_names`;
CREATE TABLE `rls_vs_names` (
  `id` varchar(48) NOT NULL default '',
  `name` varchar(64) default NULL,
  `lang` varchar(64) default NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `sd_attrs`
--

DROP TABLE IF EXISTS `sd_attrs`;
CREATE TABLE `sd_attrs` (
  `id` varchar(64) NOT NULL default '',
  `name` varchar(32) NOT NULL default '',
  `value` varchar(225) default NULL,
  `type` int(11) NOT NULL default '0',
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `sd_idx` (`id`,`name`,`value`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `silo`
--

DROP TABLE IF EXISTS `silo`;
CREATE TABLE `silo` (
  `mid` int(11) NOT NULL auto_increment,
  `from_hdr` varchar(255) NOT NULL default '',
  `to_hdr` varchar(255) NOT NULL default '',
  `ruri` varchar(255) NOT NULL default '',
  `uid` varchar(64) NOT NULL default '',
  `inc_time` datetime NOT NULL default '1970-01-01 00:00:00',
  `exp_time` datetime NOT NULL default '1970-01-01 00:00:00',
  `ctype` varchar(128) NOT NULL default 'text/plain',
  `body` blob NOT NULL,
  UNIQUE KEY `silo_idx1` (`mid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `speed_dial`
--

DROP TABLE IF EXISTS `speed_dial`;
CREATE TABLE `speed_dial` (
  `id` int(11) NOT NULL auto_increment,
  `uid` varchar(64) NOT NULL default '',
  `dial_username` varchar(64) NOT NULL default '',
  `dial_did` varchar(64) NOT NULL default '',
  `new_uri` varchar(255) NOT NULL default '',
  UNIQUE KEY `speeddial_idx1` (`uid`,`dial_did`,`dial_username`),
  UNIQUE KEY `speeddial_id` (`id`),
  KEY `speeddial_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `trusted`
--

DROP TABLE IF EXISTS `trusted`;
CREATE TABLE `trusted` (
  `src_ip` varchar(39) NOT NULL default '',
  `proto` varchar(4) NOT NULL default '',
  `from_pattern` varchar(64) NOT NULL default '',
  UNIQUE KEY `trusted_idx` (`src_ip`,`proto`,`from_pattern`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `tuple_notes`
--

DROP TABLE IF EXISTS `tuple_notes`;
CREATE TABLE `tuple_notes` (
  `presid` int(10) unsigned NOT NULL default '0',
  `tupleid` varchar(64) NOT NULL default '',
  `note` varchar(128) NOT NULL default '',
  `lang` varchar(64) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `uri`
--

DROP TABLE IF EXISTS `uri`;
CREATE TABLE `uri` (
  `uid` varchar(64) NOT NULL default '',
  `did` varchar(64) NOT NULL default '',
  `username` varchar(64) NOT NULL default '',
  `flags` int(10) unsigned NOT NULL default '0',
  `scheme` int(11) NOT NULL default '0',
  KEY `uri_idx1` (`username`,`did`),
  KEY `uri_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `uri_attrs`
--

DROP TABLE IF EXISTS `uri_attrs`;
CREATE TABLE `uri_attrs` (
  `username` varchar(64) NOT NULL default '',
  `did` varchar(64) NOT NULL default '',
  `name` varchar(32) NOT NULL default '',
  `value` varchar(100) default NULL,
  `type` int(11) NOT NULL default '0',
  `flags` int(10) unsigned NOT NULL default '0',
  `scheme` int(11) NOT NULL default '0',
  UNIQUE KEY `uriattrs_idx` (`username`,`did`,`name`,`value`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `user_attrs`
--

DROP TABLE IF EXISTS `user_attrs`;
CREATE TABLE `user_attrs` (
  `uid` varchar(64) NOT NULL default '',
  `name` varchar(32) NOT NULL default '',
  `value` varchar(225) default NULL,
  `type` int(11) NOT NULL default '0',
  `flags` int(10) unsigned NOT NULL default '0',
  UNIQUE KEY `userattrs_idx` (`uid`,`name`,`value`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `version`
--

DROP TABLE IF EXISTS `version`;
CREATE TABLE `version` (
  `table_name` varchar(32) NOT NULL default '',
  `table_version` int(10) unsigned NOT NULL default '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `watcherinfo`
--

DROP TABLE IF EXISTS `watcherinfo`;
CREATE TABLE `watcherinfo` (
  `r_uri` varchar(255) NOT NULL default '',
  `w_uri` varchar(255) NOT NULL default '',
  `display_name` varchar(128) NOT NULL default '',
  `s_id` varchar(64) NOT NULL default '',
  `package` varchar(32) NOT NULL default 'presence',
  `status` varchar(32) NOT NULL default 'pending',
  `event` varchar(32) NOT NULL default '',
  `expires` int(11) NOT NULL default '0',
  `accepts` int(11) NOT NULL default '0',
  `presid` int(10) unsigned NOT NULL default '0',
  `server_contact` varchar(255) NOT NULL default '',
  `dialog` blob NOT NULL,
  `doc_index` int(11) NOT NULL default '0',
  UNIQUE KEY `wi_idx1` (`s_id`),
  KEY `wi_ruri_idx` (`r_uri`),
  KEY `wi_wuri_idx` (`w_uri`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- MySQL dump 10.9
--
-- Host: localhost    Database: sip2ims
-- ------------------------------------------------------
-- Server version	4.1.20-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Dumping data for table `acc`
--


/*!40000 ALTER TABLE `acc` DISABLE KEYS */;
LOCK TABLES `acc` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `acc` ENABLE KEYS */;

--
-- Dumping data for table `attr_types`
--


/*!40000 ALTER TABLE `attr_types` DISABLE KEYS */;
LOCK TABLES `attr_types` WRITE;
INSERT INTO `attr_types` VALUES ('uid','string',2,NULL,NULL,1,0,0,0,0),('did','string',2,NULL,NULL,1,0,0,0,0),('datetime_created','string',2,NULL,'creation time',33,0,0,1,0),('asserted_id','string',2,NULL,'asserted identity',33,0,256,1,200),('fr_timer','int',0,NULL,'@ff_fr_timer',33,0,1073807616,0,140),('fr_inv_timer','int',0,NULL,'@ff_fr_inv_timer',33,0,1073807616,0,150),('gflags','int',0,NULL,'global flags',33,0,1073741824,0,0),('digest_realm','string',2,NULL,'digest realm',33,4096,65536,0,0),('acl','string',2,NULL,'access control list of user',33,1024,0,0,0),('first_name','string',2,NULL,'@ff_first_name',32,2048,256,0,10),('last_name','string',2,NULL,'@ff_last_name',32,2048,256,0,20),('email','email_adr',2,NULL,'@ff_email',33,6144,256,0,30),('timezone','timezone',2,NULL,'@ff_timezone',32,2048,1073807616,0,60),('sw_allow_find','boolean',0,NULL,'@ff_allow_lookup_for_me',32,0,256,0,110),('lang','lang',2,NULL,'@ff_language',33,0,1073807616,0,50),('sw_show_status','boolean',0,NULL,'@ff_status_visibility',32,0,1073742080,0,100),('sw_admin','string',2,NULL,'admin of domain',32,1024,0,0,0),('sw_owner','string',2,NULL,'owner of domain',32,0,0,0,0),('sw_domain_default_flags','int',0,NULL,'@ff_domain_def_f',32,4096,1073741824,0,0),('sw_deleted_ts','int',0,NULL,'deleted timestamp',32,0,0,0,0),('phone','string',2,NULL,'@ff_phone',32,2048,256,0,40),('sw_acl_control','string',2,NULL,'acl control',32,1024,0,0,0),('sw_credential_default_flags','int',0,NULL,'@ff_credential_def_f',32,4096,1073741824,0,0),('sw_uri_default_flags','int',0,NULL,'@ff_uri_def_f',32,4096,1073741824,0,0),('sw_is_admin','boolean',0,NULL,'admin privilege',32,0,0,0,0),('sw_is_hostmaster','boolean',0,NULL,'hostmaster privilege',32,0,0,0,0),('sw_confirmation','string',2,NULL,'registration confirmation',32,0,0,0,0),('sw_pending_ts','string',2,NULL,'registration timestamp',32,0,0,0,0),('sw_require_confirm','boolean',0,NULL,'@ff_reg_confirmation',32,0,1073807360,0,120),('sw_send_missed','boolean',0,NULL,'@ff_send_daily_missed_calls',32,0,1073807616,0,130),('uid_format','list',2,'a:3:{i:0;s:14:\"username@realm\";i:1;s:21:\"integer (incremental)\";i:2;s:15:\"UUID by RFC4122\";}','@ff_uid_format',32,0,1073741824,0,160),('did_format','list',2,'a:3:{i:0;s:11:\"domain name\";i:1;s:21:\"integer (incremental)\";i:2;s:15:\"UUID by RFC4122\";}','@ff_did_format',32,0,1073741824,0,170),('contact_email','email_adr',2,NULL,'@ff_contact_email',32,4096,1073807360,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `attr_types` ENABLE KEYS */;

--
-- Dumping data for table `cpl`
--


/*!40000 ALTER TABLE `cpl` DISABLE KEYS */;
LOCK TABLES `cpl` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `cpl` ENABLE KEYS */;

--
-- Dumping data for table `credentials`
--


/*!40000 ALTER TABLE `credentials` DISABLE KEYS */;
LOCK TABLES `credentials` WRITE;
INSERT INTO `credentials` VALUES ('alice','_none','open-ims.test','alice',1,'012d7aeb444b8c77be2756591f90accb','a3da98c9b36dde1f17e3f713225dc4d1','1'),('bob','_none','open-ims.test','bob',1,'d7404d478d7ea2d230739464f0314fbb','b0342795e048d7e9f4e2432285776dd3','2');
UNLOCK TABLES;
/*!40000 ALTER TABLE `credentials` ENABLE KEYS */;

--
-- Dumping data for table `customers`
--


/*!40000 ALTER TABLE `customers` DISABLE KEYS */;
LOCK TABLES `customers` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `customers` ENABLE KEYS */;

--
-- Dumping data for table `domain`
--


/*!40000 ALTER TABLE `domain` DISABLE KEYS */;
LOCK TABLES `domain` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `domain` ENABLE KEYS */;

--
-- Dumping data for table `domain_attrs`
--


/*!40000 ALTER TABLE `domain_attrs` DISABLE KEYS */;
LOCK TABLES `domain_attrs` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `domain_attrs` ENABLE KEYS */;

--
-- Dumping data for table `domain_settings`
--


/*!40000 ALTER TABLE `domain_settings` DISABLE KEYS */;
LOCK TABLES `domain_settings` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `domain_settings` ENABLE KEYS */;

--
-- Dumping data for table `global_attrs`
--


/*!40000 ALTER TABLE `global_attrs` DISABLE KEYS */;
LOCK TABLES `global_attrs` WRITE;
INSERT INTO `global_attrs` VALUES ('sw_domain_default_flags',0,'33',32),('sw_credential_default_flags',0,'33',32),('sw_uri_default_flags',0,'57',32),('sw_show_status',0,'1',32),('sw_require_conf',0,'1',32),('lang',2,'en',33),('sw_timezone',2,'Europe/Prague',32),('uid_format',2,'0',32),('did_format',2,'0',32);
UNLOCK TABLES;
/*!40000 ALTER TABLE `global_attrs` ENABLE KEYS */;

--
-- Dumping data for table `grp`
--


/*!40000 ALTER TABLE `grp` DISABLE KEYS */;
LOCK TABLES `grp` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `grp` ENABLE KEYS */;

--
-- Dumping data for table `gw`
--


/*!40000 ALTER TABLE `gw` DISABLE KEYS */;
LOCK TABLES `gw` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gw` ENABLE KEYS */;

--
-- Dumping data for table `gw_grp`
--


/*!40000 ALTER TABLE `gw_grp` DISABLE KEYS */;
LOCK TABLES `gw_grp` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `gw_grp` ENABLE KEYS */;

--
-- Dumping data for table `i18n`
--


/*!40000 ALTER TABLE `i18n` DISABLE KEYS */;
LOCK TABLES `i18n` WRITE;
INSERT INTO `i18n` VALUES (100,NULL,'en_US.ascii','Trying'),(180,NULL,'en_US.ascii','Ringing'),(181,NULL,'en_US.ascii','Call Is Being Forwarded'),(182,NULL,'en_US.ascii','Queued'),(183,NULL,'en_US.ascii','Session Progress'),(200,NULL,'en_US.ascii','OK'),(202,NULL,'en_US.ascii','Pending'),(300,NULL,'en_US.ascii','Multiple Choices'),(301,NULL,'en_US.ascii','Moved Permanently'),(302,NULL,'en_US.ascii','Moved Temporarily'),(305,NULL,'en_US.ascii','Use Proxy'),(380,NULL,'en_US.ascii','Alternative Service'),(400,NULL,'en_US.ascii','Bad Request'),(401,NULL,'en_US.ascii','Unauthorized'),(402,NULL,'en_US.ascii','Payment Required'),(403,NULL,'en_US.ascii','Forbidden'),(404,NULL,'en_US.ascii','Not Found'),(405,NULL,'en_US.ascii','Method Not Allowed'),(406,NULL,'en_US.ascii','Not Acceptable'),(407,NULL,'en_US.ascii','Proxy Authentication Required'),(408,NULL,'en_US.ascii','Request Timeout'),(410,NULL,'en_US.ascii','Gone'),(413,NULL,'en_US.ascii','Request Entity Too Large'),(414,NULL,'en_US.ascii','Request-URI Too Long'),(415,NULL,'en_US.ascii','Unsupported Media Type'),(416,NULL,'en_US.ascii','Unsupported URI Scheme'),(420,NULL,'en_US.ascii','Bad Extension'),(421,NULL,'en_US.ascii','Extension Required'),(423,NULL,'en_US.ascii','Interval Too Brief'),(480,NULL,'en_US.ascii','Temporarily Unavailable'),(481,NULL,'en_US.ascii','Call/Transaction Does Not Exist'),(482,NULL,'en_US.ascii','Loop Detected'),(483,NULL,'en_US.ascii','Too Many Hops'),(484,NULL,'en_US.ascii','Address Incomplete'),(485,NULL,'en_US.ascii','Ambiguous'),(486,NULL,'en_US.ascii','Busy Here'),(487,NULL,'en_US.ascii','Request Terminated'),(488,NULL,'en_US.ascii','Not Acceptable Here'),(491,NULL,'en_US.ascii','Request Pending'),(493,NULL,'en_US.ascii','Undecipherable'),(500,NULL,'en_US.ascii','Server Internal Error'),(501,NULL,'en_US.ascii','Not Implemented'),(502,NULL,'en_US.ascii','Bad Gateway'),(503,NULL,'en_US.ascii','Service Unavailable'),(504,NULL,'en_US.ascii','Server Time-out'),(505,NULL,'en_US.ascii','Version Not Supported'),(513,NULL,'en_US.ascii','Message Too Large'),(600,NULL,'en_US.ascii','Busy Everywhere'),(603,NULL,'en_US.ascii','Decline'),(604,NULL,'en_US.ascii','Does Not Exist Anywhere'),(606,NULL,'en_US.ascii','Not Acceptable'),(100,NULL,'cs_CZ.ascii','Navazuji spojeni'),(180,NULL,'cs_CZ.ascii','Vyzvani'),(181,NULL,'cs_CZ.ascii','Hovor je presmerovan'),(182,NULL,'cs_CZ.ascii','Jste v poradi'),(183,NULL,'cs_CZ.ascii','Probiha navazovani spojeni'),(200,NULL,'cs_CZ.ascii','Uspesne provedeno'),(202,NULL,'cs_CZ.ascii','Bude vyrizeno pozdeji'),(300,NULL,'cs_CZ.ascii','Existuje vice moznosti'),(301,NULL,'cs_CZ.ascii','Trvale presmerovano'),(302,NULL,'cs_CZ.ascii','Docasne presmerovano'),(305,NULL,'cs_CZ.ascii','Pouzijte jiny server'),(380,NULL,'cs_CZ.ascii','Alternativni sluzba'),(400,NULL,'cs_CZ.ascii','Chyba protokolu'),(401,NULL,'cs_CZ.ascii','Overeni totoznosti'),(402,NULL,'cs_CZ.ascii','Placena sluzba'),(403,NULL,'cs_CZ.ascii','Zakazano'),(404,NULL,'cs_CZ.ascii','Nenalezeno'),(405,NULL,'cs_CZ.ascii','Nepovoleny prikaz'),(406,NULL,'cs_CZ.ascii','Neni povoleno'),(407,NULL,'cs_CZ.ascii','Server vyzaduje overeni totoznosti'),(408,NULL,'cs_CZ.ascii','Casovy limit vyprsel'),(410,NULL,'cs_CZ.ascii','Nenalezeno'),(413,NULL,'cs_CZ.ascii','Prilis dlouhy identifikator'),(414,NULL,'cs_CZ.ascii','Request-URI je prilis dlouhe'),(415,NULL,'cs_CZ.ascii','Nepodporovany typ dat'),(416,NULL,'cs_CZ.ascii','Nepodporovany typ identifikatoru'),(420,NULL,'cs_CZ.ascii','Neplatne cislo linky'),(421,NULL,'cs_CZ.ascii','Zadejte cislo linky'),(423,NULL,'cs_CZ.ascii','Prilis kratky interval'),(480,NULL,'cs_CZ.ascii','Docasne nedostupne'),(481,NULL,'cs_CZ.ascii','Spojeni nenalezeno'),(482,NULL,'cs_CZ.ascii','Zprava se zacyklila'),(483,NULL,'cs_CZ.ascii','Prilis mnoho kroku'),(484,NULL,'cs_CZ.ascii','Neuplna adresa'),(485,NULL,'cs_CZ.ascii','Nejednoznacne'),(486,NULL,'cs_CZ.ascii','Volany je zaneprazdnen'),(487,NULL,'cs_CZ.ascii','Prikaz predcasne ukoncen'),(488,NULL,'cs_CZ.ascii','Nebylo akceptovano'),(491,NULL,'cs_CZ.ascii','Cekam na odpoved'),(493,NULL,'cs_CZ.ascii','Nelze dekodovat'),(500,NULL,'cs_CZ.ascii','Interni chyba serveru'),(501,NULL,'cs_CZ.ascii','Neni implementovano'),(502,NULL,'cs_CZ.ascii','Chybna brana'),(503,NULL,'cs_CZ.ascii','Sluzba neni dostupna'),(504,NULL,'cs_CZ.ascii','Casovy limit serveru vyprsel'),(505,NULL,'cs_CZ.ascii','Nepodporovana verze protokolu'),(513,NULL,'cs_CZ.ascii','Zprava je prilis dlouha'),(600,NULL,'cs_CZ.ascii','Uzivatel je zaneprazdnen'),(603,NULL,'cs_CZ.ascii','Odmitnuto'),(604,NULL,'cs_CZ.ascii','Neexistujici uzivatel nebo sluzba'),(606,NULL,'cs_CZ.ascii','Nelze akceptovat');
UNLOCK TABLES;
/*!40000 ALTER TABLE `i18n` ENABLE KEYS */;

--
-- Dumping data for table `ipmatch`
--


/*!40000 ALTER TABLE `ipmatch` DISABLE KEYS */;
LOCK TABLES `ipmatch` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `ipmatch` ENABLE KEYS */;

--
-- Dumping data for table `lcr`
--


/*!40000 ALTER TABLE `lcr` DISABLE KEYS */;
LOCK TABLES `lcr` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `lcr` ENABLE KEYS */;

--
-- Dumping data for table `location`
--


/*!40000 ALTER TABLE `location` DISABLE KEYS */;
LOCK TABLES `location` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `location` ENABLE KEYS */;

--
-- Dumping data for table `missed_calls`
--


/*!40000 ALTER TABLE `missed_calls` DISABLE KEYS */;
LOCK TABLES `missed_calls` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `missed_calls` ENABLE KEYS */;

--
-- Dumping data for table `offline_winfo`
--


/*!40000 ALTER TABLE `offline_winfo` DISABLE KEYS */;
LOCK TABLES `offline_winfo` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `offline_winfo` ENABLE KEYS */;

--
-- Dumping data for table `pdt`
--


/*!40000 ALTER TABLE `pdt` DISABLE KEYS */;
LOCK TABLES `pdt` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `pdt` ENABLE KEYS */;

--
-- Dumping data for table `phonebook`
--


/*!40000 ALTER TABLE `phonebook` DISABLE KEYS */;
LOCK TABLES `phonebook` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `phonebook` ENABLE KEYS */;

--
-- Dumping data for table `presentity`
--


/*!40000 ALTER TABLE `presentity` DISABLE KEYS */;
LOCK TABLES `presentity` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `presentity` ENABLE KEYS */;

--
-- Dumping data for table `presentity_contact`
--


/*!40000 ALTER TABLE `presentity_contact` DISABLE KEYS */;
LOCK TABLES `presentity_contact` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `presentity_contact` ENABLE KEYS */;

--
-- Dumping data for table `presentity_notes`
--


/*!40000 ALTER TABLE `presentity_notes` DISABLE KEYS */;
LOCK TABLES `presentity_notes` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `presentity_notes` ENABLE KEYS */;

--
-- Dumping data for table `presentity_persons`
--


/*!40000 ALTER TABLE `presentity_persons` DISABLE KEYS */;
LOCK TABLES `presentity_persons` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `presentity_persons` ENABLE KEYS */;

--
-- Dumping data for table `rls_subscription`
--


/*!40000 ALTER TABLE `rls_subscription` DISABLE KEYS */;
LOCK TABLES `rls_subscription` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `rls_subscription` ENABLE KEYS */;

--
-- Dumping data for table `rls_vs`
--


/*!40000 ALTER TABLE `rls_vs` DISABLE KEYS */;
LOCK TABLES `rls_vs` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `rls_vs` ENABLE KEYS */;

--
-- Dumping data for table `rls_vs_names`
--


/*!40000 ALTER TABLE `rls_vs_names` DISABLE KEYS */;
LOCK TABLES `rls_vs_names` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `rls_vs_names` ENABLE KEYS */;

--
-- Dumping data for table `sd_attrs`
--


/*!40000 ALTER TABLE `sd_attrs` DISABLE KEYS */;
LOCK TABLES `sd_attrs` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `sd_attrs` ENABLE KEYS */;

--
-- Dumping data for table `silo`
--


/*!40000 ALTER TABLE `silo` DISABLE KEYS */;
LOCK TABLES `silo` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `silo` ENABLE KEYS */;

--
-- Dumping data for table `speed_dial`
--


/*!40000 ALTER TABLE `speed_dial` DISABLE KEYS */;
LOCK TABLES `speed_dial` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `speed_dial` ENABLE KEYS */;

--
-- Dumping data for table `trusted`
--


/*!40000 ALTER TABLE `trusted` DISABLE KEYS */;
LOCK TABLES `trusted` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `trusted` ENABLE KEYS */;

--
-- Dumping data for table `tuple_notes`
--


/*!40000 ALTER TABLE `tuple_notes` DISABLE KEYS */;
LOCK TABLES `tuple_notes` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `tuple_notes` ENABLE KEYS */;

--
-- Dumping data for table `uri`
--


/*!40000 ALTER TABLE `uri` DISABLE KEYS */;
LOCK TABLES `uri` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `uri` ENABLE KEYS */;

--
-- Dumping data for table `uri_attrs`
--


/*!40000 ALTER TABLE `uri_attrs` DISABLE KEYS */;
LOCK TABLES `uri_attrs` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `uri_attrs` ENABLE KEYS */;

--
-- Dumping data for table `user_attrs`
--


/*!40000 ALTER TABLE `user_attrs` DISABLE KEYS */;
LOCK TABLES `user_attrs` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `user_attrs` ENABLE KEYS */;

--
-- Dumping data for table `version`
--


/*!40000 ALTER TABLE `version` DISABLE KEYS */;
LOCK TABLES `version` WRITE;
INSERT INTO `version` VALUES ('acc',3),('missed_calls',3),('credentials',7),('attr_types',3),('global_attrs',1),('domain_attrs',1),('user_attrs',3),('uri_attrs',1),('domain',2),('domain_settings',1),('location',9),('trusted',1),('ipmatch',1),('phonebook',1),('gw',3),('gw_grp',2),('lcr',1),('grp',3),('silo',4),('uri',2),('speed_dial',2),('sd_attrs',1),('presentity',1),('presentity_notes',1),('presentity_persons',1),('presentity_contact',1),('watcherinfo',1),('tuple_notes',1),('offline_winfo',1),('rls_subscription',1),('rls_vs',1),('rls_vs_names',1),('i18n',1),('pdt',1),('cpl',2),('customers',1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `version` ENABLE KEYS */;

--
-- Dumping data for table `watcherinfo`
--


/*!40000 ALTER TABLE `watcherinfo` DISABLE KEYS */;
LOCK TABLES `watcherinfo` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `watcherinfo` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

grant delete,insert,select,update on sip2ims.* to sip2ims@localhost identified by 'heslo';
grant delete,insert,select,update on sip2ims.* to provisioning@localhost identified by 'provi';
