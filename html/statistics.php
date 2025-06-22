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
  $date = (isset($_REQUEST['q'])?$_REQUEST['q']:select_sql("DATE","NOW()"));
?>
<span>
<table>
  <tbody align="center">
    <tr>
      <td><button style="width: 7vw;" onclick="(function(){
          removeHeadId('CanvasScriptID');
          var element=document.getElementById('SELECTED_DATE');
          var date=element.innerHTML;
          ReplaceAjax('STATISTICS', 'statistics', YesterdayMySql(date));
        })();">&#x25C0;</button></td>
      <td><button id="SELECTED_DATE" style="width: 81vw;" onclick="(function(){
          var element = document.getElementById('CALENDAR');
          if (element.style.display === 'none') {
            element.style.display = 'block';
            element=document.getElementById('SELECTED_DATE');
            var date=element.innerHTML;
            drawCalendar(date);
          } else {
            element.style.display = 'none';
            element=document.getElementById('SELECTED_DATE');
            var date=element.innerHTML;
            if (date != <?php echo $sq.$date.$sq; ?>) {
              removeHeadId('CanvasScriptID');
              ReplaceAjax('STATISTICS', 'statistics', date);
            }
          }
        })();"><?php echo $date; ?></button></td>
      <td><button style="width: 7vw;" onclick="(function(){
          removeHeadId('CanvasScriptID');
          var element=document.getElementById('SELECTED_DATE');
          var date=element.innerHTML;
          ReplaceAjax('STATISTICS', 'statistics', TomorowMySql(date));
        })();">&#x25B6;</button></td>
    </tr>
  </tbody>
</table>
</span>
<div id="CALENDAR" style="display:none;"></div>
<canvas id="GRAPH"></canvas>
<img src="img/empty.png" <?php
echo 'onload="if (!(typeof ReplaceAjax==='.$sq.'function'.$sq.')) window.location.href='.$sq.'main.php'.$sq.';var script=document.createElement('.$sq.'script'.$sq.');script.src='.$sq.'canvas.php?q='.$date.'&v='.time().$sq.';script.type='.$sq.'text/javascript'.$sq.';script.defer=true;script.id='.$sq.'CanvasScriptID'.$sq.';var head=document.getElementsByTagName('.$sq.'head'.$sq.').item(0);head.appendChild(script);this.parentNode.removeChild(this);"'; ?> />
<table>
 <tbody>
  <tr>
   <td><span style="width: 18vw">Time</span></td>
   <td><span style="width: 18vw">Real</span></td>
   <td><span style="width: 18vw">Target</span></td>
   <td><span style="width: 18vw">Mode</span></td>
   <td><span style="width: 18vw">Relay</span></td>
  </tr>
  <tr>
   <td><span id="TIME"        style="width: 18vw">&nbsp;</span></td>
   <td><span id="REAL_TEMP"   style="width: 18vw">&nbsp;</span></td>
   <td><span id="TARGET_TEMP" style="width: 18vw">&nbsp;</span></td>
   <td><span id="MODE"        style="width: 18vw">&nbsp;</span></td>
   <td><span id="RELAY"       style="width: 18vw">&nbsp;</span></td>
  </tr>
 </tbody>
</table>
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
