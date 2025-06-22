<?php
if (file_exists("config.php")) { include_once("config.php"); }
else {
?>
<img src="img/empty.png" onload="window.location.href='createdb.php';this.parentNode.removeChild(this);" />
<?php
}
include_once("functions.php");
sec_session_start();
if (login_check() == true) {
  $sq = "'";
  if (isset($_REQUEST['q'])) {
    $direction = $_REQUEST['q'];
    if ($direction=="0")
      call_sql("alter_tolerance","0");
    if ($direction=="1")
      call_sql("alter_tolerance","1");
  }
  $tolerance = select_sql("get_var","'tolerance'");
  echo $tolerance." &#8451;";
?>
<img src="img/empty.png" onload="if (!(typeof ReplaceAjax==='function')) window.location.href='main.php';this.parentNode.removeChild(this);" />
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
