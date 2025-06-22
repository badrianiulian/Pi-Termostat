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
    $program_id = $_REQUEST['q'];
    echo '<span><div class="tab" align="center"><table><tbody><tr>';
    echo '<td><button style="width: 13.2vw;" class="tablink active" onclick="openTab(event, '.$sq.'wday1'.$sq.')">Mon</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday2'.$sq.')">Tue</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday3'.$sq.')">Wed</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday4'.$sq.')">Thu</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday5'.$sq.')">Fri</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday6'.$sq.')">Sat</button></td>';
    echo '<td><button style="width: 13.2vw;" class="tablink" onclick="openTab(event, '.$sq.'wday7'.$sq.')">Sun</button></td>';
    echo '</tr></tbody></table></div></span>';
    $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
    if ($conn->connect_error)
      die("Connection failed: ".$conn->connect_error);
    $sql="SELECT `id`,`wday`,`hour`,`temp` FROM `th_temps` WHERE `group`='$program_id';";
    if ($stmt=$conn->prepare($sql)) {
      $stmt->execute();
      $stmt->store_result();
      $stmt->bind_result($id,$wday,$hour,$temp);
      $var_wday = '1';
      echo '<div id="wday'.$var_wday.'" class="tabcontent" style="display: block;" align="center"><table><tbody>';
      while ($stmt->fetch()) {
        if ($wday != $var_wday) {
          $var_wday = $wday;
          echo '</tbody></table></div>';
          echo '<div id="wday'.$var_wday.'" class="tabcontent" style="display: none;" align="center"><table><tbody>';
        }
        echo '<tr><td><span><table><tbody><tr>';
        echo '<td><span>'.(($hour<10)?'0':'').$hour.':00</span></td>';
        echo '<td><button style="width: 65px;" onclick="ReplaceAjax('.$sq.$id.$sq.', '.$sq.'programs_modif'.$sq.', '.$sq.'0&w='.$id.$sq.');">&#x25C0;</button>';
        echo '<td><span id="'.$id.'">'.$temp.'</span></td>';
        echo '<td><button style="width: 65px;" onclick="ReplaceAjax('.$sq.$id.$sq.', '.$sq.'programs_modif'.$sq.', '.$sq.'1&w='.$id.$sq.');">&#x25B6;</button></td>';
        echo '</tr></tbody></table></span></td></tr>';
      }
      echo '</tbody></table></div>';
      $stmt->close();
    }
    mysqli_close($conn);
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
