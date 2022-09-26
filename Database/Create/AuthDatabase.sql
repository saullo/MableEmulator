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
