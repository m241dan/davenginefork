SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `stat_frameworks`;
CREATE TABLE IF NOT EXISTS `stat_frameworks` (
   `statFrameworkID` int NOT NULL DEFAULT '-1',
   `name` varchar(255) NOT NULL DEFAULT ' ',
   `softcap` int NOT NULL DEFAULT '0 ',
   `hardcap`  int NOT NULL DEFAULT '0',
   `softfloor`  int NOT NULL DEFAULT '0',
   `hardfloor`  int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
