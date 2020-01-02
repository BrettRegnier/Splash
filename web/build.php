<?php
include_once("sql.php");

function BuildPlants()
{
	$name = "";
	$id = 0;

	$plants = "";
	$detectors = "";

	$sql = GetAllPlants();

	while ($row = $sql->fetchArray()) {
		$name = $row['name'];
		$dcount = $row['detectors'];

		$detectors = "";

		for ($i = 0; $i < $dcount; $i++) {
			$id = $i;
			ob_start();
			include("detector.php");
			$detectors = $detectors . ob_get_clean();
		}

		ob_start();
		include("plant.php");
		$plants = $plants . ob_get_clean();
	}

	return $plants;
}

function GetMoistures()
{
	$name = $_REQUEST["name"];
	$detector = $_REQUEST["detector"];
	$moistures = array();

	$sql = GetAllMoistureLevels($name, $detector);
	$test = "";
	// TODO i dunno figure this out somehow
	while ($row = $sql->fetchArray(2)) 
	{
		// var_dump($row);
		// $leng = count($row);
		// for ($i = 0; $i < $leng; $i++)
		// {
		// 	$moistures .= strval($row[$i]);
		// }
		// $moistures .= " ";
		
		$moistures[] = $row;
	}
	// var_dump($moistures);

	return json_encode($moistures);
}

// Types: 0 = plant, 1 = moistures

$type = $_REQUEST["t"];

if ($type == 0)
	echo BuildPlants();
else if ($type == 1)
	echo GetMoistures();
