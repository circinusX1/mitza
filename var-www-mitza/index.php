
<HTML>
<TITLE>
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
    $CAT=0;
    if($_SERVER['HTTP_HOST']=="staging.mine.nu")
        $CAT=6;
    if($CAT==0)
        echo "MITZA SEARCH ENGINE";
    else
        echo "Home Improvement, Toronto Search Engine. &copy EnjoyDecor 2013";

?>

</TITLE>


<BODY bgcolor='#EEA'>

<link rel="stylesheet" type="text/css" href="include/stil.css">
<link rel="stylesheet" type="text/css" href="http://fonts.googleapis.com/css?family=Philosopher&subset=latin,cyrillic">
<script type="text/javascript" src="include/jquery-1.2.2.pack.js"></script>
<script type="text/javascript" src="include/ajaxtooltip.js">
    /***********************************************
    * Ajax Tooltip script- by JavaScript Kit (www.javascriptkit.com)
    * This notice must stay intact for usage
    * Visit JavaScript Kit at http://www.javascriptkit.com/ for this script and 100s more
    ***********************************************/
</script>

<div align='center'>
</div>
<div align='center'>
    <table cellpadding="5" cellspacing="1   " >
    <tr>
        <td align="center">
            <form action="index.php" method="get">
            <table border='0'><tr><td colspan='2'>

<?php
        $ss="";
        if(isset($_GET["w"]))
            $ss=$_GET["w"];
        if($CAT==0){

           echo "Mitza:<input type='text' name='w' id='w' size='30' value='{$ss}' /></td><td>";
           echo "<input type='submit' value='Search...'></td><td valign='center'><img src='include/logo.png' alt='logo'/>";
        }
        else{
            echo "Toronto Search:<input type='text' name='w' id='w' size='30' value='{$ss}' /></td><td>";
            echo "<input type='submit' value='Find...'></td><td valign='center'>&nbsp";
        }
?>
        </td>
    </tr>
    </table>

<?php
require_once("include/@config.php");
require_once("include/ex.php");
require_once("include/ads.php");
require_once("include/swcount.php");

function main()
{
    global $_PAGE;
    $s=isset($_GET["s"]) ? $_GET["s"] : 0; // web site only
    $k=isset($_GET["k"]) ? $_GET["k"] : 0; // _catid
    $n=isset($_GET["n"]) ? $_GET["n"] : $_PAGE; // per page, n-count
    $f=isset($_GET["f"]) ? $_GET["f"] : 0; // from
    $ww=isset($_GET["w"]) ? $_GET["w"] : ""; //word

    $fwght=isset($_GET["g"]) ? $_GET["g"] : 0; //max weight
    $np = $f;
    $tp = $f;
    $w = config_prep($ww);
    echo ("<hr>");
    echo "<table width='960px'><tr><td width='840px'>";
    if(isset($_GET["sitemap"]) )
    {
        sm($_GET["sitemap"]);
    }
    if(isset($_GET["sitasets"]) )
    {
        sa($_GET["sitasets"],$_GET['sid']);
    }
    if(isset($_GET["wcount"]) )
    {
        coutwords($_GET["wcount"]);
    }

    else if(strlen($w))
    {
        if(substr($w,0,4)=="http")
            dbcontent($w);
        if($w=="xxx")
            sites();
        else
            $np=ex($w, $f, $n, $k, $s,$fwght);
    }
    else
    {
        ads();
    }

    if($np != $f)
    {
        if($np>$n)
        {
            $x=$f-$n;
            echo ("<a href='index.php?w={$ww}&f={$x}&n={$n}&s=$s&g=$fwght'><<-Prev</a> MITZA-1.0.0.1 <font color='red'>Mitzzzzzzzzzz</font><font color='blue'>z</font> <font color='green'>z </font> <font color='blue'>z</font> <font color='red'> i </font> <font color='yellow'>n </font> <font color='blue'>g</font>...");
        }
        else
        {
            echo ("X<-Prev  MITZA-1.0.0.1 <font color='red'>Mitzzzzzzzzzz</font><font color='blue'>z</font> <font color='green'>z </font> <font color='blue'>z</font> <font color='red'> i </font> <font color='yellow'>n </font> <font color='blue'>g</font>...");
        }
        if($tp+$_PAGE==$np)
            echo ("<a href='index.php?w={$ww}&f={$np}&n={$n}&s=$s&g=$fwght'>Next->></a> ");
        else
            echo ("Next->X");
    }
    global $CAT;

    echo "</td><td valign='top' class='fancy'><div align='center'>";
    if($CAT==0)
        echo "<a href='donate.html'>support</div><div><img src='include/logo.png'></a>";
    else
        echo "Powered by<bt><a href='http://enjoydecor.com' title='Toronto home staging and interior decorating'><img src='http://enjoydecor.com/_themes/_simple3/llogo.jpg' width='110px' alt='home staging enjoydecor logo'></a>";
    echo "</div></td></tr></table>";
    echo "<hr>";

    return $f;
}


$ff=main();
if($ff==0)
{
    include ("./config.inc.php");
    include ("./counter.class.php");
    $counter = new dcounter();
    $visits = $counter->show_counter();
    $aa=$visits['visits_today'];
    //$bb=$visits['total'];
    echo "<div align='center'><font size='1'>{$aa}</font></div>";
}

?>

</BODY>
</HTML>
