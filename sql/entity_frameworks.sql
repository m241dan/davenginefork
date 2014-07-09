SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_frameworks`;
CREATE TABLE IF NOT EXISTS `entity_frameworks` (
   `entityFrameworkID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(25) NOT NULL DEFAULT '',
   `short_descr` varchar(30) NOT NULL DEFAULT '',
   `long_descr` varchar(60) NOT NULL DEFAULT '',
   `description` text NOT NULL DEFAULT '',
   PRIMARY KEY (`entityFrameworkID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
