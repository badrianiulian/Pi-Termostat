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
?>
<!DOCTYPE html>
<html>
<head>
  <title>Control Thermostat</title>
  <link rel="apple-touch-icon" sizes="57x57" href="img/apple-icon-57x57.png">
  <link rel="apple-touch-icon" sizes="60x60" href="img/apple-icon-60x60.png">
  <link rel="apple-touch-icon" sizes="72x72" href="img/apple-icon-72x72.png">
  <link rel="apple-touch-icon" sizes="76x76" href="img/apple-icon-76x76.png">
  <link rel="apple-touch-icon" sizes="114x114" href="img/apple-icon-114x114.png">
  <link rel="apple-touch-icon" sizes="120x120" href="img/apple-icon-120x120.png">
  <link rel="apple-touch-icon" sizes="144x144" href="img/apple-icon-144x144.png">
  <link rel="apple-touch-icon" sizes="152x152" href="img/apple-icon-152x152.png">
  <link rel="apple-touch-icon" sizes="180x180" href="img/apple-icon-180x180.png">
  <link rel="icon" type="image/png" sizes="192x192"  href="img/android-icon-192x192.png">
  <link rel="icon" type="image/png" sizes="32x32" href="img/favicon-32x32.png">
  <link rel="icon" type="image/png" sizes="96x96" href="img/favicon-96x96.png">
  <link rel="icon" type="image/png" sizes="16x16" href="img/favicon-16x16.png">
  <link rel="manifest" href="manifest.json">
  <meta name="msapplication-TileColor" content="#ffffff">
  <meta name="msapplication-TileImage" content="img/ms-icon-144x144.png">
  <meta name="theme-color" content="#ffffff">
  <meta name="msapplication-config" content="browserconfig.xml">
  <link rel="stylesheet" href="css/standard.css?v=<?=time();?>" type="text/css">
  <script type="text/javascript" language="javascript" src="javascript/javascript.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/javascript_sec.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/aes.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/md5.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/pad-zeropadding-min.js?v=<?=time();?>"></script>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
</head>
<body>
<div class="topnav">
 <div class="usericon" onclick="ToggleId('usermenu','flex');">
  <div class="bars"></div>
  <div class="bars"></div>
  <div class="bars"></div>
 </div>
 <div class="usermenu" id="usermenu" style="display: none;">
  <div onclick="ReplaceAjax('MAIN', 'main_update', 'USER');setTimeout(function(){ToggleId('usermenu');},1000);">User &#9881;</div>
  <div onclick="self.location.href='logout.php';">Logout</div>
 </div>
 <div class="text">Control Thermostat</div>
</div>
<div id="MAIN" class="content" >
  <script type="text/javascript" language="javascript">
    ReplaceAjax('MAIN', 'main_update', 'MAIN');
  </script>
</div>
<div id="KEEP_ALIVE" style="display: none;"><img src="img/empty.png" onload="setTimeout(function(){ReplaceAjax('KEEP_ALIVE', 'alive');},60000);this.parentNode.removeChild(this);" /></div>
</body>
</html>
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
