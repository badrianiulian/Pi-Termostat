function ReplaceAjax(id,php,content) {
  if (window.XMLHttpRequest)
  {// code for IE7+, Firefox, Chrome, Opera, Safari
    xhttp=new XMLHttpRequest();
  }
  else
  {// code for IE6, IE5
    xhttp=new ActiveXObject("Microsoft.XMLHTTP");
  }
  if (typeof content !== 'undefined') xhttp.open("GET",php+".php?q="+content,true);
  else xhttp.open("GET",php+".php",true);
  xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhttp.onload = function() {
    var element = document.getElementById(id);
    if (element != null) {
      element.innerHTML = this.response;
    }
  };
  xhttp.send();
}

function openTab(evt, numetab) {
    var i, content, tablink, xhttp; /* Declare variables */
    content = document.getElementsByClassName("tabcontent");
    for (i = 0; i < content.length; i++) {
        content[i].style.display = "none"; /* Hyde class="tabcontent" elements  */
    }
    tablink = document.getElementsByClassName("tablink");
    for (i = 0; i < tablink.length; i++) { /* Disable class="tablink" elements  */
        tablink[i].className = tablink[i].className.replace(" active", "");
    }
    document.getElementById(numetab).style.display = "block"; /* Show current tab */
    evt.currentTarget.className += " active"; /* Set current tab button as active */
}

function removeHeadId(id){
    var head = document.getElementsByTagName('head').item(0);
    var old;
    while (old = document.getElementById(id)) head.removeChild(old);
}

function ToggleId(id,type) {
    type = type || "none";
    var element = document.getElementById(id);
    if (element.style.display === "none") {
      element.style.display = type;
    } else {
      element.style.display = "none";
    }
}

function cancelTimer() {
    var d = new Date();
    d.setMinutes(d.getMinutes()+1);
    var future = d.getTime();
    var attached = false;
    var counter = setInterval(function() {
      if (attached == false) {
        document.getElementById("cancel").addEventListener('click', function() { clearInterval(counter); }, false);
        attached = true;
      }
      var now = new Date().getTime();
      var remain = future - now;
      var seconds = Math.floor((remain % (1000 * 60)) / 1000);
      document.getElementById("cancel").innerHTML = "Cancel ("+seconds+")";
      if (remain < 1) {
        clearInterval(counter);
        document.getElementById("cancel").click();
      }
    }, 1000);
}

function monthName(param) {
  var month = ['JAN','FEB','MAR','APR','MAY','JUN','JUL','AUG','SEP','OCT','NOV','DEC'];
  return month[param];
}

function DateToMySql(d) {
  var year  = d.getFullYear();
  var month = d.getMonth()+1;
  var day   = d.getDate();
  return year + "-" + ((month < 10)?"0":"") + month + "-" + ((day < 10)?"0":"") + day;
}

function MySqlToDate(param) {
  var d = new Date(Date.parse(param.replace(/[-]/g,'/')));
  return d;
}

function YesterdayMySql(param) {
  var d = MySqlToDate(param);
  var n = new Date(d.getFullYear(), d.getMonth(), d.getDate()-1);
  return DateToMySql(n);
}

function TomorowMySql(param) {
  var d = MySqlToDate(param);
  var n = new Date(d.getFullYear(), d.getMonth(), d.getDate()+1);
  return DateToMySql(n);
}

function drawCalendar(param,type){
  type = type || 'DD';
  var d = MySqlToDate(param);
  var currentDay   = d.getDate();
  var currentMonth = d.getMonth();
  var currentYear  = d.getFullYear();
  if (type == 'DD') {
    var prevMonth = new Date(currentYear, currentMonth-1, currentDay);
    var nextMonth = new Date(currentYear, currentMonth+1, currentDay);
    var firstDay  = new Date(currentYear, currentMonth, 1);
    var lastDay   = new Date(currentYear, currentMonth+1, 0);
    var colStart = firstDay.getDay();
    var stopAt   = lastDay.getDate();
    if (colStart == 0) colStart = 6;
    else colStart--;
    var _table = '<table><tbody>';
    _table += '<tr>';
    var rows = 6; // 6 rows
    var cols = 7; // 7 cols
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(prevMonth) + '\');\">&#x25C0;</button></td>';
    _table += '<td colspan='+cols+'><button class=\"calendar\" onclick=\"drawCalendar(\''+DateToMySql(d)+'\',\'MM\');\">' + monthName(currentMonth) + ' ' + currentYear + '</button></td>';
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(nextMonth) + '\');\">&#x25B6;</button></td>';
    _table += '</tr>';
    var c = 1;
    for(var i = 0; i < rows; i++) {
      _table += '<tr><td><span class=\"calendar\">&nbsp;</span></td>';
      for(var j = 0; j < cols; j++) {
        _table += '<td>';
        if (((i == 0) && (j >= colStart)) || ((i > 0) && (c <= stopAt))) {
          d.setDate(c);
          _table += '<button'+((c==currentDay)?' class=\"active calendar\"':' class=\"calendar\"')+' onclick=\"(function(){var element=document.getElementById(\'SELECTED_DATE\');element.innerHTML=\'' + DateToMySql(d) + '\';element.click();})();\">'+c+'</button>';
          c++;
        } else {
          _table += '<span class=\"calendar\">&nbsp;</span>';
        }
        _table += '</td>';
      }
      _table += '<td><span class=\"calendar\">&nbsp;</span></td></tr>';
    }
    _table += '</tbody></table>';
  } else if (type == 'MM') {
    var prevYear = new Date(currentYear-1, currentMonth, currentDay);
    var nextYear = new Date(currentYear+1, currentMonth, currentDay);
    var _table = '<table><tbody>';
    var rows = 4; // 4 rows
    var cols = 3; // 3 cols
    _table += '<tr>';
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(prevYear) + '\',\'MM\');\">&#x25C0;</button></td>';
    _table += '<td colspan='+cols+'><button class=\"calendar\" onclick=\"drawCalendar(\''+DateToMySql(d)+'\',\'YY\');\">' + currentYear + '</button></td>';
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(nextYear) + '\',\'MM\');\">&#x25B6;</button></td>';
    _table += '</tr>';
    var c = 0;
    for(var i = 0; i < rows; i++) {
      _table += '<tr><td><span class=\"calendar\">&nbsp;</span></td>';
      for(var j = 0; j < cols; j++, c++) {
        c = j + i * cols;
        d.setMonth(c);
        _table += '<td><button'+((c==currentMonth)?' class=\"active calendar\"':' class=\"calendar\"')+' onclick=\"drawCalendar(\'' + DateToMySql(d) + '\');\">'+monthName(c)+'</button></td>';
      }
      _table += '<td><span class=\"calendar\">&nbsp;</span></td></tr>';
    }
    _table += '</tbody></table>';
  } else if (type == 'YY') {
    var prevYear = new Date(currentYear-9, currentMonth, currentDay);
    var nextYear = new Date(currentYear+9, currentMonth, currentDay);
    var _table = '<table><tbody>';
    var rows = 5; // 3 rows
    var cols = 3; // 3 cols
    _table += '<tr>';
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(prevYear) + '\',\'YY\');\">&#x25C0;</button></td>';
    _table += '<td colspan='+cols+'><span>&nbsp;</span></td>';
    _table += '<td><button class=\"calendar\" onclick=\"drawCalendar(\'' + DateToMySql(nextYear) + '\',\'YY\');\">&#x25B6;</button></td>';
    _table += '</tr>';
    var c = currentYear-7;
    for(var i = 0; i < rows; i++) {
      _table += '<tr><td><span class=\"calendar\">&nbsp;</span></td>';
      for(var j = 0; j < cols; j++, c++) {
        d.setYear(c);
        _table += '<td><button'+((c==currentYear)?' class=\"active calendar\"':' class=\"calendar\"')+' onclick=\"drawCalendar(\'' + DateToMySql(d) + '\',\'MM\');\">'+c+'</button></td>';
      }
      _table += '<td><span class=\"calendar\">&nbsp;</span></td></tr>';
    }
    _table += '</tbody></table>';
  }
  document.getElementById('CALENDAR').innerHTML = _table;
}
