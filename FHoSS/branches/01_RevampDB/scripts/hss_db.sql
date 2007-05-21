-- MySQL dump 10.10
--
-- Host: localhost    Database: hss_db
-- ------------------------------------------------------
-- Server version	5.0.21-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `hss_db`
--

/*!40000 DROP DATABASE IF EXISTS `hss_db`*/;

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `hss_db` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `hss_db`;

--
-- Table structure for table `application_server`
--

DROP TABLE IF EXISTS `application_server`;
CREATE TABLE `application_server` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `server_name` varchar(255) NOT NULL default '',
  `default_handling` int(11) NOT NULL default '0',
  `service_info` varchar(255) NOT NULL default '',
  `diameter_address` varchar(255) NOT NULL default '',
  `rep_data_size_limit` int(11) NOT NULL default '0',
  `udr` tinyint(4) NOT NULL default '0',
  `pur` tinyint(4) NOT NULL default '0',
  `snr` tinyint(4) NOT NULL default '0',
  `udr_rep_data` tinyint(4) NOT NULL default '0',
  `udr_impu` tinyint(4) NOT NULL default '0',
  `udr_ims_user_state` tinyint(4) NOT NULL default '0',
  `udr_scscf_name` tinyint(4) NOT NULL default '0',
  `udr_ifc` tinyint(4) NOT NULL default '0',
  `udr_location` tinyint(4) NOT NULL default '0',
  `udr_user_state` tinyint(4) NOT NULL default '0',
  `udr_charging_info` tinyint(4) NOT NULL default '0',
  `udr_msisdn` tinyint(4) NOT NULL default '0',
  `udr_psi_activation` tinyint(4) NOT NULL default '0',
  `udr_dsai` tinyint(4) NOT NULL default '0',
  `udr_aliases_rep_data` tinyint(4) NOT NULL default '0',
  `pur_rep_data` tinyint(4) NOT NULL default '0',
  `pur_psi_activation` tinyint(4) NOT NULL default '0',
  `pur_dsai` tinyint(4) NOT NULL default '0',
  `pur_aliases_rep_data` tinyint(4) NOT NULL default '0',
  `snr_rep_data` tinyint(4) NOT NULL default '0',
  `snr_impu` tinyint(4) NOT NULL default '0',
  `snr_ims_user_state` tinyint(4) NOT NULL default '0',
  `snr_scscf_name` tinyint(4) NOT NULL default '0',
  `snr_ifc` tinyint(4) NOT NULL default '0',
  `snr_psi_activation` tinyint(4) NOT NULL default '0',
  `snr_dsai` tinyint(4) NOT NULL default '0',
  `snr_aliases_rep_data` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_diameter_address` (`diameter_address`),
  KEY `idx_server_name` (`server_name`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Application Servers';

--
-- Table structure for table `capabilities_set`
--

DROP TABLE IF EXISTS `capabilities_set`;
CREATE TABLE `capabilities_set` (
  `id` int(11) NOT NULL auto_increment,
  `id_set` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `id_capability` int(11) NOT NULL default '0',
  `is_mandatory` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_capability` (`id_capability`),
  KEY `idx_id_set` USING BTREE (`id_set`),
  KEY `idx_name` USING BTREE (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Capabilities Sets';

--
-- Table structure for table `capability`
--

DROP TABLE IF EXISTS `capability`;
CREATE TABLE `capability` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Capabilities Definition';

--
-- Table structure for table `charging_info`
--

DROP TABLE IF EXISTS `charging_info`;
CREATE TABLE `charging_info` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `pri_ecf` varchar(255) NOT NULL default '',
  `sec_ecf` varchar(255) NOT NULL default '',
  `pri_ccf` varchar(255) NOT NULL default '',
  `sec_ccf` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Charging Information Templates';

--
-- Table structure for table `ifc`
--

DROP TABLE IF EXISTS `ifc`;
CREATE TABLE `ifc` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `id_application_server` int(11) default NULL,
  `id_tp` int(11) default NULL,
  `profile_part_ind` int(11) default NULL,
  PRIMARY KEY  (`id`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Initial Filter Criteria';

--
-- Table structure for table `impi`
--

DROP TABLE IF EXISTS `impi`;
CREATE TABLE `impi` (
  `id` int(11) NOT NULL auto_increment,
  `id_imsu` int(11) default NULL,
  `identity` varchar(255) NOT NULL default '',
  `k` varchar(255) NOT NULL default '',
  `auth_scheme` int(11) NOT NULL default '0',
  `default_auth_scheme` int(11) NOT NULL default '1',
  `amf` tinyblob NOT NULL,
  `op` tinyblob NOT NULL,
  `sqn` varchar(64) NOT NULL default '0',
  `ip` varchar(64) NOT NULL default '',
  `line_identifier` varchar(64) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `idx_identity` (`identity`),
  KEY `idx_id_imsu` (`id_imsu`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='IM Private Identities table';

--
-- Table structure for table `impi_impu`
--

DROP TABLE IF EXISTS `impi_impu`;
CREATE TABLE `impi_impu` (
  `id` int(11) NOT NULL auto_increment,
  `id_impi` int(11) NOT NULL default '0',
  `id_impu` int(11) NOT NULL default '0',
  `user_state` tinyint(4) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_impi` (`id_impi`),
  KEY `idx_id_impu` (`id_impu`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='IM Private/Public Identities Mappings';

--
-- Table structure for table `impu`
--

DROP TABLE IF EXISTS `impu`;
CREATE TABLE `impu` (
  `id` int(11) NOT NULL auto_increment,
  `identity` varchar(255) NOT NULL default '',
  `type` tinyint(4) NOT NULL default '0',
  `barring` tinyint(4) NOT NULL default '0',
  `user_state` tinyint(4) NOT NULL default '0',
  `id_sp` int(11) default NULL,
  `id_implicit_set` int(11) NOT NULL default '0',
  `id_charging_info` int(11) default NULL,
  `wildcard_psi` varchar(255) NOT NULL default '',
  `display_name` varchar(255) NOT NULL default '',
  `psi_activation` tinyint(4) NOT NULL default '0',
  `can_register` tinyint(4) NOT NULL default '1',
  PRIMARY KEY  (`id`),
  KEY `idx_identity` (`identity`),
  KEY `idx_id_impu_implicitset` (`id_implicit_set`),
  KEY `idx_type` (`type`),
  KEY `idx_sp` (`id_sp`),
  KEY `idx_wildcard_psi` (`wildcard_psi`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='IM Public Identities';

--
-- Table structure for table `impu_visited_network`
--

DROP TABLE IF EXISTS `impu_visited_network`;
CREATE TABLE `impu_visited_network` (
  `id` int(11) NOT NULL auto_increment,
  `id_impu` int(11) NOT NULL default '0',
  `id_visited_network` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_impu` (`id_impu`),
  KEY `idx_visited_network` (`id_visited_network`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Public Identity - Visited Network mappings';

--
-- Table structure for table `imsu`
--

DROP TABLE IF EXISTS `imsu`;
CREATE TABLE `imsu` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `scscf_name` varchar(255) default NULL,
  `diameter_name` varchar(255) default NULL,
  `id_capabilities_set` int(11) default NULL,
  `id_preferred_scscf_set` int(11) default NULL,
  PRIMARY KEY  (`id`),
  KEY `idx_capabilities_set` (`id_capabilities_set`),
  KEY `idx_preferred_scscf` (`id_preferred_scscf_set`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='IMS Subscription';

--
-- Table structure for table `preferred_scscf_set`
--

DROP TABLE IF EXISTS `preferred_scscf_set`;
CREATE TABLE `preferred_scscf_set` (
  `id` int(11) NOT NULL auto_increment,
  `id_set` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `scscf_name` varchar(255) NOT NULL default '',
  `priority` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_priority` (`priority`),
  KEY `idx_set` USING BTREE (`id_set`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Preferred S-CSCF sets';

--
-- Table structure for table `repository_data`
--

DROP TABLE IF EXISTS `repository_data`;
CREATE TABLE `repository_data` (
  `id` int(11) NOT NULL auto_increment,
  `sqn` int(11) NOT NULL default '0',
  `id_impu` int(11) NOT NULL default '0',
  `service_indication` varchar(255) NOT NULL default '',
  `rep_data` blob,
  PRIMARY KEY  (`id`),
  KEY `idx_id_impu` (`id_impu`),
  KEY `idx_sqn` (`sqn`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Repository Data';

--
-- Table structure for table `rtr_ppr`
--

DROP TABLE IF EXISTS `rtr_ppr`;
CREATE TABLE `rtr_ppr` (
  `id` int(11) NOT NULL auto_increment,
  `hopbyhop` bigint(20) default NULL,
  `endtoend` bigint(20) default NULL,
  `id_impu` int(11) default NULL,
  `id_impi` int(11) default NULL,
  `type` tinyint(1) NOT NULL default '0',
  `subtype` tinyint(4) NOT NULL default '0',
  `grp` int(11) default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_hopbyhop` USING BTREE (`hopbyhop`),
  KEY `idx_endtoend` (`endtoend`),
  KEY `idx_type` (`type`),
  KEY `idx_grp` (`grp`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Cx interface RTR and PPR events';

--
-- Table structure for table `sh_notification`
--

DROP TABLE IF EXISTS `sh_notification`;
CREATE TABLE `sh_notification` (
  `id` int(11) NOT NULL auto_increment,
  `id_impu` int(11) NOT NULL default '0',
  `id_application_server` int(11) NOT NULL default '0',
  `type` tinyint(4) NOT NULL default '0',
  `rep_data` blob NOT NULL,
  `id_ifc` int(11) NOT NULL default '0',
  `scscf_name` varchar(255) NOT NULL default '',
  `reg_state` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_impu` (`id_impu`),
  KEY `idx_as` (`id_application_server`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Sh Interface Notifications';

--
-- Table structure for table `sh_subscriptions`
--

DROP TABLE IF EXISTS `sh_subscriptions`;
CREATE TABLE `sh_subscriptions` (
  `id` int(11) NOT NULL auto_increment,
  `id_application_server` int(11) NOT NULL default '0',
  `id_impi` int(11) default NULL,
  `id_impu` int(11) default NULL,
  `data_ref` int(11) NOT NULL default '0',
  `expires` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_impi` (`id_impi`),
  KEY `idx_id_impu` (`id_impu`),
  KEY `idx_id_as` USING BTREE (`id_application_server`),
  KEY `idx_expires` (`expires`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Sh Interface Subscriptions';

--
-- Table structure for table `shared_ifc_set`
--

DROP TABLE IF EXISTS `shared_ifc_set`;
CREATE TABLE `shared_ifc_set` (
  `id` int(11) NOT NULL auto_increment,
  `id_set` int(11) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `id_ifc` int(11) default NULL,
  `priority` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_set` (`id_set`),
  KEY `idx_priority` (`priority`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Shared IFC Sets';

--
-- Table structure for table `sp`
--

DROP TABLE IF EXISTS `sp`;
CREATE TABLE `sp` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(16) NOT NULL default '',
  `cn_service_auth` int(11) default NULL,
  PRIMARY KEY  (`id`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Service Profiles';

--
-- Table structure for table `sp_ifc`
--

DROP TABLE IF EXISTS `sp_ifc`;
CREATE TABLE `sp_ifc` (
  `id` int(11) NOT NULL auto_increment,
  `id_sp` int(11) NOT NULL default '0',
  `id_ifc` int(11) NOT NULL default '0',
  `priority` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `id_sp` (`id_sp`),
  KEY `id_ifc` (`id_ifc`),
  KEY `idx_priority` (`priority`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Service Profile - IFC mappings';

--
-- Table structure for table `sp_shared_ifc_set`
--

DROP TABLE IF EXISTS `sp_shared_ifc_set`;
CREATE TABLE `sp_shared_ifc_set` (
  `id` int(11) NOT NULL auto_increment,
  `id_sp` int(11) NOT NULL default '0',
  `id_shared_ifc_set` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_id_sp` (`id_sp`),
  KEY `idx_id_shared_ifc_set` (`id_shared_ifc_set`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Service Profile - Shared IFC Sets mappings';

--
-- Table structure for table `spt`
--

DROP TABLE IF EXISTS `spt`;
CREATE TABLE `spt` (
  `id` int(11) NOT NULL auto_increment,
  `id_tp` int(11) NOT NULL default '0',
  `condition_negated` int(11) NOT NULL default '0',
  `grp` int(11) NOT NULL default '0',
  `type` int(11) NOT NULL default '0',
  `requesturi` varchar(255) default NULL,
  `method` varchar(255) default NULL,
  `header` varchar(255) default NULL,
  `header_content` varchar(255) default NULL,
  `session_case` int(11) default NULL,
  `sdp_line` varchar(255) default NULL,
  `sdp_line_content` varchar(255) default NULL,
  `registration_type` int(11) default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_trigger_point` (`id_tp`),
  KEY `idx_grp` (`grp`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Service Point Trigger';

--
-- Table structure for table `tp`
--

DROP TABLE IF EXISTS `tp`;
CREATE TABLE `tp` (
  `id` int(11) NOT NULL auto_increment,
  `name` varchar(255) NOT NULL default '',
  `condition_type_cnf` int(11) NOT NULL default '0',
  PRIMARY KEY  (`id`),
  KEY `idx_name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Trigger Points';

--
-- Table structure for table `visited_network`
--

DROP TABLE IF EXISTS `visited_network`;
CREATE TABLE `visited_network` (
  `id` int(11) NOT NULL auto_increment,
  `identity` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`id`),
  KEY `idx_identity` (`identity`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Visited Networks';
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

# DB access rights
grant delete,insert,select,update on hss_db.* to hss@localhost identified by 'hss';
