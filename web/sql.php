<?php
	function GetAllPlants()
	{
		$db = new SQLite3("../Splash.db");
		
		$plants = $db->query("SELECT name, detectors FROM Plants");
		// while ($row = $plants->fetchArray()) {
		// 	echo $row['name'];
		// 	echo $row['detectors'];
		// }
		
		return $plants;
	}

	function GetAllMoistureLevels($name, $detector)
	{
		$db = new SQLite3("../Splash.db");
		$moistures = $db->query("SELECT time, wasWatered, preMoistureLevel, postMoistureLevel FROM Moistures WHERE name='$name' AND detectorId=$detector");
		return $moistures;
	}
	
	function GetCurrPrevMoistureLevels($name, $detec)
	{
		// query database based on the name and id
		// return the current moisture level and the previous moisture level
	}

?>