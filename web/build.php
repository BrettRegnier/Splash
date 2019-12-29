<?php

$name = 'Hugh';
$id = 0;

$plants = "";
$detectors = "";

// TODO query database and adjust variables    
// for ($i = 0; $i < 2; $i++) {
// 	$id = $i;
	
// 	ob_start();
// 	include("plant.php");
// 	$v = ob_get_clean(); // get the ob of plant
// 	array_push($plants,  $v);

// 	$name = "jorg";	
// }



// TODO for loop for each plant in the db
// this is a test for if there two detectors
for ($i = 0; $i < 2; $i++) 
{
	$id = $i;
	ob_start();
	include("detector.php");
	$detectors = $detectors . ob_get_clean();
}

ob_start();
include("plant.php");
$plants = $plants . ob_get_clean();


echo $plants;
?>