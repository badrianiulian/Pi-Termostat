<?php
if (file_exists("config.php")) {
  include_once("config.php");
  include_once("functions.php");
  sec_session_start();
  if (login_check() == true) {
?>
<img src="img/empty.png" onload="window.location.href='main.php';this.parentNode.removeChild(this);" />
<?php
  } else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
  }
} else {
?>
<img src="img/empty.png" onload="window.location.href='createdb.php';this.parentNode.removeChild(this);" />
<?php
}
?>
