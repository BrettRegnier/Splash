<div class="plant" id='<?php echo $name; ?>'>
	<div class="plant__title">
		<h2><?php echo $name ?></h2>
	</div>
	<div class="plant__container">
		<?php echo $detectors; ?>
		<div class="plant__info">
			<div class="plant__stats">
				<ul>
					<li>Moisture levels</li>
					<ul class="stats__moisture">
						<li class="moisture__highest">
							<span>Highest: --</span>
						</li>
						<li class="moisture__lowest">
							<span>Lowest: --</span>
						</li>
						<li class="moisture__average">
							<span>Average: --</span>
						</li>
					</ul>
					<li class="stats__lastMeasurement">
						<span>Last Measurement: --</span>
					</li>
					<li class="stats__lastWatered">
						<span>Last time watered: --</span>
					</li>
				</ul>
			</div>
			<div class="plant__config">
				<p>nothing yet</p>
			</div>
		</div>
	</div>
</div>
