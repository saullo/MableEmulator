use `auth`;

DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `username` VARCHAR(32) NOT NULL DEFAULT '',
    `email` VARCHAR(255) NOT NULL DEFAULT '',
    `salt` BINARY(32) NOT NULL,
    `verifier` BINARY(32) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `index_username` (`username`),
    UNIQUE KEY `index_email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `build_information`;
CREATE TABLE `build_information` (
    `build` INT NOT NULL,
    `major` INT DEFAULT NULL,
    `minor` INT DEFAULT NULL,
    `revision` INT DEFAULT NULL,
    PRIMARY KEY (`build`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `realmlist`;
CREATE TABLE `realmlist` (
    `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(32) NOT NULL DEFAULT '',
    `address` VARCHAR(255) NOT NULL DEFAULT '127.0.0.1',
    `local_address` VARCHAR(255) NOT NULL DEFAULT '127.0.0.1',
    `local_subnet_mask` VARCHAR(255) NOT NULL DEFAULT '255.255.255.0',
    `port` SMALLINT UNSIGNED NOT NULL DEFAULT '8085',
    `type` TINYINT UNSIGNED NOT NULL DEFAULT '0',
    `flags` TINYINT UNSIGNED NOT NULL DEFAULT '2',
    `category` TINYINT UNSIGNED NOT NULL DEFAULT '0',
    `population` FLOAT UNSIGNED NOT NULL DEFAULT '0',
    PRIMARY KEY (`id`),
    UNIQUE KEY `index_name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
