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
  $data = (isset($_REQUEST['q'])?$_REQUEST['q']:'');
  switch ($data) {
  case "0":
    call_sql("rotate_mode","1");
    break;
  case "1":
    call_sql("rotate_program","0");
    break;
  case "2":
    call_sql("rotate_program","1");
    break;
  case "3":
    call_sql("alter_manual_temp","1");
    break;
  case "4":
    call_sql("alter_manual_temp","0");
    break;
  default:
    break;
  }
  $mode = select_sql("get_var","'mode'");
  $tolerance = select_sql("get_var","'tolerance'");
?>
<button class="layout" style="height: 14vh;" onclick="ReplaceAjax('CONTROL', 'status_control', '0');">Change Mode</button>
<?php
  if ($mode == 1)
  {
?>
<table>
  <tbody>
    <tr>
      <td><button class="layout" style="height: 28vh; width: calc(50vw - 14px);" onclick="ReplaceAjax('CONTROL', 'status_control', '1');">Program<br/>Backward</button></td>
      <td><button class="layout" style="height: 28vh; width: calc(50vw - 14px);" onclick="ReplaceAjax('CONTROL', 'status_control', '2');">Program<br/>Forward</button></td>
    </tr>
  </tbody>
</table>
<?php
  }
  else if ($mode == 2)
  {
?>
<table>
  <tbody>
    <tr>
      <td><button class="layout" style="height: 28vh; width: calc(50vw - 14px);" onclick="ReplaceAjax('CONTROL', 'status_control', '4');">Temperature<br/>Down</button></td>
      <td><button class="layout" style="height: 28vh; width: calc(50vw - 14px);" onclick="ReplaceAjax('CONTROL', 'status_control', '3');">Temperature<br/>Up</button></td>
    </tr>
  </tbody>
</table>
<?php
  }
  else
  {
?>
<table>
  <tbody>
    <tr>
      <td><button class="layout" style="height: 28vh; width: calc(100vw - 22px); padding: 0px;" onclick="ToggleId('TOLERANCE_DIV','block');" title="Tolerance can be modified in this mode only">OFF MODE<br/>Modify tolerance (click)</button></td>
    </tr>
  </tbody>
</table>
<div id="TOLERANCE_DIV" style="display: none;">
  <span>
    <table>
      <tbody>
        <tr>
          <td><button style="width: 7vw;" onclick="ReplaceAjax('TOLERANCE', 'status_tolerance', '0');">&#x25C0;</button></td>
          <td><span style="width:81vw;" id="TOLERANCE"><?php echo $tolerance; ?> &#8451;</span></td>
          <td><button style="width: 7vw;" onclick="ReplaceAjax('TOLERANCE', 'status_tolerance', '1');">&#x25B6;</button></td>
        </tr>
      </tbody>
    </table>
  </span>
</div>
<?php
  }
?>
<img src="img/empty.png" onload="if (!(typeof ReplaceAjax==='function')) window.location.href='main.php';this.parentNode.removeChild(this);" />
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
