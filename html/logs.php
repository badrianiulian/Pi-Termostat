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
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $sql="SELECT COUNT(*) AS `count` FROM `th_act_log`;";
  $count = PHP_INT_MAX;
  if ($stmt=$conn->prepare($sql)) {
    $stmt->execute();
    $stmt->store_result();
    $stmt->bind_result($count);
    $stmt->fetch();
    $stmt->close();
  }
  $_azza = (isset($_REQUEST['q'])?$_REQUEST['q']:''); // Order ascending (1) descending (0)
  $_rows = (isset($_REQUEST['w'])?$_REQUEST['w']:''); // Number of rows
  $_no = (isset($_REQUEST['e'])?$_REQUEST['e']:'');   // Starting number
  $order = (($_azza=='0')||($_azza == '1')?intval($_azza):0);
  $row_no = ((($_rows=='25')||($_rows=='50')||($_rows=='100')||($_rows=='250')||($_rows=='500'))?intval($_rows):25);
  $start_at = ((ctype_digit(ltrim((string)$_no, '-')))?intval($_no):0);
  $sql=(($order==0)?'SELECT `date_time`,`user`,`log_data` FROM `th_act_log` ORDER BY `id` DESC LIMIT ?,?;':'SELECT `date_time`,`user`,`log_data` FROM `th_act_log` ORDER BY `id` ASC LIMIT ?,?;');
  if ($stmt=$conn->prepare($sql)) {
    $stmt->bind_param('ii',$start_at,$row_no);
    $stmt->execute();
    $stmt->store_result();
    $stmt->bind_result($date_time,$user,$log_data);
    echo '<div class="layout"><button onclick="ToggleId('.$sq.'history'.$sq.','.$sq.'flex'.$sq.');">History</button>';
    echo '<div id="history">';
    echo '<table><tbody><tr>';
    echo '<td><span>&nbsp;Order:&nbsp;';
    echo '<select id="selector_order" name="selector_order" onchange="var e = document.getElementById('.$sq.'selector_order'.$sq.');var order = e.options[e.selectedIndex].value;ReplaceAjax('.$sq.'LOGS'.$sq.', '.$sq.'logs'.$sq.', order);">';
    echo '<option value="0"'.(($order==0)?' selected="selected" style="font-weight: bold;"':'').'>Newer First</option>';
    echo '<option value="1"'.(($order==1)?' selected="selected" style="font-weight: bold;"':'').'>Older First</option>';
    echo '</select>&nbsp;</span></td>';
    echo '<td><span>&nbsp;Rows:&nbsp;';
    echo '<select id="selector_rows" name="selector_rows" onchange="var e = document.getElementById('.$sq.'selector_order'.$sq.');var order = e.options[e.selectedIndex].value;var e = document.getElementById('.$sq.'selector_rows'.$sq.');var row_no = e.options[e.selectedIndex].value;ReplaceAjax('.$sq.'LOGS'.$sq.', '.$sq.'logs'.$sq.', order+'.$sq.'&w='.$sq.'+row_no);">';
    echo '<option value="25"'.(($row_no==25)?' selected="selected" style="font-weight: bold;"':'').'>25</option>';
    echo '<option value="50"'.(($row_no==50)?' selected="selected" style="font-weight: bold;"':'').'>50</option>';
    echo '<option value="100"'.(($row_no==100)?' selected="selected" style="font-weight: bold;"':'').'>100</option>';
    echo '<option value="250"'.(($row_no==250)?' selected="selected" style="font-weight: bold;"':'').'>250</option>';
    echo '<option value="500"'.(($row_no==500)?' selected="selected" style="font-weight: bold;"':'').'>500</option>';
    echo '</select>&nbsp;</span></td>';
    echo '<td><span>&nbsp;Page:&nbsp;';
    echo '<select id="selector_page" name="selector_page" onchange="var e = document.getElementById('.$sq.'selector_order'.$sq.');var order = e.options[e.selectedIndex].value;var e = document.getElementById('.$sq.'selector_rows'.$sq.');var row_no = e.options[e.selectedIndex].value;var e = document.getElementById('.$sq.'selector_page'.$sq.');var start_at = e.options[e.selectedIndex].value;ReplaceAjax('.$sq.'LOGS'.$sq.', '.$sq.'logs'.$sq.', order+'.$sq.'&w='.$sq.'+row_no+'.$sq.'&e='.$sq.'+start_at);">';
    $i = 0;
    $text = 1;
    while ($i < $count) {
      if ($i == $start_at) echo '<option selected="selected" style="font-weight: bold;" value="'.$i.'">'.strval($text).'</option>';
      else echo '<option value="'.$i.'">'.strval($text).'</option>';
      $i += $row_no;
      $text++;
    }
    echo '</select>&nbsp;</span></td>';
    echo '</tr></tbody></table>';
    echo '<div class="history"><table><tbody>';
    echo '<tr>';
    echo '<th style="width: 150px;">Date/Time</th>';
    echo '<th style="width: 100px;">Username</th>';
    echo '<th style="width: 500px;">Log Data</th>';
    echo '</tr>';
    while ($stmt->fetch()) {
      echo '<tr>';
      echo '<td style="width: 150px; text-align: center;">'.$date_time.'</td>';
      echo '<td style="width: 100px; text-align: center;">'.$user.'</td>';
      echo '<td style="width: 500px;">'.$log_data.'</td>';
      echo '</tr>';
    }
    echo '</tbody></table></div></div></div>';
    $stmt->close();
  }
  mysqli_close($conn);
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}

?>

