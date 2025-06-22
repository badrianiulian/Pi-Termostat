<?php
if (file_exists("config.php")) { header("Refresh:0;URL=index.php"); }
?>
<html>
<head>
<title>Control Thermostat</title>
<link rel="stylesheet" href="css/standard.css?v=<?=time();?>" type="text/css">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<script type="text/javascript" language="javascript" src="javascript/javascript_sec.js?v=<?=time();?>"></script>
</head>
<body>
<?php
include_once("random.php");
$action="";
if(isset($_POST['action'])){
  $action=$_POST['action'];
}
if ($action=="")    /* display the form for database credentials*/
    {
    ?>
    <center>
    <div class="style-form" align="left">
    <form action="" method="POST" enctype="multipart/form-data" id="mainFrm">
    <fieldset>
    <input type="hidden" name="action" value="submit">
    <h2>To create the thermostat database, you need to provide some credentials:</h2><br>
    <table>
	<tr>
	  <td>MySQL administrator account:<span class="required">*</span></td>
	  <td><input name="user" id="user" type="text" autocomplete="off" value="root" /></td>
	</tr>
	<tr>
	  <td>MySQL administrator password:<span class="required">*</span></td>
	  <td>
            <input name="pass" id="pass" type="text" class="text-password" autocomplete="off" value="" />
            <script type="text/javascript">TogglePass("pass");</script>
          </td>
	</tr>
	<tr>
	  <td>Thermostat database name:<span class="required">*</span></td>
	  <td><input name="thdb" id="thdb" type="text" autocomplete="off" value="thermostat" /></td>
	</tr>
	<tr>
	  <td>Administrator account:<span class="required">*</span></td>
	  <td><input name="thuser" id="thuser" type="text" autocomplete="off" value="thermo" /></td>
	</tr>
	<tr>
	  <td>Administrator password:<span class="required">*</span><input name="randthpass" type="checkbox" value="" class="checkbox" onchange="togglethpass()" checked/>RANDOM
	    <script type="text/javascript">
		function togglethpass(){
		    if (document.getElementById("thpass").disabled == false) {
			document.getElementById("thpass").disabled = true;
			document.getElementById("thpass").value = "RANDOM";
		    }
		    else {
			document.getElementById("thpass").disabled = false;
			document.getElementById("thpass").value = "";
		    }
		}
	    </script>
	  </td>
	  <td>
	    <input name="thpass" id="thpass" type="text" autocomplete="off" class="text-password" value="RANDOM" disabled=true />
	    <script type="text/javascript">TogglePass("thpass");</script>
	  </td>
	</tr>
    </table>
    <br>
    <center><input type="submit" value="Create Thermostat Database" /></center>
    </fieldset>
    </form>
    </div>
    </center>
    <?php
    }
else                /* send the submitted data */
    {
    $hostserver = "localhost";
    $user=$_REQUEST['user'];
    $pass=$_REQUEST['pass'];
    $thdb=$_REQUEST['thdb'];
    $thuser=$_REQUEST['thuser'];
    if(isset($_REQUEST['randthpass'])) {
	$thpass = "RANDOM";
    } else {
	$thpass=$_REQUEST['thpass'];
    }
    if (($user=="") || ($pass=="") || ($thdb=="") || ($thuser=="") || ($thpass==""))
        {
		echo "All fields are required, please fill the <a href=\"\">credentials</a> again.";
	    }
    else{
		set_time_limit(1000);
		/* Check mysql credentials */
		$conn = new mysqli($hostserver,$user,$pass);
		if ($conn->connect_error) {
		  die("Connection failed: ".$conn->connect_error);
		}
		/* Begin database creation */
		include("functions.php");
		if($thpass=="RANDOM") {
		    $thpass = random_str(37);
		}
		$logname = random_str(8,"file");
		$vf = fopen("$logname.log", "w");
		fwrite($vf,date('Y-F-d H:i:s')." Connected to database succesfully!\n");
		$sql_string = "DROP USER IF EXISTS '".$thuser."'@'$hostserver'";
		fwrite($vf,date('Y-F-d H:i:s')." DROP USER IF EXISTS 'USER'@'$hostserver'\n");
		if (!$conn->query($sql_string)) { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
		$sql_string = "DROP DATABASE IF EXISTS `".$thdb."`";
		fwrite($vf,date('Y-F-d H:i:s')." DROP DATABASE IF EXISTS `THERMOSTAT`\n");
		if (!$conn->query($sql_string)) { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
		$sql_string = "CREATE USER '".$thuser."'@'$hostserver' IDENTIFIED BY '".$thpass."'";
		fwrite($vf,date('Y-F-d H:i:s')." CREATE USER 'THUSER'@'$hostserver' IDENTIFIED BY 'PASSWORD'\n");
		if (!$conn->query($sql_string)) { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
		$sql_string = "CREATE DATABASE `".$thdb."`";
		fwrite($vf,date('Y-F-d H:i:s')." CREATE DATABASE `THERMOSTAT`\n");
		if (!$conn->query($sql_string)) { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
		$sql_string = "GRANT ALL ON `".$thdb."`.* TO '".$thuser."'@'$hostserver'";
		fwrite($vf,date('Y-F-d H:i:s')." GRANT ALL ON `THERMOSTAT`.* TO 'THUSER'@'$hostserver'\n");
		if (!$conn->query($sql_string)) { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
                $conn->close();
		/* Create tables using the user with the database proper privileges */
		$conn = new mysqli($hostserver,$thuser,$thpass,$thdb);
		if ($conn->connect_error) {
		  die("Connection failed: ".$conn->connect_error);
		}
		$handle = file_get_contents('data/thermostat.sql');
		$oldText = "DEFINER=`thermo`@`localhost` ";
		$newText = "";   // remove the above line
		$handle=str_replace("$oldText", "$newText",$handle);
		$oldText = "CHARSET utf8mb4 ";
		$newText = "";   // remove the above line
		$handle=str_replace("$oldText", "$newText",$handle);
		$oldText = "$$"; // replace delimiters $$
		$newText = ";";  // with ;
		$handle=str_replace("$oldText", "$newText",$handle);
		$oldText = "DELIMITER ;";
		$newText = "";   // remove the above line
		$handle=str_replace("$oldText", "$newText",$handle);
		fwrite($vf,date('Y-F-d H:i:s')." Replaced default data in sql file. Starting insertion.\n");
		if (!$conn->multi_query($handle))  { fwrite($vf,date('Y-F-d H:i:s')." ".$conn->error); }
		do
		{
		  // Store first result set
		  if ($result=mysqli_store_result($conn))
		  {
		    while ($row=mysqli_fetch_row($result))
		    {
		      fwrite($vf,date('Y-F-d H:i:s')." ".$row[0]."\n");
		      $error=mysqli_error($conn);
		      if($error!='') fwrite($vf,date('Y-F-d H:i:s')." error: $error\n");
		    }
		    mysqli_free_result($conn);
		  }
		}
		while (mysqli_next_result($conn));
		fwrite($vf,date('Y-F-d H:i:s')." Finished sql data insertion.\n");
		/* Insert user defined into the database */
		$thpasshash = password_hash("thermo", PASSWORD_DEFAULT);
                $thuserpass = "('$thuser','$thpasshash')";
		$sql_string = "INSERT INTO `$thdb`.`th_users`(`user`,`pass`) VALUES $thuserpass";
		if (!$conn->query($sql_string)) {
                  fwrite($vf,date('Y-F-d H:i:s')." Query failed: INSERT INTO `$thdb`.`th_users`(`user`,`pass`)\n".$conn->error);
                  die("Query failed: INSERT INTO `$thdb`.`th_users`(`user`,`pass`)<br>".$conn->error);
                }
                else {
                  fwrite($vf,date('Y-F-d H:i:s')." Added user to the database!\n");
                }
		/* Insert event for database creation */
		$data = "('$thuser','Database created succesfully')";
		$sql_string = "INSERT INTO `$thdb`.`th_act_log`(`user`,`log_data`) VALUES $data";
		if (!$conn->query($sql_string)) {
                  fwrite($vf,date('Y-F-d H:i:s')." Query failed: INSERT INTO `$thdb`.`th_act_log`(`date_time`,`user`,`log_data`)\n".$conn->error);
                  die("Query failed: INSERT INTO `$thdb`.`th_act_log`(`date_time`,`user`,`log_data`)<br>".$conn->error);
                }
                else {
                  fwrite($vf,date('Y-F-d H:i:s')." Database creation finished!\n");
		  $cf = fopen("config.php", "w");
		  $contents = "<?php\ndefine('TH_HOST', '$hostserver');\ndefine('DB_USER', '$thuser');\ndefine('DB_PASS', '$thpass');\ndefine('TH_DB', '$thdb');\n?>";
		  fwrite($cf, $contents);
		  fclose($cf);
		  echo "<meta http-equiv=\"refresh\" content=\"0;URL=index.php\">";
                }
		fclose($vf);
                $conn->close();
	    }
    }
?>
</body>
</html>
