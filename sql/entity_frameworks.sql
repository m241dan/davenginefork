SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_frameworks`;
CREATE TABLE IF NOT EXISTS `entity_frameworks` (
   `entityFrameworkID` int NOT NULL DEFAULT '-1',
   `tagType` int NOT NULL DEFAULT '-1',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `name` varchar(255) NOT NULL DEFAULT '',
   `short_descr` varchar(255) NOT NULL DEFAULT '',
   `long_descr` varchar(255) NOT NULL DEFAULT '',
   `description` text NOT NULL DEFAULT '',
   PRIMARY KEY (`entityFrameworkID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `entity_frameworks` VALUES ( '0', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'North', 'North', 'Northern exit is here.', 'An exit to the north.' );
INSERT INTO `entity_frameworks` VALUES ( '1', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'South', 'South', 'Southern exit is here.', 'An exit to the south.' );
INSERT INTO `entity_frameworks` VALUES ( '2', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'East', 'East', 'Easthern exit is here.', 'An exit to the east.' );
INSERT INTO `entity_frameworks` VALUES ( '3', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'West', 'West', 'Westhern exit is here.', 'An exit to the west.' );
INSERT INTO `entity_frameworks` VALUES ( '4', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'Up', 'Up', 'Up exit is here.', 'An exit leading up.' );
INSERT INTO `entity_frameworks` VALUES ( '5', '2', 'Davenge', 'init_script', 'Davenge', 'init_script', 'Down', 'Down', 'Down exit is here.', 'An exit leaded down.' );

