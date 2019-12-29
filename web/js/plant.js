window.onload = function ()
{
	// TODO 
	// get the data returned from php
	
	// Place meter containers onto the page.
	var req = new XMLHttpRequest();
	req.onload = function ()
	{
		cont = document.getElementById("content");
		cont.innerHTML = this.responseText;
	};

	req.open("GET", "build.php", true);
	req.send();

	this.DrawMeters();
}

function DrawMeters() 
{
	plants = document.getElementsByClassName("plant");
	console.log(plants);
	console.log(plants.length);
	AJAX_GetMoistureLevels()

	for (var i = 0; i < plants.length; i++)
	{

	}

	w = 150;
	h = 150;

	var max = 300.0;

	current = 100;
	previous = 200;

	if (current == max)
		current -= 0.1;
	if (previous == max)
		previous -= 0.1;

	var current = (current / max) * 2;
	var previous = (previous / max) * 2;

	var c = document.getElementById("meter");
	var ctx = c.getContext("2d");

	lw = 28;

	// maximum value
	ctx.beginPath();
	ctx.arc(w / 2, h / 2, 50, 0, 2 * Math.PI);
	ctx.strokeStyle = "#202020";
	ctx.lineWidth = 20;
	ctx.stroke();

	// previous value
	ctx.beginPath();
	ctx.arc(w / 2, h / 2, 46, 1.5 * Math.PI, (previous * Math.PI) - (0.5 * Math.PI));
	ctx.strokeStyle = "#79B8E5";
	ctx.lineWidth = lw / 2 - 4;
	ctx.stroke();

	// current value
	ctx.beginPath();
	ctx.arc(w / 2, h / 2, 54, 1.5 * Math.PI, (current * Math.PI) - (0.5 * Math.PI));
	ctx.strokeStyle = "#004D84";
	ctx.lineWidth = 10;
	ctx.stroke();
}

function AJAX_GetMoistureLevels()
{

}
