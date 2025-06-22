<?php
$err=(isset($_REQUEST['err'])?$_REQUEST['err']:"");
if ($err!="") echo 'Error: '.$err;
else echo "Something went wrong";
?>
