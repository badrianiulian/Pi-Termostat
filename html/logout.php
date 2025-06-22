<?php
include_once("functions.php");
sec_session_start();
if (isset($_SESSION['username'])) log_sql($_SESSION['username'],'Success: User disconnected.');
$_SESSION = array();
if (ini_get("session.use_cookies")) {
  $params = session_get_cookie_params();
  setcookie(session_name(), '', time() - 42000,$params["path"], $params["domain"],$params["secure"], $params["httponly"]);
}
session_destroy();
?>
<img src="img/empty.png" onload="window.location.href='index.php';this.parentNode.removeChild(this);" />
