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

require_once("include/@config.php");

function xx($c)
{
    echo("<pre><font color='red'>");
    if(is_array($c))
        print_r($c);
    else
        echo ($c);
    echo("</font>\n<pre>");
}

function tr_table($ord,&$r,$weight,&$words,$fwg)
{
  //  domain.name, domain.logo, domain.srvip,  links.linkid, links.title,links.dsc,links.name,links.url,links.txt, links.siteid

                    //links.linkid
    $linkid=$r[0];
    $dname=$r[1];  //domain
    $logo=$r[2];    //logo
    $ip=$r[3];      //ip

    $title=$r[5];   //title
    $dsc=$r[6];     //dsc
    $name=$r[7];    //name
    $url=$r[8];     //url
    $txt=$r[9];     //txt
    $siteid=$r[10];  //siteid

    $ctn="";
    //$domain="";

    foreach ($words as $w)
    {
        $i = strpos($txt, $w);
        $chk="..";
        if($i != FALSE)
        {
            if($i > 8) $i-=8;

            if(strlen($ctn))
                $chk .= " [...] ".substr($txt, $i, 80);
            else
                $chk .= substr($txt, $i, 128);

            $chk = @preg_replace("/{$w}/i", "<b>{$w}</b>", $chk);
            $ctn .= $chk;
            //$domain .= $w."+";
        }
    }

    if(strlen($title)==0)$title="index-page";
    if(strlen($title)>80) $title=substr($title,0,80)."...";
    if(strlen($logo))
    {
        if(substr($logo,0,4)!="http" && substr($logo,0,4)!="www.")
        {
            $logo=$dname."/".$logo;
        }
    }
    else
    {
        $logo="include/logo.png";
    }
    $weight=sprintf("%3.1f%%", $weight/($fwg+1) * 100);

echo("<tr><td><table width='840px' bgcolor='#CC8' border='0' cellspacing='1' cellpadding='1'>");
echo("<tr><td rowspan='3' width='120px' height='40px' class='bord' ".
    "align='center' style='background:url({$logo}) no-repeat left top; vertical-align: middle; '>{$r[0]}</td>");

echo("<td width='75%' class='link'>".
     "<div style='float:left;'>".
        "<a href='{$url}'>{$url}</a>".
     "</div>".
     "<div style='float:right;'>".
        "<a href='index.php?sitasets={$r[0]}&sid={$siteid}' title='page-assets'><img src=include/asets.png></a>".
     "</div>".
    "<div style='float:right;'>".
        "<a href='index.php?sitemap={$dname}' title='site-map'><img src=include/treeview.png></a>".
     "</div>".
    "<div style='float:right;'>".
        "<a href='index.php?wcount={$linkid}' title='page-score'><img src=include/score.png></a>".
     "</div>".

     "</td><td align='center'>#{$ord}</td></tr>\n");
echo("<tr bgcolor='#DD9'><td width='80%'>{$title}</td><td class='sfont' align='center' >".
"<div class='meter-wrap'>".
"<div class='meter-value' style='background-color: #994; width: {$weight};'>".
"<div class='meter-text'>".
$weight.
"</div>".
"</div>".
"</div>".
"</td></tr>\n");
echo("<tr bgcolor='#EEA' ><td width='75%' class='sfont'>{$ctn}</td><td class='ufont' align='center'>".
     "<a href='#' title='ajax:include/nmap.php?ip={$ip}'>{$ip}<br>disclose...</a></td></tr>\n");
echo("</table></td></tr>\n");
}


function get_ws(&$con,$query,&$sugest,&$bsug,&$k2id,&$id2k,&$ksums)
{
    $k=0;
    global $_WORDS;
    $words=explode(" ",$query);
    $ksum;
    foreach($words as $w)
    {
        $ksum=wtbl($w);
        $sql="SELECT wordid,wrd,ksum FROM dict WHERE ksum='{$ksum}' AND wrd='{$w}';";
        $result = mysqli_query($con,$sql) or die(mysqli_error($con));
        if(mysqli_num_rows($result)==0)
        {
            $ksum=wtbl($w);
            $sql="SELECT wordid,wrd,ksum FROM dict WHERE ksum='{$ksum}' AND wrd='{$w}';";
            $max_distance = 100;
            $near_word ="";
            $result = mysqli_query($con,$sql) or die(mysqli_error($con));
            while ($row2=mysqli_fetch_row($result))
            {
                $distance = levenshtein($row2[1], $w);
                if ($distance < 16)
                {
                    $k2id[$row2[1]]=$row2[0];
                    $id2k[$row2[0]]=$row2[1];
                    $ksums[$row2[0]]=$ksum;
                    $sugest.=$row2[1]." ";
                    $bsug=true;
                    break;
                }
            }
        }
        else
        {
            while ($row=mysqli_fetch_row($result))
            {
                $k2id[$row[1]]=$row[0];
                $id2k[$row[0]]=$row[1];
                $ksums[$row[0]]=$ksum;
                $sugest.=$row[1]." ";
                break;
            }
        }
        if($k++ > $_WORDS)
            break;
    }
    mysqli_free_result($result);
}

function by_links($a, $b)
{
    return $a->weight <= $b->weight;
}

class link
{
    var $wid=array();
    var $weight;
    var $sid;

    function __construct(&$r)
    {
        array_push($this->wid,$r[1]);
        $this->weight += intval($r[2]);
         $this->sid=$r[3];
    }
    function add(&$r)
    {
        array_push($this->wid,$r[1]);
        $this->weight += intval($r[2]);
        $this->sid=$r[3];
    }
    var $_pages=array();
};

class c_links
{
    function add_rs(&$rsl)
    {
        $this->_linkid[$rsl[0]]->_pages=$rsl;
    }
    function add($r)
    {
        if(!isset($this->_linkid[$r[0]]))
        {
            $this->_linkid[$r[0]]=new link($r);
        }
        else
        {
            $this->_linkid[$r[0]]->add($r);
        }
    }
    var $_linkid=array();
};


function ex($w, $f, $n, $k, $s, &$fwg)
{
    global $MULTIPLE_KWS;   $start = $f;    $maxweight=0;
    $k2id=array();          $sugest=null;   $show=false;
    $ksums=array();         $id2k=array();
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));
    get_ws($con,$w,$sugest,$show,$k2id,$id2k,$ksums);

    //xx($k2id);
    if($show)
        echo "I mean: {$sugest}";
    if(count($k2id)==0)
    {
        echo "<H4><font color='red'>Miau!</font> no pages matched for <font color='#622'>{$w}</font> !!!</H4>";
        mysqli_close($con);
        return 0;
    }
    $x=0;
    $qry="";

    $qry="SELECT DISTINCT(links.linkid),domain.name,domain.logo,domain.srvip,links.linkid,links.title,links.dsc,links.name,links.url,links.txt,links.siteid,words.weight".
    " FROM links\n".
    " JOIN words ON (words.linkid=links.linkid) \n".
    " JOIN domain ON (domain.siteid=links.siteid) \n".
    " WHERE words.wordid IN (\n";
    foreach($k2id as $k=>$i)
    {
        $qry.="{$i}";
        if($x++<count($k2id)-1)
        {
            $qry.=",";
        }
    }
    $qry.= ") ORDER BY words.weight DESC LIMIT {$f}, {$n}";
    //xx($qry);

    $result = mysqli_query($con,$qry) or die(mysqli_error($con));
    $rr=0;
    $maxscore=$fwg;
    echo "<table width='840px' bgcolor='#CC8'>";
    while($r=mysqli_fetch_row($result))
     {
        $sc=intval($r[11]);
        if($maxscore==0)
        {
            $maxscore=$sc;
            $fwg=$maxscore;
        }
        tr_table($start,$r,$sc,$id2k,$maxscore);
        $start++;
     }
     mysqli_free_result($result);
     echo "</table>\n";


    //--------------------------------------------------------------------------------------------
    echo "<table width='840px' bgcolor='#CC8' textcolor='#C00'>";
    $mitz=shell_exec ("ps ax | grep mitza | grep -v grep ");
	$qry="SELECT COUNT(*),url FROM que";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	if($r=mysqli_fetch_row($result))
	{
        if($mitz)
            $mitz="mitzing:{$r[1]}, queue:{$r[0]}";
        else
            $mitz="pouring:{$r[1]}, queue:{$r[0]}";
	}
	$links=0;
	$qry="SELECT COUNT(*) FROM links";
    mysqli_free_result($result);
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	if($r=mysqli_fetch_row($result))
	{
		$links=$r[0];
	}
	$sites=0;
	$qry="SELECT COUNT(*) FROM domain";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	if($r=mysqli_fetch_row($result))
	{
		$sites=$r[0];
	}
    mysqli_free_result($result);
    echo"<tr><td class='sfont' align='center'>{$mitz}, sites:{$sites}, pages:{$links}</td></tr>";
    echo "</table>";
    mysqli_close($con);
    return $start;
}


//---------------------------------------------------------------------------------------------------------
function sites()
{
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));
    $qry="SELECT * FROM domain";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	echo("<pre>");
	if($r=mysqli_fetch_row($result))
	{
		print_r($r);
	}
	echo("</pre>");
    mysqli_free_result($result);
    mysqli_close($con);
}

function dbcontent($w)
{
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));
    $qry="SELECT txt FROM links WHERE url LIKE '%{$w}%'";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	echo("<pre>");
	if($r=mysqli_fetch_row($result))
	{
		echo($r[0]);
	}
	echo("</pre>");
    mysqli_free_result($result);
    mysqli_close($con);
}



function sm($site)
{
    echo "site map: {$site}";
    echo "<div id='layer1' style='position:relative;  left:0px; top:0px; width:840px;  height:600px; z-index:1; overflow: auto; visibility: visible; '>";
    echo "<img src='include/smap.php?sitemap={$site}' width='840' alt='sitemap'/></div>";
}

function _get_logest(&$parts, &$type)
{

   // xx($parts);

    $l="";
    foreach($parts as $part)
    {
        if(strlen($part) > strlen($l))
        {
            $l=$part;
        }
    }
    return preg_replace("/['\";]+/","",$l);
}

function sa($w,$sid)
{
    echo ("<h4>Page Links,Images and scripts</h4>");
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $alt='#CC8';
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));
    $qry="SELECT assets.href,domain.name FROM assets JOIN domain ON (domain.siteid={$sid}) WHERE plinkid={$w}";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	echo "<table width='100%'><tr bgcolor='#883'><td width='20%'>Type</td><td>Link</td></tr>\n";

	while($r=mysqli_fetch_row($result))
	{
        $link=$r[0];
        $dname=$r[1];
        $nobraks=str_replace("<","",$link);
        $nobraks=str_replace(">","",$nobraks);
        $nobraks=str_replace("\"","'",$nobraks);
        preg_match_all("/(?:(?:\"(?:\\\\\"|[^\"])+\")|(?:'(?:\\\'|[^'])+'))/is", $link, $parts);

        $assumed=_get_logest($parts[0],$type);
        $short=$assumed;
        $path_info = pathinfo($assumed);
        if(isset($path_info['extension']))
            $type=$path_info['extension'];
        else
            $type="not detected!";

        if(substr($assumed,0,4)!="http")
        {
            if(substr($assumed,0,1)!="/")
                $assumed=$dname."/".$assumed;
            else
                $assumed=$dname.$assumed;
        }
        echo "<tr td bgcolor='{$alt}'><td class='sfont' width='20%'>{$type}</td>".
             "<td class='sfont'>";
            echo "<a href='$assumed'>$short</a>";
        echo "</td></tr>";
        if($alt=='#CC8')
            $alt='#EEA';
        else
            $alt='#CC8';
	}
	echo("</table>");
    mysqli_free_result($result);
    mysqli_close($con);
}


?>





