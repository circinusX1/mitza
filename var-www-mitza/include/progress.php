<?php
    require_once("include/@config.php");
    $con = mysqli_connect(DB_HOST, DB_USER, DB_PASS);
    if (!$con)
    {
        die("Could not connect:".mysql_error());
    }
    $result = mysqli_query($con,"USE mitza;") or die(mysqli_error($con));

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
	$qry="SELECT COUNT(*) FROM wss";
	$result = mysqli_query($con,$qry) or die(mysqli_error($con));
	if($r=mysqli_fetch_row($result))
	{
		$sites=$r[0];
	}
    mysqli_free_result($result);
    echo $mitz;
    mysqli_close($con);

?>
