SET FOREIGN_KEY_CHECKS=0;

DROP TABLE IF EXISTS `workspaces`;
CREATE TABLE IF NOT EXISTS `workspaces` (
   `type` int NOT NULL DEFAULT '-1',
   `name` varchar(20) NOT NULL DEFAULT 'unknown',
   `description` text NOT NULL DEFAULT '',
   `created_by` varchar(50) NOT NULL DEFAULT 'system',
   `created_on` varchar(35) NOT NULL DEFAULT '',
   `modified_by` varchar(50) NOT NULL DEFAULT 'system',
   `modified_on` varchar(35) NOT NULL DEFAULT '',
   `public` smallint NOT NULL DEFAULT '0',
   PRIMARY KEY (`type`),
   UNIQUE INDEX `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

