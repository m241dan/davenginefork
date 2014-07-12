SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `entity_frameworks_specs`;
CREATE TABLE IF NOT EXISTS `entity_frameworks_specs` (
   `entityFrameworkID` int NOT NULL DEFAULT '-1',
   `specType` int NOT NULL DEFAULT '-1',
   `value` int NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;






