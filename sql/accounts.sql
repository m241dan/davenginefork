SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `accounts`;
CREATE TABLE IF NOT EXISTS `accounts` (
   `accountID` int NOT NULL DEFAULT '-1',
   `name` varchar(25) NOT NULL DEFAULT '',
   `password` varchar(255) NOT NULL DEFAULT '',
   `level` smallint NOT NULL DEFAULT '0',
   `pagewidth` smallint NOT NULL DEFAULT '80',
   PRIMARY KEY (`accountID`),
   UNIQUE INDEX `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
