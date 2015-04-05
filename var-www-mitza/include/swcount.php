<?php
/**
# Copyright (C) 2012-2014 Chincisan Octavian-Marius(mariuschincisan@gmail.com) - getic.net - N/A
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
*/

require_once("@config.php");
require_once("smap.php");


function coutwords($linkid)
{
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));


    $qry="SELECT words.wordid,dict.wrd,weight,count FROM words ";
    $qry.="JOIN dict ON (words.wordid=dict.wordid)";
    $qry.="WHERE linkid={$linkid} ORDER BY count DESC ";
    $result = mysqli_query($con,$qry) or die(mysqli_error($con));

    echo "<table border='0' cellpadding='1' width='80%' align='center'>".
        "<tr bgcolor='#EE7'><th colspan='3'>Page Words statistics</th></tr>".
        "<tr><th bgcolor='#EE8'>Word</th><th bgcolor='#EE8'>weight</th><th bgcolor='#EE8'>word count</th></tr>";
    while($r=mysqli_fetch_row($result))
    {
        echo "<tr><td bgcolor='#EEB'>{$r[1]}</td><td bgcolor='#EEB'>{$r[2]}</td><td bgcolor='#EEB'>{$r[3]}</td></tr>";

    }
    echo "</table>";
    mysqli_free_result($result);




    mysqli_close($con);
}

function mainex()
{
//
    if(isset($_GET['sm']))
        sitemap($_GET['sm']);

}


if(isset($_GET['sitemap']))
    sitemap($_GET['sitemap']);


?>

