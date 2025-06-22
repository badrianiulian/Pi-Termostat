<?php
if (file_exists("config.php")) { include_once("config.php"); }
else { header("Refresh:0;URL=createdb.php"); }
include_once("functions.php");
sec_session_start();
if (login_check() == true) {
  $date = (isset($_REQUEST['q'])?$_REQUEST['q']:'');
  $test = date('Y-m-d',strtotime($date));
  if ($date == $test) {
    $color_no_data    = "#EEEEEE"; // There is no data to be shown
    $color_relay_on   = "#FAC090"; // Relay On background
    $color_relay_off  = "#8EB4E3"; // Relay Off background
    $color_program_th = "#0009FF"; // Program th_temp line color
    $color_manual_th  = "#C00000"; // Manual th_temp line color
    $color_off_th     = "#000000"; // OFF th_temp line color
    $color_real_temp  = "#953735"; // Real temp

    $js = <<<JS
var canvas = document.getElementById('GRAPH');
var h = (window.innerHeight - (window.innerHeight * 50 / 100));
var w = window.innerWidth;
canvas.width = w;
canvas.height = h;
var c = canvas.getContext('2d');

JS;
    $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
    if ($conn->connect_error)
      die("Connection failed: ".$conn->connect_error);
    $sql="SELECT TIMESTAMPDIFF(SECOND,'$date',`date_time`) AS `seconds`,`real_temp`,`mode`,`th_temp`,`relay` FROM `th_log` WHERE DATE(`date_time`) = '$date'";
    $js .= <<<JS
var data = {values: [
JS;
    if ($stmt=$conn->prepare($sql)) {
      $begin = 1;
      $stmt->execute();
      $stmt->store_result();
      $stmt->bind_result($s,$q,$m,$t,$r);
      // $s starting seconds position, $q real temperature, $m get mode, $t program temperature, $r relay status
      while ($stmt->fetch()) {
        if ($begin == 1) {
          $begin = 0;
          $js .= <<<JS
{ s: $s, q: $q, m: $m, t: $t, r: $r }
JS;
        } else {
          $js .= <<<JS
, { s: $s, q: $q, m: $m, t: $t, r: $r }
JS;
        }
      }
      $js .= <<<JS
]};
c.fillStyle='$color_no_data';
c.fillRect(0,0,w,h);

JS;
      $stmt->close();
    }
    mysqli_close($conn);
    $js .= <<<JS
if (data.values.length > 0) {
  var min = Infinity;
  var max = -Infinity;
  for (var i = 0; i < data.values.length; i++) {
    if (data.values[i].m != 0) {
      if (data.values[i].q < min) {
        min = data.values[i].q;
      }
      if (data.values[i].t < min) {
        min = data.values[i].t;
      }
      if (data.values[i].q > max) {
        max = data.values[i].q;
      }
      if (data.values[i].t > max) {
        max = data.values[i].t;
      }
    }
  }
  if ((min != Infinity) && (max != -Infinity)) {
    for (var i = 1; i < data.values.length; i++) {
      if (data.values[i-1].r == 1) c.fillStyle='$color_relay_on';
      else c.fillStyle='$color_relay_off';
      c.fillRect(((data.values[i-1].s * w) / 86400),0,((data.values[i].s * w) / 86400),h);
    }
    c.fillStyle='$color_no_data';
    c.fillRect(((data.values[data.values.length - 1].s * w) / 86400),0,w,h);

    var p = ((h / 2) + ((min + max) * 5));
    c.beginPath();
    c.moveTo(((data.values[0].s * w) / 86400),(p - (10 * data.values[0].q))); // starting point
    for (var i = 1; i < data.values.length; i++) {
      c.lineTo(((data.values[i].s * w) / 86400),(p - (10 * data.values[i].q))); // skew line to next point
    }
    c.lineWidth=3;
    c.strokeStyle='$color_real_temp';
    c.stroke();

    for (var i = 1; i < data.values.length; i++) {
      c.beginPath();
      var max_h = (((p - (10 * data.values[i-1].t)) > h)?h:(p - (10 * data.values[i-1].t)));
      c.moveTo(((data.values[i-1].s * w) / 86400),max_h); // starting point
      c.lineTo(((data.values[i].s * w) / 86400),max_h); // straight line right in vertical line with next point
      max_h = (((p - (10 * data.values[i].t)) > h)?h:(p - (10 * data.values[i].t)));
      c.lineTo(((data.values[i].s * w) / 86400),max_h); // straight line up/down to next point
      c.lineWidth=3;
      c.strokeStyle=((data.values[i-1].m == 1)?'$color_program_th':((data.values[i-1].m == 2)?'$color_manual_th':'$color_off_th'));
      c.stroke();
    }
  }
}

function getMousePos(canvas, evt) {
  var rect = canvas.getBoundingClientRect(), root = document.documentElement.getBoundingClientRect();
  // return relative mouse position
  var x = ~~((evt.clientX - rect.left - root.left) * 86400 / (rect.right - rect.left));
  return x;
}

function toTime(param) {
  var hours = ~~(param / 3600);
  var minutes = ~~((param - (hours * 3600)) / 60);
  var seconds = param - (hours * 3600) - (minutes * 60);
  var result = ((hours < 10)?'0':'') + hours + ':' + ((minutes < 10)?'0':'') + minutes + ':' + ((seconds < 10)?'0':'') + seconds
  return result;
}

canvas.addEventListener('mousemove', function(evt) {
  var pos = getMousePos(canvas, evt);
  var id_time = document.getElementById('TIME');
  var id_real_temp = document.getElementById('REAL_TEMP');
  var id_target_temp = document.getElementById('TARGET_TEMP');
  var id_mode = document.getElementById('MODE');
  var id_relay = document.getElementById('RELAY');
  id_time.innerHTML = toTime(pos);
  var i = 0;
  for (i = 1; (i < data.values.length); i++) {
    if ((data.values[i-1].s <= pos) && (pos <= data.values[i].s)) break;
  }
  if ((i == data.values.length) || (data.values.length == 0)) {
    id_real_temp.innerHTML = 'NO DATA';
    id_target_temp.innerHTML = 'NO DATA';
    id_mode.innerHTML = 'NO DATA';
    id_relay.innerHTML = 'NO DATA';
  } else {
    id_real_temp.innerHTML = '' + data.values[i-1].q.toFixed(3);
    id_target_temp.innerHTML = '' + ((data.values[i-1].m == 0)?'OFF':data.values[i-1].t.toFixed(3));
    id_mode.innerHTML = '' + ((data.values[i-1].m == 0)?'OFF':((data.values[i-1].m == 1)?'PROGRAM':'MANUAL'));
    id_relay.innerHTML = '' +  ((data.values[i-1].r == 0)?'OFF':'ON');
  }
});

canvas.addEventListener('mouseout', function(evt) {
  document.getElementById('TIME').innerHTML        = '&nbsp;';
  document.getElementById('REAL_TEMP').innerHTML   = '&nbsp;';
  document.getElementById('TARGET_TEMP').innerHTML = '&nbsp;';
  document.getElementById('MODE').innerHTML        = '&nbsp;';
  document.getElementById('RELAY').innerHTML       = '&nbsp;';
});

JS;

    header("Content-type: text/javascript");
    echo $js;
    exit();
  } else {
    $sq = "'";
    echo '<img src="img/empty.png" onload="if (!(typeof ReplaceAjax==='.$sq.'function'.$sq.')) window.location.href='.$sq.'main.php'.$sq.';this.parentNode.removeChild(this);" />';
  }
} else {
  header("Refresh:0;URL=login.php");
}
?>
