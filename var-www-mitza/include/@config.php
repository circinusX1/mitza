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

define ("DB_HOST","localhost");
define ("DB_DATABASE","mitza");
define ("DB_USER","muser");
define ("DB_PASS","mpass");

//#define KSUM_VALUE 256
$_WORDS=4;
$_PAGE=8;
$MULTIPLE_KWS=0;

function wtbl($word)
{
    global $MULTIPLE_KWS;

    $ksum = 256;// $_TABLES;
    $sum=0;
    for($i=0; $i<strlen($word); $i++)
    {
        $sum += ord($word[$i]);
    }
    if($MULTIPLE_KWS)
        return ($sum % $MULTIPLE_KWS);
    return ($sum % $ksum);
}

function config_prep($expresion)
{
    $n_words = count(explode(" ",$expresion));
    if (!get_magic_quotes_gpc())
    {
        $ss = addslashes($expresion);
    }
    else
    {
        $ss = $expresion;
    }
    //$my_query_string_link = stripslashes($ss);
    $ss = str_replace('_','\_',$ss); // avoid '_' in the query
    $ss = str_replace('%','\%',$ss); // avoid '%' in the query
    $ss = str_replace('\"',' ',$ss); // avoid '"' in the query
    $ss = strtolower($ss);
    $ss = trim(ereg_replace(" +"," ",$ss)); // no more than 1 blank
    return $ss;
}




function get_wsmatch(&$con, $query, &$retval, &$sugest, &$bsug)
{
    $k=0;
    global $_WORDS;

    $words=explode(" ",$query);
    foreach($words as $w)
    {
        $sql="SELECT wordid, wrd FROM dict WHERE wrd='{$w}';";
        //echo $sql;
        $result = mysqli_query($con,$sql) or die(mysqli_error($con));
        if(mysqli_num_rows($result)==0)
        {
            mysqli_free_result($result);
            $sql="SELECT wordid, wrd FROM dict where soundex(wrd)=soundex('{$w}');";
            //echo $sql;
            $max_distance = 100;
            $near_word ="";
            $result = mysqli_query($con,$sql) or die(mysqli_error($con));
            while ($row2=mysqli_fetch_row($result))
            {
                $distance = levenshtein($row2[1], $w);
                if ($distance < 16)
                {
                    $retval[$row2[1]]=$row2[0];
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

                $retval[$row[1]]=$row[0];
                $sugest.=$row[1]." ";
                break;
            }

        }
        if($k++ > $_WORDS)
            break;
    }
    //print_r($retval);

    mysqli_free_result($result);
}


?>
