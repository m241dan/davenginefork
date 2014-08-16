SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `live_specs`;
CREATE TABLE IF NOT EXISTS `live_specs` (
   `specType` varchar(255) NOT NULL DEFAULT ' ',
   `value` int NOT NULL DEFAULT '-1',
   `owner` varchar(255) NOT NULL DEFAULT ' '
) ENGINE=InnoDB DEFAULT CHARSET=utf8;






