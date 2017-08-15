-- MySQL dump 10.13  Distrib 5.7.19, for Linux (x86_64)
--
-- Host: localhost    Database: book_borrow_sys
-- ------------------------------------------------------
-- Server version	5.7.19

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
-- Table structure for table `accounts`
--

DROP TABLE IF EXISTS `accounts`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `accounts` (
  `number` varchar(100) NOT NULL,
  `passwd` varchar(20) DEFAULT NULL,
  `nickname` varchar(100) DEFAULT NULL,
  `sex` varchar(20) DEFAULT NULL,
  `address` varchar(100) DEFAULT NULL,
  `birthdate` varchar(20) DEFAULT NULL,
  `phone` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`number`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `accounts`
--

LOCK TABLES `accounts` WRITE;
/*!40000 ALTER TABLE `accounts` DISABLE KEYS */;
INSERT INTO `accounts` VALUES ('1','1','1','1','1','1','1'),('admin','admin','admin','admin','admin','admin','admin');
/*!40000 ALTER TABLE `accounts` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `book_infor`
--

DROP TABLE IF EXISTS `book_infor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `book_infor` (
  `ISBN` varchar(20) CHARACTER SET latin1 NOT NULL,
  `book_name` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `publish_house` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `author` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `count` int(11) DEFAULT NULL,
  `stat` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  PRIMARY KEY (`ISBN`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `book_infor`
--

LOCK TABLES `book_infor` WRITE;
/*!40000 ALTER TABLE `book_infor` DISABLE KEYS */;
INSERT INTO `book_infor` VALUES ('978-7-04-043197-1','é©¬å…‹æ€ä¸»ä¹‰åŸºæœ¬åŽŸç†æ¦‚è®º','é«˜ç­‰æ•™è‚²å‡ºç‰ˆç¤¾','axin',6,'yes'),('978-7-111-50069-8','æ·±å…¥åº”ç”¨C++11','æœºæ¢°å·¥ä¸šå‡ºç‰ˆç¤¾','ç¥å®‡',8,'no'),('978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','äººæ°‘é‚®ç”µå‡ºç‰ˆç¤¾','BenForta',66,'yes'),('978-7-115-36720-4','UNç½‘ç»œç¼–ç¨‹','äººæ°‘é‚®ç”µå‡ºç‰ˆç¤¾','.RichardStevens',88,'yes'),('978-7-302-18565-9','æ•°å€¼åˆ†æž','æ¸…åŽå¤§å­¦å‡ºç‰ˆç¤¾','æŽæ‰¬',8,'yes');
/*!40000 ALTER TABLE `book_infor` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `borrow_infor`
--

DROP TABLE IF EXISTS `borrow_infor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `borrow_infor` (
  `account` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  `ISBN` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  `book_name` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `borrow_date` datetime DEFAULT NULL,
  `ret_date` datetime DEFAULT NULL,
  KEY `borrow_fk` (`ISBN`),
  CONSTRAINT `borrow_fk` FOREIGN KEY (`ISBN`) REFERENCES `book_infor` (`ISBN`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `borrow_infor`
--

LOCK TABLES `borrow_infor` WRITE;
/*!40000 ALTER TABLE `borrow_infor` DISABLE KEYS */;
INSERT INTO `borrow_infor` VALUES ('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 21:49:26','2017-10-13 21:49:26'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-15 09:30:39','2017-08-18 09:30:39'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-15 10:27:30','2017-08-16 10:27:30');
/*!40000 ALTER TABLE `borrow_infor` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `ret_infor`
--

DROP TABLE IF EXISTS `ret_infor`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ret_infor` (
  `account` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  `ISBN` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  `book_name` varchar(100) CHARACTER SET latin1 DEFAULT NULL,
  `return_date` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `ret_infor`
--

LOCK TABLES `ret_infor` WRITE;
/*!40000 ALTER TABLE `ret_infor` DISABLE KEYS */;
INSERT INTO `ret_infor` VALUES ('1','1','1','2017-08-14 09:29:53'),('1','1','1','2017-08-14 09:29:56'),('1','1','1','2017-08-14 09:29:57'),('1','1','1','2017-08-14 09:29:59'),('1','1','1','2017-08-14 09:30:01'),('1','1','1','2017-08-14 09:30:03'),('1','1','1','2017-08-14 09:30:05'),('1','1','1','2017-08-14 09:30:06'),('1','1','1','2017-08-14 09:30:07'),('1','1','1','2017-08-14 09:30:09'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 17:45:04'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 17:45:11'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 17:46:27'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 19:16:25'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 19:16:31'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:26:50'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:26:55'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:04'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:09'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:15'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:24'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:33'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:39'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:44'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:27:55'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:21'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:28'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:33'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:38'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:44'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:28:51'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:29:14'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:29:45'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:29:51'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:29:58'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:30:06'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:30:12'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:30:17'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:30:22'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:30:27'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:31:24'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:31:39'),('1','978-7-115-19112-0','mysqlå¿…çŸ¥å¿…ä¼š','2017-08-14 20:36:42'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 20:40:25'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 20:44:19'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 21:55:24'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 21:57:13'),('1','978-7-302-18565-9','æ•°å€¼åˆ†æž','2017-08-14 21:59:07');
/*!40000 ALTER TABLE `ret_infor` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2017-08-15 10:39:45
