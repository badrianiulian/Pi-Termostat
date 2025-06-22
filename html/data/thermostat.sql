SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

CREATE DATABASE IF NOT EXISTS `thermostat` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE `thermostat`;

DELIMITER $$
DROP PROCEDURE IF EXISTS `alter_manual_temp`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `alter_manual_temp` (IN `v_direction` INT(1))  MODIFIES SQL DATA
BEGIN
DECLARE v_new_temp DECIMAL(6,3);
IF (v_direction = 0) THEN
  SET v_new_temp = (SELECT (`val` - (25/100)) FROM `th_vars` WHERE `var`='manual_temp');
ELSE
  SET v_new_temp = (SELECT (`val` + (25/100)) FROM `th_vars` WHERE `var`='manual_temp');
END IF;
UPDATE `th_vars` SET `val` = v_new_temp WHERE `var`='manual_temp';
END$$

DROP PROCEDURE IF EXISTS `alter_tolerance`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `alter_tolerance` (IN `v_direction` INT(1))  MODIFIES SQL DATA
BEGIN
DECLARE v_new_tolerance DECIMAL(2,1);
IF (v_direction = 0) THEN
  SET v_new_tolerance = (SELECT (`val` - (10/100)) FROM `th_vars` WHERE `var`='tolerance');
ELSE
  SET v_new_tolerance = (SELECT (`val` + (10/100)) FROM `th_vars` WHERE `var`='tolerance');
END IF;
IF (v_new_tolerance <= 0) THEN SET v_new_tolerance = 0.1; END IF;
IF (v_new_tolerance >= 5) THEN SET v_new_tolerance = 4.9; END IF;
UPDATE `th_vars` SET `val` = v_new_tolerance WHERE `var`='tolerance';
END$$

DROP PROCEDURE IF EXISTS `auto_increment_fix`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `auto_increment_fix` (IN `v_table` VARCHAR(64), IN `v_inc` INT(3))  MODIFIES SQL DATA
BEGIN
    DECLARE v_stmt VARCHAR(1024);
    SET @SQL := CONCAT('ALTER TABLE ', v_table, ' AUTO_INCREMENT =  ', v_inc);
    PREPARE v_stmt FROM @SQL;
    EXECUTE v_stmt;
    DEALLOCATE PREPARE v_stmt;
END$$

DROP PROCEDURE IF EXISTS `program_delete_fix`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `program_delete_fix` ()  MODIFIES SQL DATA
BEGIN
  SET @m = (SELECT MAX(`id`) + 1 FROM `th_groups`);
  SET @n = (SELECT MAX(`id`) + 1 FROM `th_temps`);
  CALL `auto_increment_fix`('th_groups',@m);
  CALL `auto_increment_fix`('th_temps',@n);
END$$

DROP PROCEDURE IF EXISTS `rotate_mode`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `rotate_mode` (IN `v_direction` INT(1))  MODIFIES SQL DATA
BEGIN
  DECLARE v_new_mode INT(1);
  IF (v_direction = 0) THEN
  BEGIN
    SET v_new_mode = (SELECT (-1) + `val` FROM `th_vars` WHERE `var`='mode');
    IF (v_new_mode < 0) THEN SET v_new_mode = 2;
    END IF;
  END;
  ELSE
  BEGIN
    SET v_new_mode = (SELECT 1 + `val` FROM `th_vars` WHERE `var`='mode');
    IF (v_new_mode > 2) THEN SET v_new_mode = 0;
    END IF;
  END;
  END IF;
  UPDATE `th_vars` SET `val` = v_new_mode WHERE `var`='mode';
END$$

DROP PROCEDURE IF EXISTS `rotate_program`$$
CREATE DEFINER=`thermo`@`localhost` PROCEDURE `rotate_program` (IN `v_direction` INT(1))  MODIFIES SQL DATA
BEGIN
  DECLARE v_current_id INT(3);
  DECLARE v_new_id INT(3);
  IF (v_direction = 0) THEN
  BEGIN
    SET v_current_id = (SELECT `id` FROM `th_groups` WHERE `active`=1 ORDER BY `id` DESC LIMIT 1);
    SET v_new_id = (SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` < v_current_id ORDER BY `id` DESC LIMIT 1),(SELECT `id` FROM `th_groups` ORDER BY `id` DESC LIMIT 1)));
  END;
  ELSE
  BEGIN
    SET v_current_id = (SELECT `id` FROM `th_groups` WHERE `active`=1 ORDER BY `id` ASC LIMIT 1);
    SET v_new_id = (SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` > v_current_id ORDER BY `id` ASC LIMIT 1),(SELECT `id` FROM `th_groups` ORDER BY `id` ASC LIMIT 1)));
  END;
  END IF;
  UPDATE `th_groups` SET `active` = CASE WHEN `id` = v_new_id THEN 1 WHEN `id` <> v_new_id THEN 0 END;
END$$

DROP FUNCTION IF EXISTS `alter_program_temp`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `alter_program_temp` (`v_direction` INT(1), `v_id_group` INT(3)) RETURNS TINYINT(1) MODIFIES SQL DATA
BEGIN
  DECLARE v_res BOOLEAN;
  DECLARE v_id INT(11);
  DECLARE done INT(1);
  DECLARE v_new_temp DECIMAL(6,3);
  DECLARE v_plus_minus DECIMAL(6,3);
  DECLARE cursor_v_ids CURSOR FOR SELECT `id` FROM `th_temps` WHERE `group`=v_id_group;
  DECLARE CONTINUE HANDLER FOR NOT FOUND SET done=1;
  IF (v_direction=1) THEN SET v_plus_minus=(-0.250);
  ELSE SET v_plus_minus=0.250;
  END IF;
  SET done=0;
  SET v_res=FALSE;
  OPEN cursor_v_ids;
  loop_v_ids: LOOP
    FETCH cursor_v_ids INTO v_id;
    IF (done=1) THEN LEAVE loop_v_ids;
    END IF;
    IF (v_res=FALSE) THEN SET v_res=TRUE;
    END IF;
    SET v_new_temp = (SELECT (v_plus_minus+`temp`) FROM `th_temps` WHERE `id`=v_id);
    UPDATE `th_temps` SET `temp`=v_new_temp WHERE `id`=v_id;
  END LOOP loop_v_ids;
  CLOSE cursor_v_ids;
  RETURN v_res;
END$$

DROP FUNCTION IF EXISTS `get_active_program`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_active_program` () RETURNS TEXT CHARSET utf8mb4 READS SQL DATA
BEGIN
  DECLARE v_data TEXT DEFAULT "";
  SELECT `group` FROM `th_groups` WHERE `active`=1 LIMIT 1 INTO v_data;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `get_active_program_id`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_active_program_id` () RETURNS INT(3) READS SQL DATA
BEGIN
  DECLARE v_data INT(3);
  SELECT `id` FROM `th_groups` WHERE `active`=1 LIMIT 1 INTO v_data;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `get_program_id_next`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_program_id_next` (`v_id` INT(3)) RETURNS INT(3) READS SQL DATA
BEGIN
  DECLARE v_id_next INT(3);
  SET v_id_next = (SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` > v_id ORDER BY `id` ASC LIMIT 1),(SELECT `id` FROM `th_groups` ORDER BY `id` ASC LIMIT 1)));
  RETURN v_id_next;
END$$

DROP FUNCTION IF EXISTS `get_program_id_prev`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_program_id_prev` (`v_id` INT(3)) RETURNS INT(3) READS SQL DATA
BEGIN
  DECLARE v_id_prev INT(3);
  SET v_id_prev = (SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` < v_id ORDER BY `id` DESC LIMIT 1), (SELECT `id` FROM `th_groups` ORDER BY `id` DESC LIMIT 1)));
  RETURN v_id_prev;
END$$

DROP FUNCTION IF EXISTS `get_program_name`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_program_name` (`v_id` INT(3)) RETURNS TEXT CHARSET utf8mb4 READS SQL DATA
BEGIN
  DECLARE v_data TEXT DEFAULT "";
  SELECT `group` FROM `th_groups` WHERE `id`=v_id LIMIT 1 INTO v_data;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `get_program_temp`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_program_temp` () RETURNS TEXT CHARSET utf8mb4 READS SQL DATA
BEGIN
  DECLARE v_data TEXT DEFAULT "";
  DECLARE v_wday INT(1);
  DECLARE v_hour INT(2);
  SET v_wday=(SELECT (WEEKDAY(NOW())+1));
  SET v_hour=(SELECT HOUR(NOW()));
  SELECT `temp` FROM `th_temps` `t` JOIN `th_groups` `g` ON `t`.`group`=`g`.`id` WHERE `g`.`active`=1 AND `t`.`wday`=v_wday AND `t`.`hour`=v_hour LIMIT 1 INTO v_data; 
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `get_var`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `get_var` (`v_var` TEXT) RETURNS TEXT CHARSET utf8mb4 READS SQL DATA
BEGIN
  DECLARE v_data TEXT DEFAULT "";
  SELECT `val` FROM `th_vars` WHERE `var`=v_var LIMIT 1 INTO v_data;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `program_clone`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `program_clone` (`v_new_name` VARCHAR(20), `v_old_id_group` INT(3)) RETURNS INT(3) MODIFIES SQL DATA
BEGIN
  DECLARE v_new_id_group INT(3);
  INSERT INTO `th_groups`(`group`,`active`) VALUES (v_new_name,'0');
  SET v_new_id_group = (SELECT `id` FROM `th_groups` WHERE `group`=v_new_name);
  INSERT INTO `th_temps`(`group`,`wday`,`hour`,`temp`) SELECT v_new_id_group,`wday`,`hour`,`temp` FROM `th_temps` WHERE `group`=v_old_id_group;
  RETURN v_new_id_group;
END$$

DROP FUNCTION IF EXISTS `program_delete`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `program_delete` (`v_id_group` INT(3)) RETURNS INT(3) MODIFIES SQL DATA
BEGIN
  DECLARE v_data INT(3);
  SET v_data = (SELECT get_program_id_prev(v_id_group));
  DELETE FROM `th_groups` WHERE `id`=v_id_group;
  DELETE FROM `th_temps` WHERE `group`=v_id_group;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `program_rename`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `program_rename` (`v_new_name` VARCHAR(20), `v_id_group` INT(3)) RETURNS TINYINT(1) MODIFIES SQL DATA
BEGIN
  DECLARE v_er BOOLEAN;
  DECLARE v_fin BOOLEAN;
  SET v_fin = FALSE;
  SELECT EXISTS(SELECT `id` FROM `th_groups` WHERE `group`=v_new_name LIMIT 1) INTO v_er;
  IF (v_er=FALSE) THEN
  BEGIN
    SET v_fin = TRUE;
    UPDATE `th_groups` SET `group`=v_new_name WHERE `id`=v_id_group;
  END;
  END IF;
  RETURN v_fin;
END$$

DROP FUNCTION IF EXISTS `set_temp_down_prog_id`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `set_temp_down_prog_id` (`v_id` INT(11)) RETURNS DECIMAL(6,3) MODIFIES SQL DATA
BEGIN
  DECLARE v_data DECIMAL(6,3);
  SELECT (`temp` - (25/100)) FROM `th_temps` WHERE `id`=v_id INTO v_data;
  UPDATE `th_temps` SET `temp` = v_data WHERE `id`=v_id;
  RETURN v_data;
END$$

DROP FUNCTION IF EXISTS `set_temp_up_prog_id`$$
CREATE DEFINER=`thermo`@`localhost` FUNCTION `set_temp_up_prog_id` (`v_id` INT(11)) RETURNS DECIMAL(6,3) MODIFIES SQL DATA
BEGIN
  DECLARE v_data DECIMAL(6,3);
  SELECT (`temp` + (25/100)) FROM `th_temps` WHERE `id`=v_id INTO v_data;
  UPDATE `th_temps` SET `temp` = v_data WHERE `id`=v_id;
  RETURN v_data;
END$$

DELIMITER ;

DROP TABLE IF EXISTS `th_act_log`;
CREATE TABLE `th_act_log` (
  `id` int(11) NOT NULL,
  `date_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `user` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `log_data` varchar(250) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

DROP TABLE IF EXISTS `th_groups`;
CREATE TABLE `th_groups` (
  `id` int(3) NOT NULL,
  `group` varchar(20) COLLATE utf8_unicode_ci NOT NULL,
  `active` tinyint(1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `th_groups` (`id`, `group`, `active`) VALUES
(1, 'default 1', 0),
(2, 'default 2', 0),
(3, 'default 3', 0),
(4, 'default 4', 1);

DROP TABLE IF EXISTS `th_log`;
CREATE TABLE `th_log` (
  `id` int(11) NOT NULL,
  `date_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `real_temp` decimal(6,3) NOT NULL,
  `mode` int(1) NOT NULL,
  `prog` int(3) NOT NULL,
  `th_temp` decimal(6,3) NOT NULL,
  `relay` int(1) NOT NULL,
  `tolerance` decimal(2,1) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

DROP TABLE IF EXISTS `th_sec`;
CREATE TABLE `th_sec` (
  `client_side` varchar(32) NOT NULL,
  `server_side` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

DROP TABLE IF EXISTS `th_temps`;
CREATE TABLE `th_temps` (
  `id` int(11) NOT NULL,
  `group` int(3) NOT NULL,
  `wday` int(1) NOT NULL,
  `hour` int(2) NOT NULL,
  `temp` decimal(6,3) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `th_temps` (`id`, `group`, `wday`, `hour`, `temp`) VALUES
(1, 1, 1, 0, '21.000'),
(2, 1, 1, 1, '21.000'),
(3, 1, 1, 2, '21.000'),
(4, 1, 1, 3, '21.000'),
(5, 1, 1, 4, '21.000'),
(6, 1, 1, 5, '21.000'),
(7, 1, 1, 6, '21.000'),
(8, 1, 1, 7, '21.000'),
(9, 1, 1, 8, '20.000'),
(10, 1, 1, 9, '20.000'),
(11, 1, 1, 10, '20.000'),
(12, 1, 1, 11, '20.000'),
(13, 1, 1, 12, '20.000'),
(14, 1, 1, 13, '20.000'),
(15, 1, 1, 14, '20.000'),
(16, 1, 1, 15, '20.000'),
(17, 1, 1, 16, '21.000'),
(18, 1, 1, 17, '20.000'),
(19, 1, 1, 18, '20.000'),
(20, 1, 1, 19, '20.000'),
(21, 1, 1, 20, '21.000'),
(22, 1, 1, 21, '21.000'),
(23, 1, 1, 22, '21.000'),
(24, 1, 1, 23, '21.000'),
(25, 1, 2, 0, '21.000'),
(26, 1, 2, 1, '21.000'),
(27, 1, 2, 2, '21.000'),
(28, 1, 2, 3, '21.000'),
(29, 1, 2, 4, '21.000'),
(30, 1, 2, 5, '21.000'),
(31, 1, 2, 6, '21.000'),
(32, 1, 2, 7, '21.000'),
(33, 1, 2, 8, '20.000'),
(34, 1, 2, 9, '20.000'),
(35, 1, 2, 10, '20.000'),
(36, 1, 2, 11, '20.000'),
(37, 1, 2, 12, '20.000'),
(38, 1, 2, 13, '20.000'),
(39, 1, 2, 14, '20.000'),
(40, 1, 2, 15, '20.000'),
(41, 1, 2, 16, '21.000'),
(42, 1, 2, 17, '20.000'),
(43, 1, 2, 18, '20.000'),
(44, 1, 2, 19, '20.000'),
(45, 1, 2, 20, '21.000'),
(46, 1, 2, 21, '21.000'),
(47, 1, 2, 22, '21.000'),
(48, 1, 2, 23, '21.000'),
(49, 1, 3, 0, '21.000'),
(50, 1, 3, 1, '21.000'),
(51, 1, 3, 2, '21.000'),
(52, 1, 3, 3, '21.000'),
(53, 1, 3, 4, '21.000'),
(54, 1, 3, 5, '21.000'),
(55, 1, 3, 6, '21.000'),
(56, 1, 3, 7, '21.000'),
(57, 1, 3, 8, '20.000'),
(58, 1, 3, 9, '20.000'),
(59, 1, 3, 10, '20.000'),
(60, 1, 3, 11, '20.000'),
(61, 1, 3, 12, '20.000'),
(62, 1, 3, 13, '20.000'),
(63, 1, 3, 14, '20.000'),
(64, 1, 3, 15, '20.000'),
(65, 1, 3, 16, '21.000'),
(66, 1, 3, 17, '20.000'),
(67, 1, 3, 18, '20.000'),
(68, 1, 3, 19, '20.000'),
(69, 1, 3, 20, '21.000'),
(70, 1, 3, 21, '21.000'),
(71, 1, 3, 22, '21.000'),
(72, 1, 3, 23, '21.000'),
(73, 1, 4, 0, '21.000'),
(74, 1, 4, 1, '21.000'),
(75, 1, 4, 2, '21.000'),
(76, 1, 4, 3, '21.000'),
(77, 1, 4, 4, '21.000'),
(78, 1, 4, 5, '21.000'),
(79, 1, 4, 6, '21.000'),
(80, 1, 4, 7, '21.000'),
(81, 1, 4, 8, '20.000'),
(82, 1, 4, 9, '20.000'),
(83, 1, 4, 10, '20.000'),
(84, 1, 4, 11, '20.000'),
(85, 1, 4, 12, '20.000'),
(86, 1, 4, 13, '20.000'),
(87, 1, 4, 14, '20.000'),
(88, 1, 4, 15, '20.000'),
(89, 1, 4, 16, '21.000'),
(90, 1, 4, 17, '20.000'),
(91, 1, 4, 18, '20.000'),
(92, 1, 4, 19, '20.000'),
(93, 1, 4, 20, '21.000'),
(94, 1, 4, 21, '21.000'),
(95, 1, 4, 22, '21.000'),
(96, 1, 4, 23, '21.000'),
(97, 1, 5, 0, '21.000'),
(98, 1, 5, 1, '21.000'),
(99, 1, 5, 2, '21.000'),
(100, 1, 5, 3, '21.000'),
(101, 1, 5, 4, '21.000'),
(102, 1, 5, 5, '21.000'),
(103, 1, 5, 6, '21.000'),
(104, 1, 5, 7, '21.000'),
(105, 1, 5, 8, '20.000'),
(106, 1, 5, 9, '20.000'),
(107, 1, 5, 10, '20.000'),
(108, 1, 5, 11, '20.000'),
(109, 1, 5, 12, '20.000'),
(110, 1, 5, 13, '20.000'),
(111, 1, 5, 14, '21.000'),
(112, 1, 5, 15, '21.000'),
(113, 1, 5, 16, '21.000'),
(114, 1, 5, 17, '20.000'),
(115, 1, 5, 18, '20.000'),
(116, 1, 5, 19, '20.000'),
(117, 1, 5, 20, '21.000'),
(118, 1, 5, 21, '21.000'),
(119, 1, 5, 22, '21.000'),
(120, 1, 5, 23, '21.000'),
(121, 1, 6, 0, '21.000'),
(122, 1, 6, 1, '21.000'),
(123, 1, 6, 2, '21.000'),
(124, 1, 6, 3, '21.000'),
(125, 1, 6, 4, '21.000'),
(126, 1, 6, 5, '21.000'),
(127, 1, 6, 6, '21.000'),
(128, 1, 6, 7, '21.000'),
(129, 1, 6, 8, '20.000'),
(130, 1, 6, 9, '20.000'),
(131, 1, 6, 10, '20.000'),
(132, 1, 6, 11, '20.000'),
(133, 1, 6, 12, '20.000'),
(134, 1, 6, 13, '20.000'),
(135, 1, 6, 14, '21.000'),
(136, 1, 6, 15, '21.000'),
(137, 1, 6, 16, '21.000'),
(138, 1, 6, 17, '20.000'),
(139, 1, 6, 18, '20.000'),
(140, 1, 6, 19, '20.000'),
(141, 1, 6, 20, '21.000'),
(142, 1, 6, 21, '21.000'),
(143, 1, 6, 22, '21.000'),
(144, 1, 6, 23, '21.000'),
(145, 1, 7, 0, '21.000'),
(146, 1, 7, 1, '21.000'),
(147, 1, 7, 2, '21.000'),
(148, 1, 7, 3, '21.000'),
(149, 1, 7, 4, '21.000'),
(150, 1, 7, 5, '21.000'),
(151, 1, 7, 6, '21.000'),
(152, 1, 7, 7, '21.000'),
(153, 1, 7, 8, '20.000'),
(154, 1, 7, 9, '20.000'),
(155, 1, 7, 10, '20.000'),
(156, 1, 7, 11, '20.000'),
(157, 1, 7, 12, '20.000'),
(158, 1, 7, 13, '20.000'),
(159, 1, 7, 14, '21.000'),
(160, 1, 7, 15, '21.000'),
(161, 1, 7, 16, '21.000'),
(162, 1, 7, 17, '20.000'),
(163, 1, 7, 18, '20.000'),
(164, 1, 7, 19, '20.000'),
(165, 1, 7, 20, '21.000'),
(166, 1, 7, 21, '21.000'),
(167, 1, 7, 22, '21.000'),
(168, 1, 7, 23, '21.000'),
(169, 2, 1, 0, '23.000'),
(170, 2, 1, 1, '23.000'),
(171, 2, 1, 2, '23.000'),
(172, 2, 1, 3, '23.000'),
(173, 2, 1, 4, '23.000'),
(174, 2, 1, 5, '23.000'),
(175, 2, 1, 6, '23.000'),
(176, 2, 1, 7, '23.000'),
(177, 2, 1, 8, '22.000'),
(178, 2, 1, 9, '22.000'),
(179, 2, 1, 10, '22.000'),
(180, 2, 1, 11, '22.000'),
(181, 2, 1, 12, '22.000'),
(182, 2, 1, 13, '22.000'),
(183, 2, 1, 14, '22.000'),
(184, 2, 1, 15, '22.000'),
(185, 2, 1, 16, '23.000'),
(186, 2, 1, 17, '22.000'),
(187, 2, 1, 18, '22.000'),
(188, 2, 1, 19, '22.000'),
(189, 2, 1, 20, '23.000'),
(190, 2, 1, 21, '23.000'),
(191, 2, 1, 22, '23.000'),
(192, 2, 1, 23, '23.000'),
(193, 2, 2, 0, '23.000'),
(194, 2, 2, 1, '23.000'),
(195, 2, 2, 2, '23.000'),
(196, 2, 2, 3, '23.000'),
(197, 2, 2, 4, '23.000'),
(198, 2, 2, 5, '23.000'),
(199, 2, 2, 6, '23.000'),
(200, 2, 2, 7, '23.000'),
(201, 2, 2, 8, '22.000'),
(202, 2, 2, 9, '22.000'),
(203, 2, 2, 10, '22.000'),
(204, 2, 2, 11, '22.000'),
(205, 2, 2, 12, '22.000'),
(206, 2, 2, 13, '22.000'),
(207, 2, 2, 14, '22.000'),
(208, 2, 2, 15, '22.000'),
(209, 2, 2, 16, '23.000'),
(210, 2, 2, 17, '22.000'),
(211, 2, 2, 18, '22.000'),
(212, 2, 2, 19, '22.000'),
(213, 2, 2, 20, '23.000'),
(214, 2, 2, 21, '23.000'),
(215, 2, 2, 22, '23.000'),
(216, 2, 2, 23, '23.000'),
(217, 2, 3, 0, '23.000'),
(218, 2, 3, 1, '23.000'),
(219, 2, 3, 2, '23.000'),
(220, 2, 3, 3, '23.000'),
(221, 2, 3, 4, '23.000'),
(222, 2, 3, 5, '23.000'),
(223, 2, 3, 6, '23.000'),
(224, 2, 3, 7, '23.000'),
(225, 2, 3, 8, '22.000'),
(226, 2, 3, 9, '22.000'),
(227, 2, 3, 10, '22.000'),
(228, 2, 3, 11, '22.000'),
(229, 2, 3, 12, '22.000'),
(230, 2, 3, 13, '22.000'),
(231, 2, 3, 14, '22.000'),
(232, 2, 3, 15, '22.000'),
(233, 2, 3, 16, '23.000'),
(234, 2, 3, 17, '22.000'),
(235, 2, 3, 18, '22.000'),
(236, 2, 3, 19, '22.000'),
(237, 2, 3, 20, '23.000'),
(238, 2, 3, 21, '23.000'),
(239, 2, 3, 22, '23.000'),
(240, 2, 3, 23, '23.000'),
(241, 2, 4, 0, '23.000'),
(242, 2, 4, 1, '23.000'),
(243, 2, 4, 2, '23.000'),
(244, 2, 4, 3, '23.000'),
(245, 2, 4, 4, '23.000'),
(246, 2, 4, 5, '23.000'),
(247, 2, 4, 6, '23.000'),
(248, 2, 4, 7, '23.000'),
(249, 2, 4, 8, '22.000'),
(250, 2, 4, 9, '22.000'),
(251, 2, 4, 10, '22.000'),
(252, 2, 4, 11, '22.000'),
(253, 2, 4, 12, '22.000'),
(254, 2, 4, 13, '22.000'),
(255, 2, 4, 14, '22.000'),
(256, 2, 4, 15, '22.000'),
(257, 2, 4, 16, '23.000'),
(258, 2, 4, 17, '22.000'),
(259, 2, 4, 18, '22.000'),
(260, 2, 4, 19, '22.000'),
(261, 2, 4, 20, '23.000'),
(262, 2, 4, 21, '23.000'),
(263, 2, 4, 22, '23.000'),
(264, 2, 4, 23, '23.000'),
(265, 2, 5, 0, '23.000'),
(266, 2, 5, 1, '23.000'),
(267, 2, 5, 2, '23.000'),
(268, 2, 5, 3, '23.000'),
(269, 2, 5, 4, '23.000'),
(270, 2, 5, 5, '23.000'),
(271, 2, 5, 6, '23.000'),
(272, 2, 5, 7, '23.000'),
(273, 2, 5, 8, '22.000'),
(274, 2, 5, 9, '22.000'),
(275, 2, 5, 10, '22.000'),
(276, 2, 5, 11, '22.000'),
(277, 2, 5, 12, '22.000'),
(278, 2, 5, 13, '22.000'),
(279, 2, 5, 14, '23.000'),
(280, 2, 5, 15, '23.000'),
(281, 2, 5, 16, '23.000'),
(282, 2, 5, 17, '22.000'),
(283, 2, 5, 18, '22.000'),
(284, 2, 5, 19, '22.000'),
(285, 2, 5, 20, '23.000'),
(286, 2, 5, 21, '23.000'),
(287, 2, 5, 22, '23.000'),
(288, 2, 5, 23, '23.000'),
(289, 2, 6, 0, '23.000'),
(290, 2, 6, 1, '23.000'),
(291, 2, 6, 2, '23.000'),
(292, 2, 6, 3, '23.000'),
(293, 2, 6, 4, '23.000'),
(294, 2, 6, 5, '23.000'),
(295, 2, 6, 6, '23.000'),
(296, 2, 6, 7, '23.000'),
(297, 2, 6, 8, '22.000'),
(298, 2, 6, 9, '22.000'),
(299, 2, 6, 10, '22.000'),
(300, 2, 6, 11, '22.000'),
(301, 2, 6, 12, '22.000'),
(302, 2, 6, 13, '22.000'),
(303, 2, 6, 14, '23.000'),
(304, 2, 6, 15, '23.000'),
(305, 2, 6, 16, '23.000'),
(306, 2, 6, 17, '22.000'),
(307, 2, 6, 18, '22.000'),
(308, 2, 6, 19, '22.000'),
(309, 2, 6, 20, '23.000'),
(310, 2, 6, 21, '23.000'),
(311, 2, 6, 22, '23.000'),
(312, 2, 6, 23, '23.000'),
(313, 2, 7, 0, '23.000'),
(314, 2, 7, 1, '23.000'),
(315, 2, 7, 2, '23.000'),
(316, 2, 7, 3, '23.000'),
(317, 2, 7, 4, '23.000'),
(318, 2, 7, 5, '23.000'),
(319, 2, 7, 6, '23.000'),
(320, 2, 7, 7, '23.000'),
(321, 2, 7, 8, '22.000'),
(322, 2, 7, 9, '22.000'),
(323, 2, 7, 10, '22.000'),
(324, 2, 7, 11, '22.000'),
(325, 2, 7, 12, '22.000'),
(326, 2, 7, 13, '22.000'),
(327, 2, 7, 14, '23.000'),
(328, 2, 7, 15, '23.000'),
(329, 2, 7, 16, '23.000'),
(330, 2, 7, 17, '22.000'),
(331, 2, 7, 18, '22.000'),
(332, 2, 7, 19, '22.000'),
(333, 2, 7, 20, '23.000'),
(334, 2, 7, 21, '23.000'),
(335, 2, 7, 22, '23.000'),
(336, 2, 7, 23, '23.000'),
(337, 3, 1, 0, '25.000'),
(338, 3, 1, 1, '25.000'),
(339, 3, 1, 2, '25.000'),
(340, 3, 1, 3, '25.000'),
(341, 3, 1, 4, '25.000'),
(342, 3, 1, 5, '25.000'),
(343, 3, 1, 6, '25.000'),
(344, 3, 1, 7, '25.000'),
(345, 3, 1, 8, '24.000'),
(346, 3, 1, 9, '24.000'),
(347, 3, 1, 10, '24.000'),
(348, 3, 1, 11, '24.000'),
(349, 3, 1, 12, '24.000'),
(350, 3, 1, 13, '24.000'),
(351, 3, 1, 14, '24.000'),
(352, 3, 1, 15, '24.000'),
(353, 3, 1, 16, '25.000'),
(354, 3, 1, 17, '24.000'),
(355, 3, 1, 18, '24.000'),
(356, 3, 1, 19, '24.000'),
(357, 3, 1, 20, '25.000'),
(358, 3, 1, 21, '25.000'),
(359, 3, 1, 22, '25.000'),
(360, 3, 1, 23, '25.000'),
(361, 3, 2, 0, '25.000'),
(362, 3, 2, 1, '25.000'),
(363, 3, 2, 2, '25.000'),
(364, 3, 2, 3, '25.000'),
(365, 3, 2, 4, '25.000'),
(366, 3, 2, 5, '25.000'),
(367, 3, 2, 6, '25.000'),
(368, 3, 2, 7, '25.000'),
(369, 3, 2, 8, '24.000'),
(370, 3, 2, 9, '24.000'),
(371, 3, 2, 10, '24.000'),
(372, 3, 2, 11, '24.000'),
(373, 3, 2, 12, '24.000'),
(374, 3, 2, 13, '24.000'),
(375, 3, 2, 14, '24.000'),
(376, 3, 2, 15, '24.000'),
(377, 3, 2, 16, '25.000'),
(378, 3, 2, 17, '24.000'),
(379, 3, 2, 18, '24.000'),
(380, 3, 2, 19, '24.000'),
(381, 3, 2, 20, '25.000'),
(382, 3, 2, 21, '25.000'),
(383, 3, 2, 22, '25.000'),
(384, 3, 2, 23, '25.000'),
(385, 3, 3, 0, '25.000'),
(386, 3, 3, 1, '25.000'),
(387, 3, 3, 2, '25.000'),
(388, 3, 3, 3, '25.000'),
(389, 3, 3, 4, '25.000'),
(390, 3, 3, 5, '25.000'),
(391, 3, 3, 6, '25.000'),
(392, 3, 3, 7, '25.000'),
(393, 3, 3, 8, '24.000'),
(394, 3, 3, 9, '24.000'),
(395, 3, 3, 10, '24.000'),
(396, 3, 3, 11, '24.000'),
(397, 3, 3, 12, '24.000'),
(398, 3, 3, 13, '24.000'),
(399, 3, 3, 14, '24.000'),
(400, 3, 3, 15, '24.000'),
(401, 3, 3, 16, '25.000'),
(402, 3, 3, 17, '24.000'),
(403, 3, 3, 18, '24.000'),
(404, 3, 3, 19, '24.000'),
(405, 3, 3, 20, '25.000'),
(406, 3, 3, 21, '25.000'),
(407, 3, 3, 22, '25.000'),
(408, 3, 3, 23, '25.000'),
(409, 3, 4, 0, '25.000'),
(410, 3, 4, 1, '25.000'),
(411, 3, 4, 2, '25.000'),
(412, 3, 4, 3, '25.000'),
(413, 3, 4, 4, '25.000'),
(414, 3, 4, 5, '25.000'),
(415, 3, 4, 6, '25.000'),
(416, 3, 4, 7, '25.000'),
(417, 3, 4, 8, '24.000'),
(418, 3, 4, 9, '24.000'),
(419, 3, 4, 10, '24.000'),
(420, 3, 4, 11, '24.000'),
(421, 3, 4, 12, '24.000'),
(422, 3, 4, 13, '24.000'),
(423, 3, 4, 14, '24.000'),
(424, 3, 4, 15, '24.000'),
(425, 3, 4, 16, '25.000'),
(426, 3, 4, 17, '24.000'),
(427, 3, 4, 18, '24.000'),
(428, 3, 4, 19, '24.000'),
(429, 3, 4, 20, '25.000'),
(430, 3, 4, 21, '25.000'),
(431, 3, 4, 22, '25.000'),
(432, 3, 4, 23, '25.000'),
(433, 3, 5, 0, '25.000'),
(434, 3, 5, 1, '25.000'),
(435, 3, 5, 2, '25.000'),
(436, 3, 5, 3, '25.000'),
(437, 3, 5, 4, '25.000'),
(438, 3, 5, 5, '25.000'),
(439, 3, 5, 6, '25.000'),
(440, 3, 5, 7, '25.000'),
(441, 3, 5, 8, '24.000'),
(442, 3, 5, 9, '24.000'),
(443, 3, 5, 10, '24.000'),
(444, 3, 5, 11, '24.000'),
(445, 3, 5, 12, '24.000'),
(446, 3, 5, 13, '24.000'),
(447, 3, 5, 14, '25.000'),
(448, 3, 5, 15, '25.000'),
(449, 3, 5, 16, '25.000'),
(450, 3, 5, 17, '24.000'),
(451, 3, 5, 18, '24.000'),
(452, 3, 5, 19, '24.000'),
(453, 3, 5, 20, '25.000'),
(454, 3, 5, 21, '25.000'),
(455, 3, 5, 22, '25.000'),
(456, 3, 5, 23, '25.000'),
(457, 3, 6, 0, '25.000'),
(458, 3, 6, 1, '25.000'),
(459, 3, 6, 2, '25.000'),
(460, 3, 6, 3, '25.000'),
(461, 3, 6, 4, '25.000'),
(462, 3, 6, 5, '25.000'),
(463, 3, 6, 6, '25.000'),
(464, 3, 6, 7, '25.000'),
(465, 3, 6, 8, '24.000'),
(466, 3, 6, 9, '24.000'),
(467, 3, 6, 10, '24.000'),
(468, 3, 6, 11, '24.000'),
(469, 3, 6, 12, '24.000'),
(470, 3, 6, 13, '24.000'),
(471, 3, 6, 14, '25.000'),
(472, 3, 6, 15, '25.000'),
(473, 3, 6, 16, '25.000'),
(474, 3, 6, 17, '24.000'),
(475, 3, 6, 18, '24.000'),
(476, 3, 6, 19, '24.000'),
(477, 3, 6, 20, '25.000'),
(478, 3, 6, 21, '25.000'),
(479, 3, 6, 22, '25.000'),
(480, 3, 6, 23, '25.000'),
(481, 3, 7, 0, '25.000'),
(482, 3, 7, 1, '25.000'),
(483, 3, 7, 2, '25.000'),
(484, 3, 7, 3, '25.000'),
(485, 3, 7, 4, '25.000'),
(486, 3, 7, 5, '25.000'),
(487, 3, 7, 6, '25.000'),
(488, 3, 7, 7, '25.000'),
(489, 3, 7, 8, '24.000'),
(490, 3, 7, 9, '24.000'),
(491, 3, 7, 10, '24.000'),
(492, 3, 7, 11, '24.000'),
(493, 3, 7, 12, '24.000'),
(494, 3, 7, 13, '24.000'),
(495, 3, 7, 14, '25.000'),
(496, 3, 7, 15, '25.000'),
(497, 3, 7, 16, '25.000'),
(498, 3, 7, 17, '24.000'),
(499, 3, 7, 18, '24.000'),
(500, 3, 7, 19, '24.000'),
(501, 3, 7, 20, '25.000'),
(502, 3, 7, 21, '25.000'),
(503, 3, 7, 22, '25.000'),
(504, 3, 7, 23, '25.000'),
(505, 4, 1, 0, '24.000'),
(506, 4, 1, 1, '24.000'),
(507, 4, 1, 2, '24.000'),
(508, 4, 1, 3, '24.000'),
(509, 4, 1, 4, '24.000'),
(510, 4, 1, 5, '24.000'),
(511, 4, 1, 6, '24.000'),
(512, 4, 1, 7, '24.000'),
(513, 4, 1, 8, '23.000'),
(514, 4, 1, 9, '23.000'),
(515, 4, 1, 10, '23.000'),
(516, 4, 1, 11, '23.000'),
(517, 4, 1, 12, '23.000'),
(518, 4, 1, 13, '23.000'),
(519, 4, 1, 14, '23.000'),
(520, 4, 1, 15, '23.000'),
(521, 4, 1, 16, '24.000'),
(522, 4, 1, 17, '23.000'),
(523, 4, 1, 18, '23.000'),
(524, 4, 1, 19, '23.000'),
(525, 4, 1, 20, '24.000'),
(526, 4, 1, 21, '24.000'),
(527, 4, 1, 22, '24.000'),
(528, 4, 1, 23, '24.000'),
(529, 4, 2, 0, '24.000'),
(530, 4, 2, 1, '24.000'),
(531, 4, 2, 2, '24.000'),
(532, 4, 2, 3, '24.000'),
(533, 4, 2, 4, '24.000'),
(534, 4, 2, 5, '24.000'),
(535, 4, 2, 6, '24.000'),
(536, 4, 2, 7, '24.000'),
(537, 4, 2, 8, '23.000'),
(538, 4, 2, 9, '23.000'),
(539, 4, 2, 10, '23.000'),
(540, 4, 2, 11, '23.000'),
(541, 4, 2, 12, '23.000'),
(542, 4, 2, 13, '23.000'),
(543, 4, 2, 14, '23.000'),
(544, 4, 2, 15, '23.000'),
(545, 4, 2, 16, '24.000'),
(546, 4, 2, 17, '23.000'),
(547, 4, 2, 18, '23.000'),
(548, 4, 2, 19, '23.000'),
(549, 4, 2, 20, '24.000'),
(550, 4, 2, 21, '24.000'),
(551, 4, 2, 22, '24.000'),
(552, 4, 2, 23, '24.000'),
(553, 4, 3, 0, '24.000'),
(554, 4, 3, 1, '24.000'),
(555, 4, 3, 2, '24.000'),
(556, 4, 3, 3, '24.000'),
(557, 4, 3, 4, '24.000'),
(558, 4, 3, 5, '24.000'),
(559, 4, 3, 6, '24.000'),
(560, 4, 3, 7, '24.000'),
(561, 4, 3, 8, '23.000'),
(562, 4, 3, 9, '23.000'),
(563, 4, 3, 10, '23.000'),
(564, 4, 3, 11, '23.000'),
(565, 4, 3, 12, '23.000'),
(566, 4, 3, 13, '23.000'),
(567, 4, 3, 14, '23.000'),
(568, 4, 3, 15, '23.000'),
(569, 4, 3, 16, '24.000'),
(570, 4, 3, 17, '23.000'),
(571, 4, 3, 18, '23.000'),
(572, 4, 3, 19, '23.000'),
(573, 4, 3, 20, '24.000'),
(574, 4, 3, 21, '24.000'),
(575, 4, 3, 22, '24.000'),
(576, 4, 3, 23, '24.000'),
(577, 4, 4, 0, '24.000'),
(578, 4, 4, 1, '24.000'),
(579, 4, 4, 2, '24.000'),
(580, 4, 4, 3, '24.000'),
(581, 4, 4, 4, '24.000'),
(582, 4, 4, 5, '24.000'),
(583, 4, 4, 6, '24.000'),
(584, 4, 4, 7, '24.000'),
(585, 4, 4, 8, '23.000'),
(586, 4, 4, 9, '23.000'),
(587, 4, 4, 10, '23.000'),
(588, 4, 4, 11, '23.000'),
(589, 4, 4, 12, '23.000'),
(590, 4, 4, 13, '23.000'),
(591, 4, 4, 14, '23.000'),
(592, 4, 4, 15, '23.000'),
(593, 4, 4, 16, '24.000'),
(594, 4, 4, 17, '23.000'),
(595, 4, 4, 18, '23.000'),
(596, 4, 4, 19, '23.000'),
(597, 4, 4, 20, '24.000'),
(598, 4, 4, 21, '24.000'),
(599, 4, 4, 22, '24.000'),
(600, 4, 4, 23, '24.000'),
(601, 4, 5, 0, '24.000'),
(602, 4, 5, 1, '24.000'),
(603, 4, 5, 2, '24.000'),
(604, 4, 5, 3, '24.000'),
(605, 4, 5, 4, '24.000'),
(606, 4, 5, 5, '24.000'),
(607, 4, 5, 6, '24.000'),
(608, 4, 5, 7, '24.000'),
(609, 4, 5, 8, '23.000'),
(610, 4, 5, 9, '23.000'),
(611, 4, 5, 10, '23.000'),
(612, 4, 5, 11, '23.000'),
(613, 4, 5, 12, '23.000'),
(614, 4, 5, 13, '23.000'),
(615, 4, 5, 14, '24.000'),
(616, 4, 5, 15, '24.000'),
(617, 4, 5, 16, '24.000'),
(618, 4, 5, 17, '23.000'),
(619, 4, 5, 18, '23.000'),
(620, 4, 5, 19, '23.000'),
(621, 4, 5, 20, '24.000'),
(622, 4, 5, 21, '24.000'),
(623, 4, 5, 22, '24.000'),
(624, 4, 5, 23, '24.000'),
(625, 4, 6, 0, '24.000'),
(626, 4, 6, 1, '24.000'),
(627, 4, 6, 2, '24.000'),
(628, 4, 6, 3, '24.000'),
(629, 4, 6, 4, '24.000'),
(630, 4, 6, 5, '24.000'),
(631, 4, 6, 6, '24.000'),
(632, 4, 6, 7, '24.000'),
(633, 4, 6, 8, '23.000'),
(634, 4, 6, 9, '23.000'),
(635, 4, 6, 10, '23.000'),
(636, 4, 6, 11, '23.000'),
(637, 4, 6, 12, '23.000'),
(638, 4, 6, 13, '23.000'),
(639, 4, 6, 14, '24.000'),
(640, 4, 6, 15, '24.000'),
(641, 4, 6, 16, '24.000'),
(642, 4, 6, 17, '23.000'),
(643, 4, 6, 18, '23.000'),
(644, 4, 6, 19, '23.000'),
(645, 4, 6, 20, '24.000'),
(646, 4, 6, 21, '24.000'),
(647, 4, 6, 22, '24.000'),
(648, 4, 6, 23, '24.000'),
(649, 4, 7, 0, '24.000'),
(650, 4, 7, 1, '24.000'),
(651, 4, 7, 2, '24.000'),
(652, 4, 7, 3, '24.000'),
(653, 4, 7, 4, '24.000'),
(654, 4, 7, 5, '24.000'),
(655, 4, 7, 6, '24.000'),
(656, 4, 7, 7, '24.000'),
(657, 4, 7, 8, '23.000'),
(658, 4, 7, 9, '23.000'),
(659, 4, 7, 10, '23.000'),
(660, 4, 7, 11, '23.000'),
(661, 4, 7, 12, '23.000'),
(662, 4, 7, 13, '23.000'),
(663, 4, 7, 14, '24.000'),
(664, 4, 7, 15, '24.000'),
(665, 4, 7, 16, '24.000'),
(666, 4, 7, 17, '23.000'),
(667, 4, 7, 18, '23.000'),
(668, 4, 7, 19, '23.000'),
(669, 4, 7, 20, '24.000'),
(670, 4, 7, 21, '24.000'),
(671, 4, 7, 22, '24.000'),
(672, 4, 7, 23, '24.000');

DROP TABLE IF EXISTS `th_users`;
CREATE TABLE `th_users` (
  `id` int(11) NOT NULL,
  `user` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `pass` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  `attempt` int(1) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

DROP TABLE IF EXISTS `th_vars`;
CREATE TABLE `th_vars` (
  `id` int(11) NOT NULL,
  `var` varchar(50) COLLATE utf8_unicode_ci NOT NULL,
  `val` varchar(250) COLLATE utf8_unicode_ci NOT NULL,
  `type` varchar(250) COLLATE utf8_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `th_vars` (`id`, `var`, `val`, `type`) VALUES
(1, 'mode', '2', 'int(1)'),
(2, 'manual_temp', '25.000', 'decimal(6,3)'),
(3, 'real_temp', '29.375', 'decimal(6,3)'),
(4, 'relay_status', 'OFF', 'varchar(3)'),
(5, 'tolerance', '1.0', 'decimal(2,1)');


ALTER TABLE `th_act_log`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `th_groups`
  ADD PRIMARY KEY (`id`,`group`);

ALTER TABLE `th_log`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `th_sec`
  ADD PRIMARY KEY (`client_side`);

ALTER TABLE `th_temps`
  ADD PRIMARY KEY (`id`);

ALTER TABLE `th_users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `user` (`user`);

ALTER TABLE `th_vars`
  ADD PRIMARY KEY (`id`,`var`);


ALTER TABLE `th_act_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

ALTER TABLE `th_groups`
  MODIFY `id` int(3) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

ALTER TABLE `th_log`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

ALTER TABLE `th_temps`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=673;

ALTER TABLE `th_users`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;

ALTER TABLE `th_vars`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
