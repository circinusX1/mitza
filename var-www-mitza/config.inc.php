<?php
/* database or textfile */

$COUNT_CFG["use_db"]     = false;

/* config */

$COUNT_CFG["block_time"] = 3600; /* sec */
$COUNT_CFG["offset"]     = 1;

/* textfile settings */

$COUNT_CFG["logfile"] = "./ip8888.txt";
$COUNT_CFG["counter"] = "./total_visits8888.txt";
$COUNT_CFG["daylog"]  = "./daily8888.txt";

/* database settings */

$COUNT_DB["dbName"] = "counter";
$COUNT_DB["host"]   = "localhost";
$COUNT_DB["user"]   = "muser";
$COUNT_DB["pass"]   = "mpass";

$COUNT_TBL["visitors"] = "count_visitors";
$COUNT_TBL["daily"]    = "count_daily";
$COUNT_TBL["total"]    = "count_total";

?>
