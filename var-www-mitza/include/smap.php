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


function _recurs(&$con, $site, &$hier, &$labs, $siteb)
{
    $qry="SELECT linkid FROM links WHERE url='{$site}'";
    $result = mysqli_query($con,$qry) or die(mysqli_error($con));
	if($r=mysqli_fetch_row($result))
	{
        $id = $r[0];

        $labs[$id]=str_replace("{$siteb}/","",$site);

        mysqli_free_result($result);
	    $qry="SELECT linkid,url FROM links WHERE plinkid={$id}";
        $result = mysqli_query($con,$qry) or die(mysqli_error($con));
        $clilds=array();
        while($r=mysqli_fetch_row($result))
        {
            $hier.="{$id} -> {$r[0]} \n";
            $clilds[]=$r[1];
        }
        mysqli_free_result($result);

        foreach($clilds as $chi)
        {
            _recurs($con, $chi, $hier, $labs, $siteb);
        }
	}
}


function sitemap($site)
{
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));
    $sfile = preg_replace("(http|\//|\/|:|\.)","", $site);
    if(!is_file("tttmp/{$sfile}.dot"))
    {
        $labs=array();
        _recurs($con, $site, $hier, $labs, $site);

        $fc="digraph sitegraph {graph [ dpi = 60 ];\nrankdir=LR;\n bgcolor=\"#EFEFAF\";\n";
        foreach($labs as $idx=>$lab)
        {
            $fc .= "{$idx} [label=\"{$lab}\"];\n";// [label='{$lab}'];\n}";
        }

        $fc .= "\n{$hier} }\n";
        file_put_contents("tttmp/{$sfile}.dot", $fc);
        shell_exec("dot -Tjpg tttmp/{$sfile}.dot > tttmp/{$sfile}.jpg");
    }
   // header("Content-type: image/jpg");
    readfile("tttmp/{$sfile}.jpg");
    mysqli_close($con);
}

function maine()
{
//
    if(isset($_GET['sm']))
        sitemap($_GET['sm']);

}


if(isset($_GET['sitemap']))
    sitemap($_GET['sitemap']);


?>

