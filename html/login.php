<?php
if (!(file_exists("config.php"))) { header("Refresh:0;URL=createdb.php"); }
$action=(isset($_POST['action'])?$_POST['action']:"");
include_once("functions.php");
if ($action=="") {
$client_side=client_sec_gen();
$server_side=server_sec_gen($client_side);
$js=<<<JS
<script>
  var d = new Date();
  d.setMinutes(d.getMinutes()+1);
  var future = d.getTime();
  var counter = setInterval(function() {
    var now = new Date().getTime();
    var remain = future - now;
    var seconds = Math.floor((remain % (1000 * 60)) / 1000);
    document.getElementById('counter').innerHTML = seconds;
    if (remain < 1) {
      clearInterval(counter);
      window.location.href='index.php';
    }
  }, 1000);
  var f = document.getElementById('mainFrm');
  f.onsubmit = function(){
    clearInterval(counter);
    if ((f.user.value == '') || (f.pass.value == '')) {
      alert('Username or password not present!');
      event.preventDefault();
    }
    else {
      f.user.setAttribute('type', 'password');
      f.user.value = encrypt(f.user.value,'$client_side','$server_side');
      f.pass.value = encrypt(f.pass.value,'$client_side','$server_side');
      f.salt.value = '$client_side';
    }
  }
</script>
JS;
?>
<!DOCTYPE html>
<html>
<head>
  <title>Control Thermostat</title>
  <link rel="stylesheet" href="css/standard.css?v=<?=time();?>" type="text/css">
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <script type="text/javascript" language="javascript" src="javascript/javascript_sec.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/aes.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/md5.js?v=<?=time();?>"></script>
  <script type="text/javascript" language="javascript" src="javascript/pad-zeropadding-min.js?v=<?=time();?>"></script>
</head>
<body>
  <center>
    <div class="style-form" align="left">
      <form action="" method="POST" enctype="multipart/form-data" id="mainFrm" autocomplete="false">
        <fieldset>
          <input type="hidden" name="action" value="submit">
          <h2>Control Thermostat</h2><br>
          <table>
            <tbody>
              <tr>
                <td>Username:<span class="required">*</span></td>
                <td><input type="text" name="user" id="user" autocomplete="off" value="" /></td>
              </tr>
              <tr>
                <td>Password:<span class="required">*</span></td>
                <td><input type="text" name="pass" id="pass" class="text-password" autocomplete="off" value="" />
                    <script type="text/javascript">TogglePass("pass");</script>
                </td>
              </tr>
            </tbody>
          </table>
          <input type="hidden" name="salt" id="salt" value="" /><?php echo $js; ?>
          <br/>
          <center><input type="submit" value="Login" /></center>
          <span style="float: right;">Reloading page in:&nbsp;<div id="counter" style="float: right;">60</div></span>
        </fieldset>
      </form>
    </div>
  </center>
<br/>
  <center>
    <div class="style-form">
      <h2>&#9888; This is a private site.<br />Cookies are used for security reasons.</h2>
    </div>
  </center>
</body>
</html>
<?php
} else {
  sec_session_start();
  $user_enc=(isset($_POST['user'])?$_POST['user']:"");
  $pass_enc=(isset($_POST['pass'])?$_POST['pass']:"");
  $client_side=(isset($_POST['salt'])?$_POST['salt']:"");
  if (($user_enc=="") || ($pass_enc=="") || ($client_side=="")) {
    echo "All fields are required, please fill the <a href=\"\">credentials</a> again.";
  } else {
    $server_side = get_sec_key($client_side);
    $user = decryptOpenssl($user_enc,$client_side,$server_side);
    $pass = decryptOpenssl($pass_enc,$client_side,$server_side);
    if (login($user,$pass) == true) {
?>
<img src="img/empty.png" onload="window.location.href='main.php';this.parentNode.removeChild(this);" />
<?php
    } else {
?>
<img src="img/empty.png" onload="window.location.href='error.php?err=Wrong username or password!';this.parentNode.removeChild(this);" />
<?php
    }
  }
}
?>
