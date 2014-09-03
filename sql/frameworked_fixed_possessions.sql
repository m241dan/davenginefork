SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `framework_fixed_possessions`;
CREATE TABLE IF NOT EXISTS `framework_fixed_possessions` (
   `frameworkID` int NOT NULL DEFAULT '-1',
   `content_frameworkID` int NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

