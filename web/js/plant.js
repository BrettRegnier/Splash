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

		// test
		curr = 167
		prev = max - 20;

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

moistures = null;

window.onload = function ()
{
	// TODO 
	// get the data returned from php

	// Place meter containers onto the page.
	var req = new XMLHttpRequest();
	req.open("GET", "build.php?t=0", true);
	req.send();
	req.onload = function ()
	{
		cont = document.getElementById("content");
		cont.innerHTML = this.responseText;
		DrawMeters();
	};
}

function sleep(ms)
{
	return new Promise(resolve => setTimeout(resolve, ms));
}

function DrawMeters() 
{
	// TODO loop through each content and place meters for each... 
	// TODO get data from php
	plants = document.getElementsByClassName("plant");
	console.log(plants);
	console.log(plants.length);

	for (var i = 0; i < plants.length; i++)
	{
		console.log(plants[i]);
		var name = plants[i].id;

		detectors = plants[i].getElementsByClassName("plant__detector");
		for (var j = 0; j < detectors.length; j++)
		{
			moistures = null
			console.log(detectors[j]);
			var detector = j;

			canvas = detectors[j].getElementsByClassName("meter__canvas")[0]
			console.log(canvas);

			// Get moisture level for this detector
			var req = new XMLHttpRequest();
			req.open("GET", "build.php?t=1&name=" + name + "&detector=" + detector);
			req.send();
		
			req.onload = function ()
			{
				moistures = this.responseText;
				
				console.log(moistures);	
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

function GetMoisturesForDetector(name, detector)
{
	var req = new XMLHttpRequest();
	req.open("GET", "build.php?t=1&name=" + name + "&detector=" + detector);
	req.send();

	req.onload = function ()
	{
		cont = document.getElementById("content");
		moistures = this.responseText;
		console.log(this.responseText);
	}
}
