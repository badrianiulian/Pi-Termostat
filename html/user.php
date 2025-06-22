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
    case "0": // List user accounts
      $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
      if ($conn->connect_error)
        die("Connection failed: ".$conn->connect_error);
      $sql="SELECT `id`,`user`,`attempt` FROM `th_users`;";
      echo '<div class="layout"><table><tbody>';
      echo '<tr>';
      echo '<td colspan=3><div class="layout"><span>User Accounts</span></div></td>';
      echo '<td><div class="layout"><button style="width: 95%;" onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'1'.$sq.');">&nbsp;<b>+</b>&nbsp;</button></div></td>';
      echo '</tr>';
      if ($stmt=$conn->prepare($sql)) {
        $stmt->execute();
        $stmt->store_result();
        $stmt->bind_result($id,$user,$attempt);
        while ($stmt->fetch()) {
          echo '<tr>';
          echo '<td><div class="layout"><span>&#128100;</span></div></td>';
          echo '<td><div class="layout"><span>'.$user.'</span></div></td>';
          echo '<td><div class="layout"><button onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'3&w='.$id.$sq.');">&nbsp;&#128274;&nbsp;</button></div></td>';
          echo '<td><div class="layout">';
          if ($stmt->num_rows != 1) echo '<button style="width: 95%" onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'5&w='.$id.$sq.');">&nbsp;&#9249;&nbsp;</button>';
          else echo '<span>&nbsp;</span>';
          echo '</div></td>';
          echo '</tr>';
        }
        $stmt->close();
      }
      echo '</tbody></table></div>';
      mysqli_close($conn);
      break;
    case "1": // Add user account form
      echo '<div class="layout"><table><tbody>';
      echo '<tr>';
      echo '<td colspan=2><div class="layout"><span>Create new user</span></div></td>';
      echo '</tr>';
      echo '<tr>';
      echo '<td><div class="layout"><span>Username:</span></div></td>';
      echo '<td><input type="text" id="user" value=""></td>';
      echo '</tr>';
      echo '<tr>';
      echo '<td><div class="layout"><span>Password:</span></div></td>';
      echo '<td><input type="text" name="pass" id="pass" class="text-password" autocomplete="off" value="" /><img src="img/empty.png" onload="TogglePass('.$sq.'pass'.$sq.');this.parentNode.removeChild(this);" /></td>';
      echo '</tr>';
      echo '<tr>';
      $client_side=client_sec_gen();
      $server_side=server_sec_gen($client_side);
      $js=<<<JS
(function(){
  var user = document.getElementById('user').value;
  var pass = document.getElementById('pass').value;
  if ((user == '') || (pass == '')) alert('Username or password not present!');
  else if (user.length < 6) alert('Minimum length for username has to be higher or equal than 6!');
  else if (pass.length < 8) alert('Minimum length for password has to be higher or equal than 8!');
  else if (!(usernameIsValid(user))) alert('Username must contain only numbers, letters, dot, underscore or minus characters!');
  else if (user == pass) alert('Username and password cannot be the same!');
  else {
    var text = user + ':' + pass;
    var text_enc = encodeURIComponent(encrypt(text,'$client_side','$server_side'));
    var args = '2&w=$client_side&e='+text_enc;
    ReplaceAjax('USER','user',args);
  }
})();
JS;
      echo '<td><div class="layout"><button onclick="'.$js.'">Create</button></div></td>';
      echo '<td><div class="layout"><button id="cancel" onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'0'.$sq.');">Cancel</button><img src="img/empty.png" onload="cancelTimer();this.parentNode.removeChild(this);" /></div></td>';
      echo '</tr>';
      echo '</tbody></table></div>';
      break;
    case "2": // Add user account processor
      $res = FALSE;
      if ((isset($_REQUEST['w'])) && (isset($_REQUEST['e']))) {
        $client_side = $_REQUEST['w'];
        $text_enc = $_REQUEST['e'];
        $server_side = get_sec_key($client_side);
        list($user,$pass) = explode(':',decryptOpenssl($text_enc,$client_side,$server_side));
        $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
        if ($conn->connect_error)
          die("Connection failed: ".$conn->connect_error);
        $sql = "INSERT INTO `th_users`(`user`,`pass`) VALUES (?,?);";
        if ($stmt=$conn->prepare($sql)) {
          $pass_enc = password_hash($pass, PASSWORD_DEFAULT);
          $stmt->bind_param('ss',$user,$pass_enc);
          $stmt->execute();
          log_sql($_SESSION['username'],'Success: Added user '.$user.'.');
          $stmt->close();
          $res = TRUE;
        }
        mysqli_close($conn);
      }
      if ($res) {
        echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      } else {
        echo '<img src="img/empty.png" onload="alert('.$sq.'Failed to add user '.$user.'! Something went wrong in the process or no connection!'.$sq.');ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    case "3": // Change user password form
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
        if ($conn->connect_error)
          die("Connection failed: ".$conn->connect_error);
        $sql="SELECT `user` FROM `th_users` WHERE `id`=$id;";
        if ($stmt=$conn->prepare($sql)) {
          $stmt->execute();
          $stmt->store_result();
          $stmt->bind_result($user);
          if ($stmt->num_rows == 1) {
            $stmt->fetch();
            echo '<div class="layout"><table><tbody>';
            echo '<tr>';
            echo '<td colspan=2><div class="layout"><span>Change password for user: '.$user.'</span></div></td>';
            echo '</tr>';
            echo '<tr>';
            echo '<td><div class="layout"><span>Old user'.$sq.'s password:</span></div></td>';
            echo '<td><input type="text" name="oldpass" id="oldpass" class="text-password" autocomplete="off" value="" /><img src="img/empty.png" onload="TogglePass('.$sq.'oldpass'.$sq.');this.parentNode.removeChild(this);" /></td>';
            echo '</tr>';
            echo '<tr>';
            echo '<td><div class="layout"><span>New user'.$sq.'s password:</span></div></td>';
            echo '<td><input type="text" name="newpass" id="newpass" class="text-password" autocomplete="off" value="" /><img src="img/empty.png" onload="TogglePass('.$sq.'newpass'.$sq.');this.parentNode.removeChild(this);" /></td>';
            echo '</tr>';
            echo '<tr>';
            $client_side=client_sec_gen();
            $server_side=server_sec_gen($client_side);
            $js=<<<JS
(function(){
  var oldpass = document.getElementById('oldpass').value;
  var newpass = document.getElementById('newpass').value;
  if ((oldpass == '') || (newpass == '')) alert('Passwords cannot be blank!');
  else if (newpass.length < 8) alert('Minimum length for new password has to be higher or equal than 8!');
  else if (oldpass == newpass) alert('Old password and new password cannot be the same!');
  else {
    var text = oldpass + ':' + newpass;
    var text_enc = encodeURIComponent(encrypt(text,'$client_side','$server_side'));
    var args = '4&w=$id&e=$client_side&r='+text_enc;
    ReplaceAjax('USER','user',args);
  }
})();
JS;
            echo '<td><div class="layout"><button onclick="'.$js.'">Modify</button></div></td>';
            echo '<td><div class="layout"><button id="cancel" onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'0'.$sq.');">Cancel</button><img src="img/empty.png" onload="cancelTimer();this.parentNode.removeChild(this);" /></div></td>';
            echo '</tr>';
            echo '</tbody></table></div>';
          }
          $stmt->close();
        }
        mysqli_close($conn);
      }
      break;
    case "4": // Change user password processor
      $res = FALSE;
      if ((isset($_REQUEST['w'])) && (isset($_REQUEST['e'])) && (isset($_REQUEST['r']))) {
        $id = $_REQUEST['w'];
        $client_side = $_REQUEST['e'];
        $text_enc = $_REQUEST['r'];
        $server_side = get_sec_key($client_side);
        list($oldpass,$newpass) = explode(':',decryptOpenssl($text_enc,$client_side,$server_side));
        $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
        if ($conn->connect_error)
          die("Connection failed: ".$conn->connect_error);
        $sql="SELECT `user`,`pass` FROM `th_users` WHERE `id` = ?;";
        if ($stmt=$conn->prepare($sql)) {
          $stmt->bind_param('i',$id);
          $stmt->execute();
          $stmt->store_result();
          $stmt->bind_result($db_user,$db_password);
          $stmt->fetch();
          if ($stmt->num_rows == 1) {
            if ((password_verify($oldpass, $db_password)) && ($oldpass != $newpass)) {
              // Password is correct!
              $pass= password_hash($newpass, PASSWORD_DEFAULT);
              $sql="UPDATE `th_users` SET `pass` = ? WHERE `id` = ?;";
              if ($stmt2=$conn->prepare($sql)) {
                $stmt2->bind_param('si',$pass,$id);
                $stmt2->execute();
                $stmt2->close();
                log_sql($_SESSION['username'],'Success: Password for user '.$db_user.' has been changed.');
                $res = TRUE;
              }
            } else {
              log_sql($_SESSION['username'],'Error: Tried to change password for user '.$db_user.' but failed. Wrong Password!');
            }
          }
          $stmt->close();
        }
        mysqli_close($conn);
      }
      if ($res) {
        if ($_SESSION['username'] == $db_user) echo '<img src="img/empty.png" onload="self.location.href='.$sq.'logout.php'.$sq.';this.parentNode.removeChild(this);" />';
        else echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      } else {
        echo '<img src="img/empty.png" onload="alert('.$sq.'Failed to change password for user id '.$id.'! Wrong password or no connection!'.$sq.');ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    case "5": // Delete user account form
      if (isset($_REQUEST['w'])) {
        $id = $_REQUEST['w'];
        $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
        if ($conn->connect_error)
          die("Connection failed: ".$conn->connect_error);
        $sql="SELECT `user` FROM `th_users` WHERE `id`=$id;";
        if ($stmt=$conn->prepare($sql)) {
          $stmt->execute();
          $stmt->store_result();
          $stmt->bind_result($user);
          $stmt->fetch();
          if ($stmt->num_rows == 1) {
            echo '<div class="layout"><table><tbody>';
            echo '<tr>';
            echo '<td colspan=2><div class="layout"><span>Confirm deletion of user: '.$user.'</span></div></td>';
            echo '</tr>';
            echo '<tr>';
            echo '<td><div class="layout"><span>User'.$sq.'s password:</span></div></td>';
            echo '<td><input type="text" name="pass" id="pass" class="text-password" autocomplete="off" value="" /><img src="img/empty.png" onload="TogglePass('.$sq.'pass'.$sq.');this.parentNode.removeChild(this);" /></td>';
            echo '</tr>';
            $client_side=client_sec_gen();
            $server_side=server_sec_gen($client_side);
            $js=<<<JS
(function(){
  var pass = document.getElementById('pass').value;
  if (pass == '') alert('Password field cannot be blank!');
  else {
    var text_enc = encodeURIComponent(encrypt(pass,'$client_side','$server_side'));
    var args = '6&w=$id&e=$client_side&r='+text_enc;
    ReplaceAjax('USER','user',args);
  }
})();
JS;
            echo '<td><div class="layout"><button onclick="'.$js.'">Delete</button></div></td>';
            echo '<td><div class="layout"><button id="cancel" onclick="ReplaceAjax('.$sq.'USER'.$sq.', '.$sq.'user'.$sq.', '.$sq.'0'.$sq.');">Cancel</button><img src="img/empty.png" onload="cancelTimer();this.parentNode.removeChild(this);" /></div></td>';
            echo '</tr>';
            echo '</tbody></table></div>';
          }
          $stmt->close();
        }
        mysqli_close($conn);
      }
      break;
    case "6": // Delete user account processor
      $res = FALSE;
      if ((isset($_REQUEST['w'])) && (isset($_REQUEST['e'])) && (isset($_REQUEST['r']))) {
        $id = $_REQUEST['w'];
        $client_side = $_REQUEST['e'];
        $text_enc = $_REQUEST['r'];
        $server_side = get_sec_key($client_side);
        $pass = decryptOpenssl($text_enc,$client_side,$server_side);
        $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
        if ($conn->connect_error)
          die("Connection failed: ".$conn->connect_error);
        $sql="SELECT COUNT(*) AS `count` FROM `th_users` ;";
        if ($stmt=$conn->prepare($sql)) {
          $stmt->execute();
          $stmt->store_result();
          $stmt->bind_result($count);
          $stmt->fetch();
          $stmt->close();
        }
        if ($count > 1) {
          $sql="SELECT `user`,`pass` FROM `th_users` WHERE `id` = ?;";
          if ($stmt=$conn->prepare($sql)) {
            $stmt->bind_param('i',$id);
            $stmt->execute();
            $stmt->store_result();
            $stmt->bind_result($db_user,$db_password);
            $stmt->fetch();
            if ($stmt->num_rows == 1) {
              if (password_verify($pass, $db_password)) {
                // Password is correct!
                $sql = "DELETE FROM `th_users` WHERE `id` = ?;";
                if ($stmt2=$conn->prepare($sql)) {
                  $stmt2->bind_param('s',$id);
                  $stmt2->execute();
                  $stmt2->close();
                  log_sql($_SESSION['username'],'Success: User '.$db_user.' has been deleted.');
                  $res = TRUE;
                }
              } else {
                log_sql($_SESSION['username'],'Error: Tried to delete user '.$db_user.' but failed. Wrong Password!');
              }
            }
            $stmt->close();
          }
        } else {
          log_sql($_SESSION['username'],'Error: Tried to delete the default user and failed.');
        }
        mysqli_close($conn);
      }
      if ($res) {
        if ($_SESSION['username'] == $db_user) echo '<img src="img/empty.png" onload="self.location.href='.$sq.'logout.php'.$sq.';this.parentNode.removeChild(this);" />';
        else echo '<img src="img/empty.png" onload="ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      } else {
        echo '<img src="img/empty.png" onload="alert('.$sq.'Failed to delete user id '.$id.'! Wrong password or no connection!'.$sq.');ReplaceAjax('.$sq.'MAIN'.$sq.', '.$sq.'main_update'.$sq.', '.$sq.'USER'.$sq.');this.parentNode.removeChild(this);" />';
      }
      break;
    default: // default no action
      break;
    }
  }
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
