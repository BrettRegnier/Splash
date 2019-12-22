function DisplayNavBurger()
{
    var x = document.getElementById("nav_links")
    if (x.style.display === "block") 
    {
        x.style.display = "none";
    }
    else
    {
        x.style.display = "block";
    }
}

function ToggleBurger(burger)
{
    burger.classList.toggle("nav__burger--change")
}