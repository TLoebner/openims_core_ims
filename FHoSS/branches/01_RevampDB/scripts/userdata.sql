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

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `hss_db` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `hss_db`;

--
-- Dumping data for table `application_server`
--


/*!40000 ALTER TABLE `application_server` DISABLE KEYS */;
LOCK TABLES `application_server` WRITE;
INSERT INTO `application_server` VALUES (1,'Presence AS','sip:open-ims.test:5065',0,'','',1024,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0),(3,'Unreg AS','sip:open-ims.test:5070',0,'','',1024,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `application_server` ENABLE KEYS */;

--
-- Dumping data for table `capabilities_set`
--


/*!40000 ALTER TABLE `capabilities_set` DISABLE KEYS */;
LOCK TABLES `capabilities_set` WRITE;
INSERT INTO `capabilities_set` VALUES (1,1,'cap_set',1,1),(2,1,'cap_set',2,0),(3,1,'cap_set',3,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `capabilities_set` ENABLE KEYS */;

--
-- Dumping data for table `capability`
--


/*!40000 ALTER TABLE `capability` DISABLE KEYS */;
LOCK TABLES `capability` WRITE;
INSERT INTO `capability` VALUES (1,'cap1'),(2,'cap2'),(3,'cap3');
UNLOCK TABLES;
/*!40000 ALTER TABLE `capability` ENABLE KEYS */;

--
-- Dumping data for table `charging_info`
--


/*!40000 ALTER TABLE `charging_info` DISABLE KEYS */;
LOCK TABLES `charging_info` WRITE;
INSERT INTO `charging_info` VALUES (1,'default_charging_set','pri_ecf_address','sec_ecf_address','pri_ccf_address','sec_ccf_address');
UNLOCK TABLES;
/*!40000 ALTER TABLE `charging_info` ENABLE KEYS */;

--
-- Dumping data for table `ifc`
--


/*!40000 ALTER TABLE `ifc` DISABLE KEYS */;
LOCK TABLES `ifc` WRITE;
INSERT INTO `ifc` VALUES (1,'default_ifc',1,1,0),(2,'unreg_ifc',3,-1,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `ifc` ENABLE KEYS */;

--
-- Dumping data for table `impi`
--


/*!40000 ALTER TABLE `impi` DISABLE KEYS */;
LOCK TABLES `impi` WRITE;
INSERT INTO `impi` VALUES (1,1,'alice@open-ims.test','alice',127,1,'\0\0','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','00000000101f','',''),(2,2,'bob@open-ims.test','bob',127,1,'\0\0','\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0','0000000000c5','','');
UNLOCK TABLES;
/*!40000 ALTER TABLE `impi` ENABLE KEYS */;

--
-- Dumping data for table `impi_impu`
--


/*!40000 ALTER TABLE `impi_impu` DISABLE KEYS */;
LOCK TABLES `impi_impu` WRITE;
INSERT INTO `impi_impu` VALUES (1,1,1,2),(2,2,2,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `impi_impu` ENABLE KEYS */;

--
-- Dumping data for table `impu`
--


/*!40000 ALTER TABLE `impu` DISABLE KEYS */;
LOCK TABLES `impu` WRITE;
INSERT INTO `impu` VALUES (1,'sip:alice@open-ims.test',0,0,2,1,1,1,'','',0,1),(2,'sip:bob@open-ims.test',0,0,0,1,2,1,'','',0,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `impu` ENABLE KEYS */;

--
-- Dumping data for table `impu_visited_network`
--


/*!40000 ALTER TABLE `impu_visited_network` DISABLE KEYS */;
LOCK TABLES `impu_visited_network` WRITE;
INSERT INTO `impu_visited_network` VALUES (1,1,1),(2,2,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `impu_visited_network` ENABLE KEYS */;

--
-- Dumping data for table `imsu`
--


/*!40000 ALTER TABLE `imsu` DISABLE KEYS */;
LOCK TABLES `imsu` WRITE;
INSERT INTO `imsu` VALUES (1,'alice','sip:scscf.open-ims.test:6060','scscf.open-ims.test',1,1),(2,'bob','','',1,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `imsu` ENABLE KEYS */;

--
-- Dumping data for table `preferred_scscf_set`
--


/*!40000 ALTER TABLE `preferred_scscf_set` DISABLE KEYS */;
LOCK TABLES `preferred_scscf_set` WRITE;
INSERT INTO `preferred_scscf_set` VALUES (1,1,'preferred_scscf_set','sip:scscf.open-ims.test:6060',0),(2,1,'preferred_scscf_set','sip:scscf2.open-ims.test:6060',1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `preferred_scscf_set` ENABLE KEYS */;

--
-- Dumping data for table `repository_data`
--


/*!40000 ALTER TABLE `repository_data` DISABLE KEYS */;
LOCK TABLES `repository_data` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `repository_data` ENABLE KEYS */;

--
-- Dumping data for table `rtr_ppr`
--


/*!40000 ALTER TABLE `rtr_ppr` DISABLE KEYS */;
LOCK TABLES `rtr_ppr` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `rtr_ppr` ENABLE KEYS */;

--
-- Dumping data for table `sh_notification`
--


/*!40000 ALTER TABLE `sh_notification` DISABLE KEYS */;
LOCK TABLES `sh_notification` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `sh_notification` ENABLE KEYS */;

--
-- Dumping data for table `sh_subscriptions`
--


/*!40000 ALTER TABLE `sh_subscriptions` DISABLE KEYS */;
LOCK TABLES `sh_subscriptions` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `sh_subscriptions` ENABLE KEYS */;

--
-- Dumping data for table `shared_ifc_set`
--


/*!40000 ALTER TABLE `shared_ifc_set` DISABLE KEYS */;
LOCK TABLES `shared_ifc_set` WRITE;
INSERT INTO `shared_ifc_set` VALUES (4,1,'shared_ifc_set',1,4),(5,1,'shared_ifc_set',2,6);
UNLOCK TABLES;
/*!40000 ALTER TABLE `shared_ifc_set` ENABLE KEYS */;

--
-- Dumping data for table `sp`
--


/*!40000 ALTER TABLE `sp` DISABLE KEYS */;
LOCK TABLES `sp` WRITE;
INSERT INTO `sp` VALUES (1,'default_sp',0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `sp` ENABLE KEYS */;

--
-- Dumping data for table `sp_ifc`
--


/*!40000 ALTER TABLE `sp_ifc` DISABLE KEYS */;
LOCK TABLES `sp_ifc` WRITE;
INSERT INTO `sp_ifc` VALUES (1,1,1,0),(2,1,2,1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `sp_ifc` ENABLE KEYS */;

--
-- Dumping data for table `sp_shared_ifc_set`
--


/*!40000 ALTER TABLE `sp_shared_ifc_set` DISABLE KEYS */;
LOCK TABLES `sp_shared_ifc_set` WRITE;
UNLOCK TABLES;
/*!40000 ALTER TABLE `sp_shared_ifc_set` ENABLE KEYS */;

--
-- Dumping data for table `spt`
--


/*!40000 ALTER TABLE `spt` DISABLE KEYS */;
LOCK TABLES `spt` WRITE;
INSERT INTO `spt` VALUES (1,1,0,0,1,NULL,'PUBLISH',NULL,NULL,NULL,NULL,NULL,0),(2,1,0,0,1,NULL,'SUBSCRIBE',NULL,NULL,NULL,NULL,NULL,0),(3,1,0,1,2,NULL,NULL,'Event','.*presence.*',NULL,NULL,NULL,0);
UNLOCK TABLES;
/*!40000 ALTER TABLE `spt` ENABLE KEYS */;

--
-- Dumping data for table `tp`
--


/*!40000 ALTER TABLE `tp` DISABLE KEYS */;
LOCK TABLES `tp` WRITE;
INSERT INTO `tp` VALUES (1,'presence_tp',1);
UNLOCK TABLES;
/*!40000 ALTER TABLE `tp` ENABLE KEYS */;

--
-- Dumping data for table `visited_network`
--


/*!40000 ALTER TABLE `visited_network` DISABLE KEYS */;
LOCK TABLES `visited_network` WRITE;
INSERT INTO `visited_network` VALUES (1,'open-ims.test');
UNLOCK TABLES;
/*!40000 ALTER TABLE `visited_network` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

