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
    case "MAIN":
      echo '<button class="strokeme" style="height: 30vh; background-image: url(img/status.png); background-size: 40vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'STATUS'.$sq.')">Status</button>';
      echo '<button class="strokeme" style="height: 30vh; background-image: url(img/programs.png); background-size: 40vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'PROGRAMS'.$sq.')">Programs</button>';
      echo '<button class="strokeme" style="height: 30vh; background-image: url(img/statistics.png); background-size: 40vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'STATISTICS'.$sq.')">Daily Statistics</button>';
      break;
    case "STATUS":
      echo '<span id="STATUS"><img src="img/empty.png" onload="ReplaceAjax('.$sq.'STATUS'.$sq.', '.$sq.'status_stats'.$sq.');this.parentNode.removeChild(this);" /></span>';
      echo '<div id="CONTROL"><img src="img/empty.png" onload="ReplaceAjax('.$sq.'CONTROL'.$sq.', '.$sq.'status_control'.$sq.');this.parentNode.removeChild(this);" /></div>';
      echo '<button class="layout" style="height: 13vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'MAIN'.$sq.')">Back to main screen</button>';
      break;
    case "PROGRAMS":
      echo '<div id="PROGRAMS"><img src="img/empty.png" onload="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.');this.parentNode.removeChild(this);" /></div>';
      echo '<button class="layout" style="height: 13vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'MAIN'.$sq.')">Back to main screen</button>';
      break;
    case "STATISTICS":
      echo '<div id="STATISTICS"><img src="img/empty.png" onload="if(window.HTMLCanvasElement){ReplaceAjax('.$sq.'STATISTICS'.$sq.', '.$sq.'statistics'.$sq.');}else{alert('.$sq.'Browser not supported!'.$sq.');} this.parentNode.removeChild(this);" /></div>';
      echo '<button class="layout" style="height: 13vh;" onclick="removeHeadId('.$sq.'CanvasScriptID'.$sq.');ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'MAIN'.$sq.')">Back to main screen</button>';
      break;
    case "USER":
      echo '<div id="USER"><img src="img/empty.png" onload="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'0'.$sq.');this.parentNode.removeChild(this);" /></div>';
      echo '<div id="LOGS"><img src="img/empty.png" onload="ReplaceAjax('.$sq.'LOGS'.$sq.', '.$sq.'logs'.$sq.');this.parentNode.removeChild(this);" /></div>';
      echo '<button class="layout" style="height: 13vh;" onclick="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'MAIN'.$sq.')">Back to main screen</button>';
      break;
    default:
      break;
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
