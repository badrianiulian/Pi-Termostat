<?php
function random_str($length,$type = "pass") {
  switch ($type) {
  case "file":
    $keyspace = '!#$-+=[]{}0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    break;
  case "key":
    $keyspace = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
    break;
  case "iv":
    $keyspace = '0123456789';
    break;
  default:
    $keyspace = '!@#$%^()><-+=[]{}|~0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
  }
  $str = '';
  for ($i = 0; $i < $length; ++$i) {
    do {
      $rand_byte = openssl_random_pseudo_bytes(1);
      $pos = strpos($keyspace, $rand_byte);
    } while ($pos === false);
    $str .= $rand_byte;
  }
  return $str;
}
?>
