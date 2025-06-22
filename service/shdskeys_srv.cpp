#include "shdskeys_srv.h"

/* ======================================================================
Run a command and get output
====================================================================== */
std::string GetOutputCommand(std::string command)
{
  std::string data;
  FILE * fstr;
  const int size = 256;
  char buffer[size];
  command.append(" 2>&1");
  fstr = popen(command.c_str(), "r");
  if (fstr) {
    while (!feof(fstr))
      if (fgets(buffer, size, fstr) != NULL) data.append(buffer);
    pclose(fstr);
  }
  return data;
}

/* ======================================================================
Check if file exists
====================================================================== */
boolean fexists(std::string filename)
{
  FILE * fstr;
  boolean result = 0;;
  filename.insert(0,"ls ");
  filename.append(" 2>&1");
  fstr = popen(filename.c_str(), "r");
  if (fstr == NULL) result = 1;
  pclose(fstr);
  return result;
}

/* ======================================================================
Get region/country list from system
====================================================================== */
void GetCountriesData(void)
{
  std::string values;
  if (!(Countries_Data.empty())) Countries_Data.erase(Countries_Data.begin(),Countries_Data.end());
  values = GetOutputCommand(COUNTRIES_LIST);
  if (!values.empty())
  {
    size_t len = values.length();
    std::string token = "";
    for(size_t i = 0; i < len; i++)
    {
      if ((!(std::string(1,values[i]).compare(DELIMITER))) && (i < len))
      {
        Countries_Data.push_back(token);
        token = "";
      }
      else
        token += values[i];
    }
    it_C = Countries_Data.begin();
  }
}

/* ======================================================================
Get wlan interface
====================================================================== */
boolean GetWlanInterface(void)
{
  wlan = GetOutputCommand(LIST_WLAN_INTERFACES);
  if (wlan.empty())
    return 1; // problem finding wlan interface
  wlan.erase(wlan.length()-1); // erase end of line
  return 0;
}

/* ======================================================================
Channels designated in the 2.4GHz frequency range (used on the Pi)
====================================================================== */
std::string Channel(std::string frequency)
{
  if (frequency == "2412") return "01";
  if (frequency == "2417") return "02";
  if (frequency == "2422") return "03";
  if (frequency == "2427") return "04";
  if (frequency == "2432") return "05";
  if (frequency == "2437") return "06";
  if (frequency == "2442") return "07";
  if (frequency == "2447") return "08";
  if (frequency == "2452") return "09";
  if (frequency == "2457") return "10";
  if (frequency == "2462") return "11";
  if (frequency == "2467") return "12";
  if (frequency == "2472") return "13";
  if (frequency == "2484") return "14";
  return "00";
}

/* ======================================================================
Get available wifi using wpa_cli
====================================================================== */
void GetWiFiData(void)
{
  if (!(WiFi_Data.empty())) WiFi_Data.erase(WiFi_Data.begin(),WiFi_Data.end());
  std::string values;
  values = GetOutputCommand("wpa_cli -i " + wlan + " scan_results | sed '1d' | tr '\t' '" + DELIMITER + "' | tr '\n' '" + DELIMITER + "'");
  if (!values.empty())
  {
    size_t len = values.length();
    std::string token = "";
    for(size_t i = 0; i < len; i++)
    {
      if ((!(std::string(1,values[i]).compare(DELIMITER))) && (i < len))
      {
        WiFi_Data.push_back(token);
        token = "";
      }
      else token += values[i];
    }
    it_W = WiFi_Data.begin();
  }
}

/* ======================================================================
Scan wlan for SSID's
====================================================================== */
void ScanWiFi(void)
{
  boolean wifi_error = 0;
  if (wlan == "") wifi_error = GetWlanInterface();
  if (!wifi_error)
  {
    boolean got_data = 0;
    do
    {
      std::string output = GetOutputCommand("wpa_cli -i " + wlan + " scan");
      if (output.compare("FAIL-BUSY\n"))
      {
        GetWiFiData();
        got_data = 1;
      }
    }
    while (!got_data);
  }
}

/* ======================================================================
Standard SQL error function
====================================================================== */
boolean finish_with_error(MYSQL *mysqlConn, const char * reason)
{
  fprintf(stderr, "Err @ %s - %s\n", reason, mysql_error(mysqlConn));
  display_error(reason);
  usleep(10000000); // sleep 10 seconds
  return 1;
}

/* ======================================================================
Set database local time
====================================================================== */
void set_sql_time(std::string time)
{
  statement = "SET GLOBAL time_zone = " + time + ";";
  if (mysql_query(mysqlConn,statement.c_str()))
    sql_error = finish_with_error(mysqlConn, statement.c_str());
}

/* ======================================================================
Get status for wireless and system country code
====================================================================== */
void GetStatusData(void)
{
  if (!(Status_Data.empty())) Status_Data.erase(Status_Data.begin(),Status_Data.end());
  boolean wifi_error = 0;
  if (wlan == "") wifi_error = GetWlanInterface();
  if (!wifi_error)
  {
    std::string values;
    sys_timezone = GetOutputCommand(GET_LOCALZONE);
    sys_timezone.erase(sys_timezone.length()-1); // erase end of line
    // set environment time zone (from system)
    char* tz = strdup(("TZ=" + sys_timezone).c_str());
    putenv(tz);
    free(tz);
    sys_localtime = GetOutputCommand(GET_LOCALTIME);
    sys_localtime.erase(sys_localtime.length()-1); // erase end of line
    sys_country_code = GetOutputCommand("cat /usr/share/zoneinfo/zone.tab | grep " + sys_timezone + " | cut -f 1");
    sys_country_code.erase(sys_country_code.length()-1); // erase end of line
    wlan_country_code = GetOutputCommand(GET_WIRELESS_COUNTRY_CODE);
    wlan_country_code.erase(wlan_country_code.length()-1); // erase end of line
    // finished setting up global variables
    // Writing data to vector
    values = "TZ: " + sys_timezone;
    Status_Data.push_back(values); // system timezone country
    values = "Local Time: GMT" + sys_localtime;
    Status_Data.push_back(values); // system local timezone
    if (sys_country_code.compare(wlan_country_code))
      values = "WARNING! " + wlan + " country code: " + wlan_country_code + " differs from system country code: " + sys_country_code;
    else
      values = "WiFi country code: " + wlan_country_code;
    Status_Data.push_back(values); // WiFi country code
    values = GetOutputCommand("wpa_cli -i " + wlan + " status | tr '\n' '" + DELIMITER + "'");
    if (!values.empty())
    {
      size_t len = values.length();
      std::string token = "";
      for(size_t i = 0; i < len; i++)
      {
        if ((!(std::string(1,values[i]).compare(DELIMITER))) && (i < len))
        {
          Status_Data.push_back(token);
          token = "";
        }
        else token += values[i];
      }
    }
    else
    {
      Status_Data.push_back("Could not communicate with wpa_supplicant");
    }
  }
  else
  {
    Status_Data.push_back("Error: wlan not found");
  }
  it_S = Status_Data.begin();
}

/* ======================================================================
Set system time zone and wireless country code
====================================================================== */
std::string do_wifi_country(std::string country)
{
  std:: string output, result = "";
  output = GetOutputCommand("wpa_cli -i " + wlan + " set country " + country);
  output = GetOutputCommand("iw reg set " + country + " 2> /dev/null");
  if (output.compare("") != 0)
  { // if something is returned from the last command
    result += "Reboot necessary to set wifi country to " + country + " : " + output + "! ";
    output = GetOutputCommand("hash rfkill 2> /dev/null");
    if ((fexists("/run/wifi-country-unset") != 0) && (output.compare("") != 0))
      output = GetOutputCommand("rfkill unblock wifi");
  }
  output = GetOutputCommand("wpa_cli -i " + wlan + " save_config > /dev/null 2>&1");
  return result;
}

/* ======================================================================
Set system time zone and wireless country code
====================================================================== */
void set_timezone(std::string timezone, std::string country)
{
  std::string output, result;
  result = "";
  output = GetOutputCommand("timedatectl set-timezone " + timezone);
  if (output.compare("") != 0) // if something is returned from the command
    result += "Error set-timezone " + timezone + " : " + output + "! ";
  output = GetOutputCommand("timedatectl set-ntp true");
  if (output.compare("") != 0) // if something is returned from the command
    result += "Error set-ntp true : " + output + "! ";
  result += do_wifi_country(country);
  module.SH1106_clearDisplay();   // clears the screen buffer
  if (result.find("Error") != std::string::npos) // Error screen
  {
    module.SH1106_printchar6(0,result.c_str(),1);
    module.SH1106_display();
    output = GetOutputCommand("reboot");
  }
  else // Success screen
  {
    module.SH1106_printchar6(1,"Success setting up system time zone and wireless country code!",1);
    module.SH1106_display();
  }
  // set environment time zone (from system)
  char* tz = strdup(("TZ=" + sys_timezone).c_str());
  putenv(tz);
  free(tz);
  // also restart sql service (to reset timezone)
  mysql_close(mysqlConn); // first close mysql connections
  output = GetOutputCommand(RESTART_SQL);

  usleep(10000000); // sleep 10 seconds

  sql_check_conn(); // check sql connection
  GetStatusData(); // Re-get status data
}

/* ======================================================================
Set system time zone and wireless country code
====================================================================== */
void set_wifi_connection(std::string ssid, std::string password)
{
  std::string result, output1, output2, id;
  if ((!(sys_country_code.compare(wlan_country_code))) && (!(wlan_country_code.compare(""))))
    result += do_wifi_country(sys_country_code); // if wlan country does not exist then set it to <sys_country_code>
  result += GetOutputCommand("#!/bin/sh \n wpa_cli -i " + wlan + " list_networks | tail -n +2 | cut -f -2 | grep -P \"\t" + ssid + "\" | cut -f1 | while read ID; do \n wpa_cli -i " + wlan + " remove_network \"$ID\" > /dev/null 2>&1 \n done \n ");
  id = GetOutputCommand("wpa_cli -i " + wlan + " add_network"); id.erase(id.length()-1);
  output1 = GetOutputCommand("wpa_cli -i " + wlan + " set_network " + id + " ssid \"\\\"" + ssid + "\\\"\" 2>&1");
  output1.erase(output1.length()-1);
  if (password.compare("") != 0)
    output2 = GetOutputCommand("wpa_cli -i " + wlan + " set_network " + id + " psk \"\\\"" + password + "\\\"\" 2>&1");
  else
    output2 = GetOutputCommand("wpa_cli -i " + wlan + " set_network " + id + " key_mgmt NONE 2>&1");
  output2.erase(output2.length()-1);
  if ((output1.compare("OK") != 0) || (output2.compare("OK") != 0))
  {
    output2 = GetOutputCommand("wpa_cli -i " + wlan + " remove_network " + id + " > /dev/null 2>&1");
    result += "Failed to set SSID or passphrase ";
  }
  else
  {
    output1 = GetOutputCommand("wpa_cli -i " + wlan + " enable_network " + id + " > /dev/null 2>&1");
    result += "Succes setting SSID and passphrase ";
  }
  output1 = GetOutputCommand("wpa_cli -i " + wlan + " save_config > /dev/null 2>&1");
  output1 = GetOutputCommand("wpa_cli -i " + wlan + " reconfigure > /dev/null 2>&1");
  module.SH1106_clearDisplay();   // clears the screen buffer
  module.SH1106_printchar6(0,result.c_str(),1);
  module.SH1106_display();
  usleep(10000000); // sleep 10 seconds
  GetStatusData(); // Re-get status data
}

/* ======================================================================
Get bit value from a byte given the bit position (0..7)
====================================================================== */
boolean getBit(uint8_t oct, uint8_t bit)
{
  boolean result = ((oct >> bit) & 0x01);
  return result;
}

/* ======================================================================
Get bit position from a byte with only one nibble (de-Brujin sequence)
int bits2position[] = { 8 , 1 , 2 , 4 , 7 , 3 , 6 , 5 };
====================================================================== */
int getBitPos(uint8_t oct)
{
  return bits2position[((oct * 0x17) >> 4) & 0x07];
}

/* ======================================================================
Get SQL connection settings from web server home folder
and check connection to SQL server
====================================================================== */
void sql_check_conn(void)
{
  sql_error = 0;
  /* Get data from config file from web server location
     to connect to the database */
  std::ifstream dbconfig;
  dbconfig.open("/var/www/html/config.php");
  if (!dbconfig)
  {
    fprintf(stderr, "config.php file not found\n");
    sql_error = 1;
  }
  if (!(sql_error))
  { /* Found config file. Let's read sql credentials */
    std::string linie;
    size_t found;
    std::string dbcred[4];
    int i = 0;
    while (std::getline(dbconfig,linie))
    {
      found = linie.find(",");
      if (found!=std::string::npos)
        dbcred[i++] = linie.substr(found+3,linie.length()-found-6);
    }
    mysqlConn = mysql_init(NULL);
    if (mysqlConn == NULL)
    {
      fprintf(stderr, "mysql_init() failed\n");
      sql_error = 1;
    }
    if (!(sql_error))
    { /* Check connection to database :  dbcred[0]:hostname  dbcred[1]:username  dbcred[2]:password  dbcred[3]:database  */
      if (mysql_real_connect(mysqlConn, dbcred[0].c_str(), dbcred[1].c_str(), dbcred[2].c_str(), dbcred[3].c_str(), 0, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
        sql_error = finish_with_error(mysqlConn, "real_connnect");
    }
  }
}

/* ======================================================================
Insert error message into database
====================================================================== */
void insert_sql_error(std::string message)
{
  std::string data = "";
  std::string id;
  if (!(sql_error))
  {
    statement = "SELECT `id`,`log_data` FROM `th_act_log` ORDER BY `id` DESC LIMIT 1;";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "insert_sql_error (get_last_message)");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) { id = sqlrow[0]; data = sqlrow[1]; }
      if (!(message.compare(data))) // If the last data message in table has the same error, then update only the timestamp
        statement = "UPDATE `th_act_log` SET `date_time`=NOW() WHERE `id` = " + id + ";";
      else // insert error
        statement = "INSERT INTO `th_act_log` (`user`, `log_data`) VALUES ('shdskeys_srv','" + message + "');";
      if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
        sql_error = finish_with_error(mysqlConn, "insert_sql_error (insert error)");
    }
  }
}

/* ======================================================================
Logging in database
====================================================================== */
void log_data(void)
{
  if (!(sql_error))
  {
    statement = "INSERT INTO `th_log` (`real_temp`, `mode`, `prog`, `th_temp`, `relay`, `tolerance`) VALUES (" + temp + ", " + std::to_string(mode) + ", " + std::to_string(prog->id) + ", " + ((temp_target_text=="OFF")?"-0.1":temp_target_text) + ", " + std::to_string(module.relay_status) + ", " + temp_tolerance + ");";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "log_data");
  }
}

/* ======================================================================
Get manual set target temperature from database
====================================================================== */
std::string get_tolerance(void)
{
  std::string data = "0.0";
  if (!(sql_error))
  {
    sqlresult = NULL;
    statement = "SELECT `val` FROM `th_vars` WHERE `var`='tolerance';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_tolerance");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) data = sqlrow[0];
      else insert_sql_error(":tolerance: variable not found in :th_vars: table");
      mysql_free_result(sqlresult);
    }
  }
  return data;
}

/* ======================================================================
Get active program from database (into a program class)
====================================================================== */
void get_active_program(void)
{
  if (!(sql_error))
  {
    sqlresult = NULL;
    statement = "SELECT `id`,`group` FROM `th_groups` WHERE `active`=1 LIMIT 1;";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_active_program");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) { prog->id = std::stoi(sqlrow[0]); prog->text = sqlrow[1]; }
      else insert_sql_error("No active program found in :th_groups: table");
      mysql_free_result(sqlresult);
    }
  }
}

/* ======================================================================
Get target temperature for current week day and hour from database
====================================================================== */
std::string get_program_temp(int c_wday, int c_hour)
{
  std::string data = "00.000";
  if (!(sql_error))
  {
    sqlresult = NULL;
    std::string week_day = std::to_string(c_wday);
    std::string day_hour = std::to_string(c_hour);
    statement = "SELECT `temp` FROM `th_temps` `t` JOIN `th_groups` `g` ON `t`.`group`=`g`.`id` WHERE `g`.`active`=1 AND `t`.`wday`=" + week_day + " AND `t`.`hour`=" + day_hour + " LIMIT 1; ";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_program_temp");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) data = sqlrow[0];
      else insert_sql_error(":temp: value not found in :th_groups: table");
      mysql_free_result(sqlresult);
    }
  }
  return data;
}

/* ======================================================================
Get manual set target temperature from database
====================================================================== */
std::string get_manual_temp(void)
{
  std::string data = "00.000";
  if (!(sql_error))
  {
    sqlresult = NULL;
    statement = "SELECT `val` FROM `th_vars` WHERE `var`='manual_temp';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_manual_temp");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) data = sqlrow[0];
      else insert_sql_error(":manual_temp: variable not found in :th_vars: table");
      mysql_free_result(sqlresult);
    }
  }
  return data;
}

/* ======================================================================
Set manual target temperature in database (increase or decrease)
====================================================================== */
void alter_manual_temp(int direction)
{
  if (!(sql_error))
  {
    float old_temp = std::stod(temp_target_text);
    temp_target_text = std::to_string(!(direction) ? (old_temp - 0.250) : (old_temp + 0.250)).substr(0,6);
    statement = "UPDATE `th_vars` SET `val` = " + temp_target_text + " WHERE `var`='manual_temp';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "alter_manual_temp");
    mode_check();
  }
}

/* ======================================================================
Set tolerance temperature in database (increase or decrease)
====================================================================== */
void alter_tolerance(int direction)
{
  if (!(sql_error))
  {
    float old_tolerance = std::stod(temp_tolerance);
    temp_tolerance = std::to_string(!(direction) ? (old_tolerance - 0.1) : (old_tolerance + 0.1)).substr(0,3);
    if (temp_tolerance == "0.0") temp_tolerance = "0.1"; // limit low
    if (temp_tolerance == "5.0") temp_tolerance = "4.9"; // limit high
    statement = "UPDATE `th_vars` SET `val` = " + temp_tolerance + " WHERE `var`='tolerance';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "alter_tolerance");
    mode_check();
  }
}

/* ======================================================================
Set real temperature value in database
====================================================================== */
void set_real_temp(std::string new_value)
{
  if (!(sql_error))
  {
    statement = "UPDATE `th_vars` SET `val` = '" + new_value + "' WHERE `var`='real_temp';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "set_real_temp");
    log_data();
  }
}

/* ======================================================================
Set relay status value in database
====================================================================== */
void set_relay_status(std::string new_value)
{
  if (!(sql_error))
  {
    statement = "UPDATE `th_vars` SET `val` = '" + new_value + "' WHERE `var`='relay_status';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "set_relay_status");
  }
}

/* ======================================================================
If mode changes then a cascade check is triggered
====================================================================== */
void mode_check(void)
{
  if ( 0 == mode )
  { /* OFF */
    prog->id = 0;
    prog->text = "OFF";
    temp_tolerance = get_tolerance();
    temp_target_text = "OFF";
    if (module.relay_status != 0)
    {
      module.Relay_OFF();
      set_relay_status("OFF");
    }
    if (!(menu)) select_1 = BIT3;
  }
  else if ( 1 == mode )
  { /* PROGRAM */
    temp_tolerance = get_tolerance();
    get_active_program();
    if (tstr->tm_wday == 0)
      temp_target_text = get_program_temp(7,tstr->tm_hour);
    else
      temp_target_text = get_program_temp(tstr->tm_wday,tstr->tm_hour);
    if (( module.temperature > (strtof(temp_target_text.c_str(),0) + strtof(temp_tolerance.c_str(),0)) ))
    {
      module.Relay_OFF();
      set_relay_status("OFF");
    }
    else if (( module.temperature < (strtof(temp_target_text.c_str(),0)) ))
    {
      module.Relay_ON();
      set_relay_status("ON");
    }
    if (!(menu)) select_1 = BIT1;
  }
  else if ( 2 == mode )
  { /* MANUAL */
    prog->id = 0;
    prog->text = "MANUAL";
    temp_tolerance = get_tolerance();
    temp_target_text = get_manual_temp();
    if (( module.temperature > (strtof(temp_target_text.c_str(),0) + strtof(temp_tolerance.c_str(),0)) ))
    {
      module.Relay_OFF();
      set_relay_status("OFF");
    }
    else if (( module.temperature < (strtof(temp_target_text.c_str(),0)) ))
    {
      module.Relay_ON();
      set_relay_status("ON");
    }
    if (!(menu)) select_1 = BIT2;
  }
}

/* ======================================================================
Get working mode from database
====================================================================== */
void get_mode(void)
{
  if (!(sql_error))
  {
    sqlresult = NULL;
    statement = "SELECT `val` FROM `th_vars` WHERE `var`='mode';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_mode");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      if ( sqlrow != NULL ) mode = std::stoi(sqlrow[0]);
      else insert_sql_error(":mode: variable not found in :th_vars: table");
      mysql_free_result(sqlresult);
      mode_check();
    }
  }
}

/* ======================================================================
Rotate mode from database forward or backward
====================================================================== */
void rotate_mode(int direction)
{
  if (!(sql_error))
  {
    int new_mode = !(direction) ? (0 == mode ? 2 : (mode-1)) : (2 == mode ? 0 : (mode+1));
    statement = "UPDATE `th_vars` SET `val` = " + std::to_string(new_mode) + " WHERE `var`='mode';";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "rotate_mode");
    if (!(sql_error))
    {
      mode = new_mode;
      mode_check();
    }
  }
}

/* ======================================================================
Rotate active program from database forward or backward (using id)
====================================================================== */
void rotate_program(int direction)
{
  if (!(sql_error))
  {
    sqlresult = NULL;
    if (direction)
      statement = "SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` > " + std::to_string(prog->id) + " ORDER BY `id` ASC LIMIT 1),(SELECT `id` FROM `th_groups` ORDER BY `id` ASC LIMIT 1));";
    else
      statement = "SELECT IFNULL((SELECT `id` FROM `th_groups` WHERE `id` < " + std::to_string(prog->id) + " ORDER BY `id` DESC LIMIT 1),(SELECT `id` FROM `th_groups` ORDER BY `id` DESC LIMIT 1));";
    if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
      sql_error = finish_with_error(mysqlConn, "get_program_id_for_rotation");
    if (!(sql_error))
    {
      sqlresult = mysql_store_result(mysqlConn);
      sqlrow = mysql_fetch_row(sqlresult);
      int new_id;
      if ( sqlrow != NULL ) new_id = std::stoi(sqlrow[0]);
      else insert_sql_error("No active program id found in :th_groups: table for rotation");
      mysql_free_result(sqlresult);
      if (new_id != prog->id) {
        statement = "UPDATE `th_groups` SET `active` = CASE WHEN `id` = " + std::to_string(new_id) + " THEN 1 WHEN `id` <> " + std::to_string(new_id) + " THEN 0 END;";
        if (mysql_real_query(mysqlConn,statement.c_str(),statement.length()))
          sql_error = finish_with_error(mysqlConn, ("rotate_program_set_new_id_active " + statement).c_str());
      }
    }
    if (!(sql_error)) mode_check();
  }
}

/* ======================================================================
Get current system time and write it to the database if it changed
====================================================================== */
void get_temp(void)
{
  std::string temp_old = temp; // save current value
  temp = (std::to_string(module.temperature)).substr(0,6);
  // if new value differes then we update the database
  if (temp_old.compare(temp)) set_real_temp(temp);
}

/* ======================================================================
Init timer (witch resets every second)
====================================================================== */
void init_timer(void)
{
  clock_gettime(CLOCK_REALTIME, &gettime);
  start_timer = gettime.tv_sec;
}

/* ======================================================================
Check timer and reset if needed
====================================================================== */
boolean check_timer(void)
{
  clock_gettime(CLOCK_REALTIME, &gettime);
  timer_difference = gettime.tv_sec - start_timer;
  if (timer_difference != 0) { //If a second passed
    init_timer(); // Reinitialize timer
    return 1;
  }
  return 0;
}

/* ======================================================================
Get current system time
====================================================================== */
void get_time(void)
{
  time_t rawtime = time(0);
  tstr = localtime(&rawtime);
  strftime(buf_date, sizeof(buf_date), "%d-%m-%Y", tstr);
  strftime(buf_time, sizeof(buf_time), "%X", tstr);
  mktime(tstr);
}

/* ======================================================================
Display error message on OLED screen
====================================================================== */
void display_error(const char * msg)
{
  module.SH1106_clearDisplay();   // clears the screen buffer
  if (!(msg))
  {
    module.SH1106_printchar6(3,"EROARE DE CONEXIUNE");
    module.SH1106_printchar6(4,"LA BAZA DE DATE");
  }
  else
    module.SH1106_printchar6(3,msg);
  module.SH1106_display();
}

/* ======================================================================
Display keyboard
====================================================================== */
void display_keyboard(void)
{
  module.SH1106_clearDisplay();   // clears the screen buffer
  int ident = 3;
  std::string k_input_text = k_input + " ";
  uint8_t pos_s = pos_k;
  if ((k_input_text.length()-pos_s) > 21)
  {
    k_input_text = k_input_text.substr(pos_s, 21);
    pos_s = 0;
  }
  else
  {
    if (k_input_text.length() > 21)
    {
      pos_s = pos_s - (k_input_text.length() - 21);
      k_input_text = k_input_text.substr(k_input_text.length()-21, 21);
    }
  }
  if (pos_s != 0)
    module.SH1106_printchar6(0,1,(k_input_text.substr(0,pos_s)).c_str());
  module.SH1106_printchar6(0,6*pos_s+1,(std::string(1,k_input_text[pos_s])).c_str(),6,1);
  if (pos_s != k_input_text.length())
    module.SH1106_printchar6(0,6*pos_s+7,(k_input_text.substr(pos_s + 1,k_input_text.length()-pos_s)).c_str());
  module.SH1106_printchar6(1,0," ---------------------",1,1);
  char q;
  int iter;
  for(int y = 0; y < 6; y++)
  {
    for(int x = 0; x < 17; x++)
    {
      iter = (y * 17) + x;
      q = keyboard[iter];
      switch (q)
      {
        case '\u0008':
          module.SH1106_printcustom(y + 2, x * 7 + ident, bk_char, 7, (iter == select_k ? 1 : 0));
          break;
        case '\u0011':
          module.SH1106_printcustom(y + 2, x * 7 + ident, up_char, 7, (iter == select_k ? 1 : 0));
          break;
        case '\u0012':
          module.SH1106_printcustom(y + 2, x * 7 + ident, down_char, 7, (iter == select_k ? 1 : 0));
          break;
        case '\u0013':
          module.SH1106_printcustom(y + 2, x * 7 + ident, right_char, 7, (iter == select_k ? 1 : 0));
          break;
        case '\u0014':
          module.SH1106_printcustom(y + 2, x * 7 + ident, left_char, 7, (iter == select_k ? 1 : 0));
          break;
        case '\u000D':
          module.SH1106_printcustom(y + 2, x * 7 + ident, ok_char, 7, (iter == select_k ? 1 : 0));
          break;
        case ' ':
          module.SH1106_printchar6(y + 2, x * 7 + ident," ", 7, (' ' == keyboard[select_k] ? 1 : 0));
          break;
        default:
          module.SH1106_printchar6(y + 2, x * 7 + ident,(" " + std::string(1,q)).c_str(),1,(iter == select_k ? 1 : 0));
      }
    }
  }
  module.SH1106_display();
}

/* ======================================================================
Display main screen on OLED
====================================================================== */
void display_screen(void)
{
  module.SH1106_clearDisplay();   // clears the screen buffer
  module.SH1106_printchar6(0,55,buf_date);
  module.SH1106_printspecial(1,44,buf_time);
  module.SH1106_printchar6(0,0," PROG ",9,getBit(select_1,0));
  module.SH1106_printchar6(1,0," MANUAL ",3,getBit(select_1,1));
  module.SH1106_printchar6(2,0,"  OFF  ",6,getBit(select_1,2));
  module.SH1106_printchar6(4,prog->text.c_str());
  module.SH1106_printchar6(5,0,"t°C");
  module.SH1106_printchar6(6,0,temp_target_text.c_str());
  module.SH1106_printchar6(7,0,temp_tolerance.c_str());
  module.SH1106_printspecial(6,48,("!"+temp).c_str());
  module.SH1106_display();
}

/* ======================================================================
Display menu on OLED
====================================================================== */
void display_menu(void)
{
  module.SH1106_clearDisplay();   // clears the screen buffer
  module.SH1106_printchar6(0,0," STATUS ",3,getBit(select_2,0));
  module.SH1106_printchar6(1,0," ------ ",3,getBit(select_2,0));
  module.SH1106_printchar6(0,42," TIMEZONE ",3,getBit(select_2,1));
  module.SH1106_printchar6(1,42," -------- ",3,getBit(select_2,1));
  module.SH1106_printchar6(0,96," WIFI ",3,getBit(select_2,2));
  module.SH1106_printchar6(1,96," ---- ",3,getBit(select_2,2));
  int rows = 0;
  if (getBit(select_2,0)) // STATUS
  {
    if (!Status_Data.empty())
    {
      do
      {
        if (getBit(select_3,rows))
          module.SH1106_printchar6(rows+2,0,(*(it_S+rows*DC_S)).c_str(),2,1,1);
        else
          module.SH1106_printchar6(rows+2,0,(*(it_S+rows*DC_S)).c_str(),2,0);
        rows++;
      }
      while ((rows < 6) && (std::distance(it_S+rows*DC_S,Status_Data.end())>0));
    }
  }
  if (getBit(select_2,1)) // TIMEZONE
  {
    if (!Countries_Data.empty())
    {
      do
      {
        if (getBit(select_3,rows))
          module.SH1106_printchar6(rows+2,0,(*(it_C+rows*DC_C)+" "+*(it_C+rows*DC_C+1)).c_str(),2,1,1);
        else
          module.SH1106_printchar6(rows+2,0,(*(it_C+rows*DC_C)+" "+*(it_C+rows*DC_C+1)).c_str(),2,0);
        rows++;
      }
      while ((rows < 6) && (std::distance(it_C+rows*DC_C,Countries_Data.end())>0));
    }
  }
  if (getBit(select_2,2)) // WIFI
  {
    if (!WiFi_Data.empty())
    {
      do
      {
        if (getBit(select_3,rows))
          module.SH1106_printchar6(rows+2,0,(*(it_W+rows*DC_W+2)+" | CH"+Channel(*(it_W+rows*DC_W+1))+" | "+*(it_W+rows*DC_W+4)).c_str(),2,1,1);
        else
          module.SH1106_printchar6(rows+2,0,(*(it_W+rows*DC_W+2)+" | CH"+Channel(*(it_W+rows*DC_W+1))+" | "+*(it_W+rows*DC_W+4)).c_str(),2,0);
        rows++;
      }
      while ((rows < 6) && (std::distance(it_W+rows*DC_W,WiFi_Data.end())>0));
    }
  }
  module.SH1106_display();
}

/* ======================================================================
Control: move_page_up
====================================================================== */
void move_page_up(std::vector<std::string> * Vector, std::vector<std::string>::iterator * It, int dc)
{
  if (select_3 != BIT1) // scroll is not at the top position
  { // get to top position
    select_3 = BIT1;
  }
  else
  { // scroll position is now at the top position
    if (std::distance((*Vector).begin(),*It) >= (dc * 6))
      // if It can be decreased by at least 6 positions (dc * 6)
      *It -= (dc * 6);
    else
      // if It can't be decreased then we go at the begining
      *It = (*Vector).begin();
  }
}

/* ======================================================================
Control: move_page_down
====================================================================== */
void move_page_down(std::vector<std::string> * Vector, std::vector<std::string>::iterator * It, int dc)
{
  if (select_3 != BIT6) // scroll is not at the bottom position
  { // get to bottom position
    int max = std::distance((*Vector).begin(),(*Vector).end());
    if (max < (dc * 6))
      select_3 = bit[max/dc];
    else
      select_3 = BIT6;
  }
  else // scroll position is now at the bottom position
    if (std::distance(*It,(*Vector).end()) > (dc * 12))
      // if It can be increased by at least 6 positions (dc * 6)
      *It += (dc * 6);
    else
      // if It can't be decreased then we go at the end page
      *It = (*Vector).end() - (dc * 6);
}

/* ======================================================================
Control: move_pos_up
====================================================================== */
void move_pos_up(std::vector<std::string> * Vector, std::vector<std::string>::iterator * It, int dc)
{
  if (select_3 != BIT1) // scroll is not at the top position
    select_3 = select_3 >> 1; // move up one position
  else // scroll position is now at the top position
    if (std::distance((*Vector).begin(),*It) >= (dc * 1))
      // decrease It by 1 position (dc records = 1 position)
      *It -= (dc * 1);
    // else we're already at the begining
}

/* ======================================================================
Control: move_pos_down
====================================================================== */
void move_pos_down(std::vector<std::string> * Vector, std::vector<std::string>::iterator * It, int dc)
{
  int max = std::distance((*Vector).begin(),(*Vector).end());
  if ((select_3 != BIT6) && (getBitPos(select_3) < (max/dc))) // scroll is not at the bottom position
    select_3 = select_3 << 1; // move down one position
  else // scroll position is now at the bottom position
    if (std::distance(*It,(*Vector).end()) > (dc * 6))
      // increase it_C by 1 position (DC_W records = 1 position)
      *It += (dc * 1);
    // else we're already at the end
}

/* ======================================================================
Control: read mode/program/temperature and turn relay on or off
====================================================================== */
void control_module(void)
{
  check_button = new_button; // retain old new_button value
  new_button = module.KEYS_check(); // get new_button current value
  // buttons checker (check if TIMEOUT is reached below)
  button_pushed += (new_button != check_button);
  get_time();
  if (check_timer()) { // If one second passed, read all data for display
    // suspend temperature reading thread while buttons are pushed
    if (button_pushed > 0)
    {
      if (!(suspend)) suspend = 1;
      button_pushed = 0; // reset buttons checker
      button_timer = 0;  // reset buttons timer
    }
    else
    {
      button_timer += 1; // add one second to buttons timer
      if (button_timer >= TIMEOUT)
      {
        if (suspend) suspend = 0;
        button_timer = TIMEOUT; // let's not overflow
        get_temp();
        get_mode();
      }
    }
  }
  if (menu == 0)
  { // ---------------------------------------------- Main screen section
    if ( (new_button != check_button) || (new_button == 6) )
    // compare old value of new_button with current value (debounce)
    { // ---------------------------------- Main screen buttons functions
      switch (check_button)
      {
        case 1 /*  left  */: {
          if ( 0 == mode ) {
            alter_tolerance(0);
          } /* decrease tolerance */
          if ( 1 == mode ) {
            rotate_program(0);
          } /* rotate programs backward */
          if ( 2 == mode ) {
            alter_manual_temp(0);
          } /* decrease manual temperature */
          break;
        }
        case 2 /* right  */: {
          if ( 0 == mode ) {
            alter_tolerance(1);
          } /* increase tolerance */
          if ( 1 == mode ) {
            rotate_program(1);
          } /* rotate programs forward */
          if ( 2 == mode ) {
            alter_manual_temp(1);
          } /* increase manual temperature */
          break;
        }
        case 3 /*   up   */: {
          rotate_mode(0);
          break;
        } /* change mode backward */
        case 4 /*  down  */: {
          rotate_mode(1);
          break;
        } /* change mode forward */
        case 5 /*  menu  */: {
          GetStatusData(); // first re-get status
          ScanWiFi(); // and let's scan for wireless networks
          menu = 1; // go to display menu
          if (select_2 == BIT0)
            select_2 = BIT1; // select first menu
          break;
        }
        case 6 /* on/off */: {
          count += unit;
          if ( count > 4000000 ) turn_off_check = 1;
        break;
        }
        default /* no button */: {
          if ( count ) count = 0;
          break;
        }
      }
    } // --------------------------- End of main screen buttons functions
    if (!(sql_error)) display_screen();
    else
    {
      display_error();
      // recheck sql connection
      sql_check_conn();
    }
  } // --------------------------------------- End of main screen section
  else if (menu == 1)
  { // ---------------------------------------------- Menu screen section
    if ( (new_button != check_button) )
    // compare old value of new_button with current value (debounce)
    {  // --------------------------------- Menu screen buttons functions
      switch (check_button) {
        case 1 /*  left  */:
        {
          if (select_3 == BIT0) // no top menu selected
          {
            if (select_2 != BIT1)
              // change top menus left
              select_2 = select_2 >> 1;
          }
          else
          { // check witch menu is selected
            if (select_2 == BIT1) // menu STATUS
              move_page_up(&Status_Data,&it_S,DC_S);
            if (select_2 == BIT2) // menu TIMEZONE
              move_page_up(&Countries_Data,&it_C,DC_C);
            if (select_2 == BIT3) // menu WIFI
              move_page_up(&WiFi_Data,&it_W,DC_W);
          }
          break;
        }
        case 2 /* right  */:
        {
          if (select_3 == BIT0) // no top menu selected
          {
            if (select_2 != BIT3)
              // change top menus left
              select_2 = select_2 << 1;
          }
          else
          { // check witch menu is selected
            if (select_2 == BIT1) // menu STATUS
              move_page_down(&Status_Data,&it_S,DC_S);
            if (select_2 == BIT2) // menu TIMEZONE
              move_page_down(&Countries_Data,&it_C,DC_C);
            if (select_2 == BIT3) // menu WIFI
              move_page_down(&WiFi_Data,&it_W,DC_W);
          }
          break;
        }
        case 3 /*   up   */:
        {
          if (select_3 != BIT0) // there is a menu selected
          { // check witch menu is selected
            if (select_2 == BIT1) // menu STATUS
              move_pos_up(&Status_Data,&it_S,DC_S);
            if (select_2 == BIT2) // menu TIMEZONE
              move_pos_up(&Countries_Data,&it_C,DC_C);
            if (select_2 == BIT3) // menu WIFI
              move_pos_up(&WiFi_Data,&it_W,DC_W);
          }
          break;
        } /* change mode backward */
        case 4 /*  down  */:
        {
          if (select_3 != BIT0) // there is a menu selected
          { // check witch menu is selected
            if (select_2 == BIT1) // menu STATUS
              move_pos_down(&Status_Data,&it_S,DC_S);
            if (select_2 == BIT2) // menu TIMEZONE
              move_pos_down(&Countries_Data,&it_C,DC_C);
            if (select_2 == BIT3) // menu WIFI
              move_pos_down(&WiFi_Data,&it_W,DC_W);
          }
          break;
        } /* change mode forward */
        case 5 /*  menu  */:
        {
          if (select_3 == BIT0)  // not inside one of the menus
          {
            select_3 = BIT1; // enter current menu
            if (select_2 == BIT2) // TIMEZONE selected
            {
              if (sys_timezone.compare(*(it_C+1)) != 0)
              {
                it_C = Countries_Data.begin();
                do
                {
                  it_C++;
                }
                while ((sys_timezone.compare(*(it_C+1)) != 0) && (std::distance(it_C,Countries_Data.end()) > (DC_C * 6)));
              }
            }
            if ((select_2 == BIT3) && (WiFi_Data.empty())) // WIFI selected
              select_3 = BIT0; // Exit menu if WiFi Access Points Found
          }
          else
          {
            if (select_2 == BIT2) // menu TIMEZONE
              set_timezone(*(it_C + getBitPos(select_3) * DC_C - 1),*(it_C + getBitPos(select_3) * DC_C - 2));
            if (select_2 == BIT3) // menu WIFI
              menu = 2; // go to display keyboard
          }
          break;
        }
        case 6 /* on/off = back */:
        {
          if (select_3 == BIT0)
          {
            select_2 = BIT0; // reset menu selection
            suspend = 0; // resume temperature readings
            menu = 0; // go to main screen
          }
          else select_3 = BIT0; // no menu selected
          break;
        }
        default /* no button */: { }
      }
    } // --------------------------- End of menu screen buttons functions
    display_menu();
  } // --------------------------------------- End of menu screen section
  else if (menu == 2)
  { // ------------------------------------------ Keyboard screen section
    if ( (new_button != check_button) )
    // compare old value of new_button with current value (debounce)
    { // ------------------------------- Keyboard screen button functions
      switch (check_button) {
        case 1 /*  left  */:
        {
          if ((select_k % 17) != 0)
          {
            if (keyboard[select_k-1] == keyboard[select_k])
              select_k--; // condition for space character
            select_k--;
          }
          break;
        }
        case 2 /* right  */:
        {
          if ((select_k % 17) != 16)
          {
            if (keyboard[select_k+1] == keyboard[select_k])
              select_k++; // condition for space character
            select_k++;
          }
          break;
        }
        case 3 /*   up   */:
        {
          if (select_k >= 17)
            select_k -= 17;
          break;
        }
        case 4 /*  down  */:
        {
          if (select_k <= 84)
            select_k += 17;
          break;
        }
        case 5 /*  menu  */:
        { // Keyboard input string
          switch (keyboard[select_k])
          {
            case '\u0008':
            {
              if (pos_k > 0)
              {
                k_input.erase(pos_k-1,1);
                pos_k--;
              }
              break;
            }
            case '\u0011':
            { // up character
              pos_k = 0;
              break;
            }
            case '\u0012':
            { // down character
              pos_k = k_input.length();
              break;
            }
            case '\u0013':
            { // right character
              if (pos_k < k_input.length()) pos_k++;
              break;
            }
            case '\u0014':
            { // left character
              if (pos_k > 0) pos_k--;
              break;
            }
            case '\u000D':
            { // accept string
              set_wifi_connection(*(it_W + getBitPos(select_3) * DC_W - 1),k_input);
              k_input = ""; pos_k = 0; // reset input string
              menu=1;
              break;
            }
            default:
            {
              if (k_input.length() < 255) // limit to 254 characters
              {
                if (pos_k == k_input.length())
                  k_input += keyboard[select_k];
                else
                  k_input.insert(pos_k, std::string(1,keyboard[select_k]));
                pos_k++;
              }
            }
          }
          break;
        }
        case 6 /* on/off = back */:
        {
          k_input = ""; pos_k = 0; // reset input string
          menu = 1; // go back to menu screen
          break;
        }
        default /* no button */: { }
      }
    } // ----------------------- End of keyboard screen buttons functions
    display_keyboard();
  } // ----------------------------------- End of keyboard screen section
}

/* ======================================================================
Function used for separate thread
====================================================================== */
void static_read_temperature(SH1106_DS18B20_KEYS * instance)
{
  int seconds;
  while (!turn_off_check)
  {
    if (!(suspend)) instance->Read_Temperature();
    //wait 10s before next read initialization
    seconds = 10 * 1000;
    while ((!turn_off_check) && (seconds > 0))
    {
      usleep(1000);
      seconds--;
    }
  }
}

/* ======================================================================
Function: main
====================================================================== */
int main(void)
{
  // Init module
  if ( !module.module_init() )
  {
    exit(1);
  }
  module.start();

  // check sql connection
  sql_check_conn();

  // Preinit timezone
  GetCountriesData();
  // Preinit status
  GetStatusData();

  // start background thread to read temperature
  std::thread t1(static_read_temperature,&module);
  // init done

  module.SH1106_clearDisplay(); // clears the screen buffer
  module.SH1106_display();   	// display it (clear display)

  init_timer(); // timer initialize

  while (!turn_off_check)
  {
    usleep(unit);
    control_module();
  }

  // close mysql connections
  mysql_close(mysqlConn);
  // rejoin background temperature read thread read
  t1.join();
  // shutdown module
  module.close();
}
