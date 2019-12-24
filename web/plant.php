<div class="plant">
	<div class="plant__title">
		<h2><?php echo $name ?></h2>
	</div>
	<div class="plant__container">
		<div class="plant__info">
			<div class="plant__meter">
				<canvas id="meter"></canvas>
				<div class="meter__legend">
					<div class="legend__item">
						<span class="legend__item--current"></span>
						<span>Current moisture level</span>
					</div>
					<div class="legend__item">
						<span class="legend__item--previous"></span>
						<span>Previous moisture level</span>
					</div>
				</div>
			</div>
			<div class="plant__graph">
				<p>nothing yet</p>
			</div>
			<div class="plant__stats">
				<ul>
					<li>Moisture levels</li>
					<ul class="stats__moisture">
						<li class="moisture__highest">
							<span>Highest: <?php echo $moistureHighest; ?></span>
						</li>
						<li class="moisture__lowest">
							<span>Lowest: <?php echo $moistureLowest; ?></span>
						</li>
						<li class="moisture__average">
							<span>Average: <?php echo $moistureAverage; ?></span>
						</li>
					</ul>
					<li class="stats__lastMeasurement">
						<span>Last Measurement: <?php echo $lastMeasurement; ?></span>
					</li>
					<li class="stats__lastWatered">
						<span>Last time watered: <?php echo $lastWatered; ?></span>
					</li>
				</ul>
			</div>
		</div>
		<div class="plant__config">
			<p>nothing yet</p>
		</div>
	</div>
</div>