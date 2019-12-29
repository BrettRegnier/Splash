<?php
	function GetAllPlants()
	{
		$db = new SQLite3('../splash.db');
		
		$plants = $db->query('SELECT name FROM Plants');
		while ($row = $plants->fetchArray()) {
			echo $row['name'];
		}
		
		return json_encode(42);
	}

	function GetAllMoistureLevels($name, $detec)
	{
		
	}
	
	function GetCurrPrevMoistureLevels($name, $detec)
	{
		// query database based on the name and id
		// return the current moisture level and the previous moisture level
	}

?>