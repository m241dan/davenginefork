SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `live_specs`;
CREATE TABLE IF NOT EXISTS `live_specs` (
   `owner` varchar(255) NOT NULL DEFAULT ' ',
   `specType` int NOT NULL DEFAULT '-1',
   `value` int NOT NULL DEFAULT '-1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;






