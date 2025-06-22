<?php
if (file_exists("config.php")) { include_once("config.php"); }
else {
?>
<img src="img/empty.png" onload="window.location.href='createdb.php';this.parentNode.removeChild(this);" />
<?php
}

function select_sql($funct,$argument="")
{
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $sql="SELECT $funct($argument);";
  $stmt=$conn->prepare($sql);
  $stmt->execute();
  $stmt->store_result();
  $stmt->bind_result($sql_data);
  $stmt->fetch();
  $stmt->close();
  mysqli_close($conn);
  return $sql_data;
}

function call_sql($funct,$argument="")
{
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $sql="CALL $funct($argument);";
  $stmt=$conn->prepare($sql);
  $stmt->execute();
  $stmt->close();
  mysqli_close($conn);
}

function log_sql($user,$message)
{
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $sql="INSERT INTO `th_act_log` (`user`, `log_data`) VALUES (?,?);";
  $stmt=$conn->prepare($sql);
  $stmt->bind_param('ss',$user,$message);
  $stmt->execute();
  $stmt->close();
  mysqli_close($conn);
}

function decryptOpenssl($data,$client_side,$server_side) {
  return openssl_decrypt(base64_decode($data),"AES-128-CBC",$client_side,OPENSSL_RAW_DATA,$server_side);
}

function encryptOpenssl($data,$client_side,$server_side) {
  return base64_encode(openssl_encrypt($data,"AES-128-CBC",$client_side,OPENSSL_RAW_DATA,$server_side));
}

function client_sec_gen()
{
  include_once("random.php");
  $client_side=random_str(16,"iv");
  return $client_side;
}

function server_sec_gen($client_side)
{
  include_once("random.php");
  $server_side=random_str(16,"key");
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $sql="INSERT INTO `th_sec` (`client_side`,`server_side`) VALUES (?,?);";
  if ($stmt=$conn->prepare($sql)) {
    $stmt->bind_param('ss',$client_side,$server_side);
    $stmt->execute();
    $stmt->close();
    $sql="CREATE EVENT `$client_side` ON SCHEDULE AT CURRENT_TIMESTAMP + INTERVAL 1 MINUTE ON COMPLETION NOT PRESERVE DO DELETE FROM `th_sec` WHERE `client_side`='$client_side';";
    if ($conn->query($sql) === FALSE)
      die("Connection failed: ".$conn->connect_error);
  }
  mysqli_close($conn);
  return $server_side;
}

function get_sec_key($client_side) {
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $res = '';
  $sql="SELECT `server_side` FROM `th_sec` WHERE `client_side`=?";
  if ($stmt=$conn->prepare($sql)) {
    $stmt->bind_param('s',$client_side);
    $stmt->execute();
    $stmt->store_result();
    $stmt->bind_result($server_side);
    $stmt->fetch();
    if ($stmt->num_rows == 1) {
      $res = $server_side;
    }
    $stmt->close();
  }
  mysqli_close($conn);
  return $res;
}

function sec_session_start() {
  $session_name = 'sec_session_id_thermo';
  $secure = false; // Using http (not https)
  $httponly = true; // Stop js from accessing session id
  // Forces sessions to only use cookies.
  if (ini_set('session.use_only_cookies', 1) === FALSE) {
    echo '<img src="img/empty.png" onload="window.location.href='."'".'error.php?err=Could not initiate a safe session (ini_set)'."'".';this.parentNode.removeChild(this);" />';
    exit();
  }
  // Gets current cookies params.
  $cookieParams = session_get_cookie_params();
  session_name($session_name); // Set the session name as defined above
  session_set_cookie_params($cookieParams["lifetime"],$cookieParams["path"],$cookieParams["domain"],$secure,$httponly);
  session_start(); // Start PHP session
  //if (!isset($_SESSION['TIME'])) session_regenerate_id(true); // Replace old session with new one
  session_regenerate_id();
}

function login($username, $password) {
  // Using prepared statements means that SQL injection is not possible.
  $conn = new mysqli(TH_HOST,DB_USER, DB_PASS,TH_DB);
  if ($conn->connect_error)
    die("Connection failed: ".$conn->connect_error);
  $res = FALSE;
  $sql="SELECT `id`, `user`, `pass`, `attempt` FROM `th_users` WHERE `user` = ?;";
  if ($stmt=$conn->prepare($sql)) {
    $stmt->bind_param('s',$username);
    $stmt->execute();
    $stmt->store_result();
    $stmt->bind_result($db_id,$db_username,$db_password,$db_attempt);
    $stmt->fetch();
    if ($stmt->num_rows == 1) {
      // Login attempts limit set to 5
      if ($db_attempt >= 5) {
        // Account is locked
        // Reset possible only from local keyboard
        log_sql($username,'Error: User tried to login but is locked out.');
      } else {
        // Check password against database hash
        if (password_verify($password, $db_password)) {
          // Password is correct!
          // Get the user-agent string of the user.
          $ip_address = $_SERVER['REMOTE_ADDR'];
          $user_browser = $_SERVER['HTTP_USER_AGENT'];
          // XSS protection as we might print this value
          $db_id = preg_replace("/[^0-9]+/", "", $db_id);
          $_SESSION['user_id'] = $db_id;
          // XSS protection as we might print this value
          $db_username = preg_replace("/[^a-zA-Z0-9_\-]+/","",$db_username);
          $_SESSION['username'] = $db_username;
          $_SESSION['login_string'] = hash('sha512',$db_password.$ip_address.$user_browser);
          // Login successful.
          if ($db_attempt != 0) {
            $sql="UPDATE `th_users` SET `attempt`=0 WHERE `id` = ?;";
            if ($stmt2=$conn->prepare($sql)) {
              $stmt2->bind_param('i',$db_id);
              $stmt2->execute();
              $stmt2->close();
            }
          }
          log_sql($username,'Success: User connected.');
          $res = TRUE;
        } else {
          // Password incorrect
          $db_attempt = $db_attempt + 1;
          // Update attempt in the database
          $sql="UPDATE `th_users` SET `attempt`=? WHERE `id` = ?;";
          if ($stmt2=$conn->prepare($sql)) {
            $stmt2->bind_param('ii',$db_attempt,$db_id);
            $stmt2->execute();
            $stmt2->close();
          }
          log_sql($username,'Error: Login attempt! Incorect password. Attempt number '.$db_attempt.'.');
        }
      }
    } else {
      // No user found.
      log_sql($username,'Error: Login attempt! Incorect username.');
    }
    $stmt->close();
  }
  mysqli_close($conn);
  return $res;
}

function login_check() {
  // Check if all session variables are set
  $res = FALSE;
  if (isset($_SESSION['user_id'],$_SESSION['username'],$_SESSION['login_string'])) {
    $user_id = $_SESSION['user_id'];
    $login_string = $_SESSION['login_string'];
    $username = $_SESSION['username'];
    // Get the user-agent string of the user.
    $ip_address = $_SERVER['REMOTE_ADDR'];
    $user_browser = $_SERVER['HTTP_USER_AGENT'];
    $conn = new mysqli(TH_HOST,DB_USER,DB_PASS,TH_DB);
    if ($conn->connect_error)
      die("Connection failed: ".$conn->connect_error);
    $sql="SELECT `pass` FROM `th_users` WHERE `id` = ?;";
    if ($stmt=$conn->prepare($sql)) {
      $stmt->bind_param('i',$user_id);
      $stmt->execute();
      $stmt->store_result();
      if ($stmt->num_rows == 1) {
        $stmt->bind_result($db_password);
        $stmt->fetch();
        $login_check = hash('sha512',$db_password.$ip_address.$user_browser);
        if (hash_equals($login_check, $login_string) ){
          // Logged In!!!!
          $res = TRUE;
        }
      }
      $stmt->close();
    }
    mysqli_close($conn);
  }
  return $res;
}

?>
