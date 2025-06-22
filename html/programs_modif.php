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
    $page = $_REQUEST['q'];
    switch ($page) {
    case "0": // program single temperature decrease
      if (isset($_REQUEST['w'])) {
        $data = $_REQUEST['w'];
        $result = select_sql("set_temp_down_prog_id",$data);
        echo $result;
      }
      break;
    case "1": // program single temperature increase
      if (isset($_REQUEST['w'])) {
        $data = $_REQUEST['w'];
        $result = select_sql("set_temp_up_prog_id",$data);
        echo $result;
      }
      break;
    case "2": // program rename
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $name = (isset($_REQUEST['e'])?$_REQUEST['e']:"undefined");
        $program = select_sql('get_program_name',$id);
        $args = '"'.$name.'",'.$id;
        $res = select_sql("program_rename",$args);
        log_sql($_SESSION['username'],'Event: Program id:'.$id.' name changed from "'.$program.'" to "'.$name.'".');
        echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.', '.$sq.$id.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    case "3": // program clone
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $program = select_sql('get_program_name',$id);
        $new_name = (isset($_REQUEST['e'])?$_REQUEST['e']:$program);
        if ($new_name == $program) $new_name = $program.'_';
        $args = '"'.$new_name.'",'.$id;
        $new_id = select_sql("program_clone",$args);
        log_sql($_SESSION['username'],'Event: Program id:'.$id.' name:"'.$program.'" cloned to id:'.$new_id.' name:"'.$new_name.'".');
        echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.', '.$sq.$new_id.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    case "4": // program delete
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $prev_id = $id;
        $program_active_id = select_sql('get_active_program_id');
        if (($id != 1) && ($id != $program_active_id)) {
          $program = select_sql('get_program_name',$id);
          $prev_id = select_sql("program_delete",$id);
          call_sql("program_delete_fix"); // auto_increment fix
          log_sql($_SESSION['username'],'Event: Program id:'.$id.' name:"'.$program.'" deleted succesfully.');
        }
        echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.', '.$sq.$prev_id.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    case "5": // increase or decrease program temperatures
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $direction = (isset($_REQUEST['e'])?$_REQUEST['e']:"0");
        $args = $direction.','.$id;
        $res = select_sql("alter_program_temp",$args);
        echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'PROGRAMS_DATA'.$sq.', '.$sq.'programs_data'.$sq.', '.$id.');this.parentNode.removeChild(this);" />';
      }
      break;
    default: // default no action
      break;
    }
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
