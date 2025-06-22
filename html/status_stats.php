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
  $mode = select_sql("get_var","'mode'");
  $relay_status = select_sql("get_var","'relay_status'");
  $real_temp = select_sql("get_var","'real_temp'");
  $time = date('l d F Y H:i:s');
  $mode_text = "";
  if ($mode == 1) $mode_text = "Program: ".select_sql("get_active_program")."<br/>".select_sql("get_program_temp")." &#8451;";
  else if ($mode == 2) $mode_text = "Manual mode:<br/>".select_sql("get_var","'manual_temp'")." &#8451;";
  else $mode_text = "Off mode<br/>Tolerance can be modified";
?>
<table><tbody>
<tr style="height: 10vh;">
  <td style="width: 50vw;"><?php echo $time; ?></td>
  <td style="width: 50vw;">Relay is <?php echo $relay_status; ?></td>
</tr>
<tr style="height: 10vh;">
  <td style="width: 50vw;"><?php echo $mode_text; ?></td>
  <td style="width: 50vw;">Room temperature:<br/><?php echo $real_temp." &#8451;"; ?> degrees<img src="img/empty.png" onload="if (!(typeof ReplaceAjax==='function')) window.location.href='main.php';setTimeout(function(){ReplaceAjax('STATUS', 'status_stats');},1999);this.parentNode.removeChild(this);" /></td>
</tr>
</tbody></table>
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
