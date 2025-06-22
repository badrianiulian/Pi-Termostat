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
  $program_id = (isset($_REQUEST['q'])?$_REQUEST['q']:select_sql('get_active_program_id'));
  $program_active_id = select_sql('get_active_program_id');
  $program = select_sql('get_program_name',$program_id);
  $left = select_sql('get_program_id_prev',$program_id);
  $right = select_sql('get_program_id_next',$program_id);
?>
<span class="content">
  <table>
    <tbody align="center">
      <tr>
        <td><button style="width: 7vw;" <?php echo 'onclick="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.', '.$sq.$left.$sq.');"';?>>&#x25C0;</button></td>
        <td><button style="width: 80.5vw;" onclick="ToggleId('PROG_OPTIONS','block');"><?php echo $program; if ($program_id==$program_active_id) echo " (active)"; ?></button></td>
        <td><button style="width: 7vw;" <?php echo 'onclick="ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs'.$sq.', '.$sq.$right.$sq.');"';?>>&#x25B6;</button></td>
      </tr>
    </tbody>
  </table>

<div id="PROG_OPTIONS">
  <table>
    <tbody align="center">
      <tr>
        <td><input id="NEW_PROG_NAME" style="width: 59vw;" type="text" <?php echo 'value="'.$program.'"' ?> />
          <script>
            var input = document.getElementById('NEW_PROG_NAME');
            input.addEventListener("keyup", function(event) {
              if (event.keyCode === 13) {
                event.preventDefault();
                document.getElementById("CHANGE_PROG_NAME").click();
              }
            });
          </script></td>
        <td><button id="CHANGE_PROG_NAME" style="width: 35.5vw;" onclick="(function(){
          var element=document.getElementById('NEW_PROG_NAME');
          if (element.value == <?php echo $sq.$program.$sq; ?>) {
            alert('Please type a new program name!');
          } else {
            var args=<?php echo $sq.'2&w='.$program_id.'&e='.$sq; ?>+element.value;
            ReplaceAjax('PROGRAMS', 'programs_modif', args);
          }
        })();">Change</button></td>
      </tr>
    </tbody>
  </table>
  <table>
    <tbody align="center">
      <tr>
        <td><button style="width: 47.5vw;" id="CLONE_PROGRAM" onclick="(function(){
          var element=document.getElementById('NEW_PROG_NAME');
          var args=<?php echo $sq.'3&w='.$program_id.'&e='.$sq; ?>+element.value;
          ReplaceAjax('PROGRAMS', 'programs_modif', args);
        })();">Clone Program</button></td>
        <td><button style="width: 47.5vw;" id="DELETE_PROGRAM" onclick="(function(){
          <?php
            if ($program_active_id==$program_id)
              echo 'alert('.$sq.'Program is set as default active program! Cannot delete current program!'.$sq.');';
            else
              if ($program_id==1)
                echo 'alert('.$sq.'This is the base template program and it cannot be deleted!'.$sq.');';
              else
                echo 'ReplaceAjax('.$sq.'PROGRAMS'.$sq.', '.$sq.'programs_modif'.$sq.', '.$sq.'4&w='.$program_id.$sq.');';
          ?>
        })();">Delete Program</button></td>
      </tr>
      <tr>
        <td><button style="width: 47.5vw;" <?php echo 'onclick="ReplaceAjax('.$sq.'PROGRAMS_DATA'.$sq.', '.$sq.'programs_modif'.$sq.', '.$sq.'5&w='.$program_id.'&e=0'.$sq.');"';?>>All &#8451; Up</button></td>
        <td><button style="width: 47.5vw;" <?php echo 'onclick="ReplaceAjax('.$sq.'PROGRAMS_DATA'.$sq.', '.$sq.'programs_modif'.$sq.', '.$sq.'5&w='.$program_id.'&e=1'.$sq.');"';?>>All &#8451; Down</button></td>
      </tr>
    </tbody>
  </table>
</div>
</span>

<div id="PROGRAMS_DATA" class="layout">
  <?php echo '<img src="img/empty.png" onload="if (!(typeof ReplaceAjax==='.$sq.'function'.$sq.')) window.location.href='.$sq.'main.php'.$sq.';ReplaceAjax('.$sq.'PROGRAMS_DATA'.$sq.', '.$sq.'programs_data'.$sq.', '.$program_id.');this.parentNode.removeChild(this);" />'; ?>
</div>
<?php
} else {
?>
<img src="img/empty.png" onload="window.location.href='login.php';this.parentNode.removeChild(this);" />
<?php
}
?>
