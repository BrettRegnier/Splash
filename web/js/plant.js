class Meter 
{
	constructor(canvas, cw, ch, prev, curr)
	{
		// this.Resize(cw, ch);
		this._canvas = canvas;
		this._previous = prev;
		this._current = curr;

		this.Draw();
	}

	Draw()
	{
		var ctx = this._canvas.getContext("2d");
		var w = this._canvas.width / 2;
		var h = this._canvas.height / 2;

		// clear the canvas
		ctx.clearRect(0, 0, w, h);


		// draw meter

		var max = 300.0; // TODO find this actual number from sensor.
		var prev = this._previous;
		var curr = this._current;

		// prevent the line being too large and over drawing.
		if (prev >= max) prev = max - 0.1;
		if (curr >= max) curr = max - 0.1;

		prev = (prev / max) * 2; // fit it onto the expected data size
		curr = (curr / max) * 2; // fit it onto the expected data size

		console.log(curr);
		// max
		this.DrawMeter(ctx, w, h, 50, 0, 2 * Math.PI, "#202020", 20);

		// previous
		this.DrawMeter(ctx, w, h, 46, 1.5 * Math.PI, (prev * Math.PI) - (0.5 * Math.PI), "#79B8E5", 6);

		// current
		this.DrawMeter(ctx, w, h, 53, 1.5 * Math.PI, (curr * Math.PI) - (0.5 * Math.PI), "#004D84", 12);
	}

	DrawMeter(ctx, w, h, r, start, end, color, lw)
	{
		ctx.beginPath();
		ctx.arc(w, h, r, start, end);
		ctx.strokeStyle = color;
		ctx.lineWidth = lw;
		ctx.stroke();
	}

	Click()
	{
		// TODO
		console.log("clicked");
	}

	UpdateReadings(prev, curr)
	{
		this._previous = prev;
		this._current = curr
	}
}

class LineGraph
{
	constructor(canvas, w, h, pre, post, dates)
	{
		console.log(dates);
		var x = canvas.getContext("2d");
		var d = new Date();
		this.config = {
			type: "line",
			data: {
				labels: dates,
				datasets: [{
					label: "Pre-watering Moistures",
					backgroundColor: "rgba(250, 100, 100)",
					borderColor: "rgba(72, 192, 192, 1)",
					fill: false,
					data: pre,
				},
				{
					label: "Post-watering Moistures",
					backgroundColor: "rgba(200, 100, 100)",
					borderColor: "rgba(72, 70, 192, 1)",
					fill: false,
					data: post,					
				}
				]
			}
		}
		
		var chart = new Chart(canvas, this.config);

		var ctx = canvas.getContext("2d");
		
	}

	Resize()// todo
	{

	}

	Update(data)
	{
		// max is always about 300 on the analog scale, maybe I should scale it down to 100...
		// TODO once the max is found replace the value here... assuming 300 for now.
		// TODO once the max is found I will need to read the values as a function of percentage.


		// TODO datetime sets on the xaxis
	}
}

moistures = null;

window.onload = function ()
{
	// TODO 
	// get the data returned from php

	// Place meter containers onto the page.
	function GetPlants(result)
	{
		cont = document.getElementById("content");
		cont.innerHTML = result;
		DrawMeters();
	}
	AjaxGetPlants(GetPlants);
	// var req = new XMLHttpRequest();
	// req.open("GET", "build.php?t=0");
	// req.send();
	// req.onload = function ()
	// {
	// };
}

function sleep(ms)
{
	return new Promise(resolve => setTimeout(resolve, ms));
}

function DrawMeters() 
{
	plants = document.getElementsByClassName("plant");

	for (var i = 0; i < plants.length; i++)
	{
		var name = plants[i].id;
		var detectors = plants[i].getElementsByClassName("plant__detector");

		for (var j = 0; j < detectors.length; j++)
		{
			moistures = null
			var detector = j;

			// Get the canvases
			var meterCanvas = detectors[j].getElementsByClassName("meter__canvas")[0]
			var graphCanvas = detectors[j].getElementsByClassName("graph__canvas")[0];

			cmd = "build.php?t=1&name=" + name + "&detector=" + detector;
			AjaxGetMoistures(cmd, HandleMoistures, meterCanvas, graphCanvas);

			function HandleMoistures(result, meterCanvas, graphCanvas)
			{
				moistures = JSON.parse(result);

				// document.getElementById("content").innerHTML = this.responseText;
				// console.log(moistures);

				// Convert each sql time to a javascript time and find the two most recent
				moistures[0][0] = new Date(moistures[0][0]);
				currentDate = moistures[0][0];
				previousDate = null;
				current = moistures[0];
				previous = null;
				
				dates = [moistures[0][0].toLocaleDateString()];
				premoistures = [moistures[0][2]];
				postmoistures = [moistures[0][3]];
				for (var i = 1; i < moistures.length; i++)
				{
					moistures[i][0] = new Date(moistures[i][0]);
					tmp = moistures[i][0];
					if (tmp > currentDate)
					{
						previousDate = currentDate;
						previous = current;

						currentDate = tmp;
						current = moistures[i];
					}
					
					dates.push(moistures[i][0].toLocaleString());
					premoistures.push(moistures[i][2]);
					postmoistures.push(moistures[i][3]);
				}
				console.log(moistures);
				// console.log(previous);
				// console.log(current);
				// console.log(dates);
				// console.log(premoistures);
				// console.log(postmoistures);

				var meter = new Meter(meterCanvas, 100, 100, previous[3], current[3]);
				var graph = new LineGraph(graphCanvas, 500, 500, premoistures, postmoistures, dates);
			}
			// sleep(100000);

			// console.log(moistures);
			// tms = 0;
			// while (moistures == null)
			// {
			// 	sleep(1000);
			// 	tms += 1000;
			// 	if (tms > 100000000)
			// 	{
			// 		console.log(tms);
			// 		console.log("timeout");
			// 		return;
			// 	}
			// }

			// console.log("test");
			// var meter = new Meter(canvas, 100, 100)
		}
	}
}

function DrawGraph(parent)
{
	var margin = { top: 10, right: 30, bottom: 30, left: 50 },
		width = 460 - margin.left - margin.right,
		height = 400 - margin.top - margin.bottom;

	// append the svg object to the body of the page
	var svg = d3.select(parent)
		.append("svg")
		.attr("width", width + margin.left + margin.right)
		.attr("height", height + margin.top + margin.bottom)
		.append("g")
		.attr("transform",
			"translate(" + margin.left + "," + margin.top + ")");

}

// function FormatDateFromSQL(sqldate)
// {
// 	var split = sqldate.split("-");

// 	var year = Number(split[0]);
// 	var month = Number(split[1]);


// 	console.log(year);
// 	console.log(month);
// }

function AjaxGetPlants(callback)
{
	var req = new XMLHttpRequest();
	req.onload = function ()
	{
		callback(this.responseText);
	}
	req.open("GET", "build.php?t=0");
	req.send();
}


function AjaxGetMoistures(cmd, callback, canvas, graph)
{
	var req = new XMLHttpRequest();
	req.onload = function ()
	{
		callback(this.responseText, canvas, graph);
	}
	req.open("GET", cmd);
	req.send();
}
