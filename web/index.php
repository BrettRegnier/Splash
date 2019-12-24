<?php

// query database

$name = 'Hugh';
$plants = array();

// need a list of items to print into html

for ($i = 0; $i < 2; $i++) {
    ob_start();
    include("plant.php");
    $v = ob_get_clean();
    array_push($plants,  $v);
    $name = "jorg";
}
?>

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="X-UA-Compatible" content="ie=edge">
    <title>Splash</title>

    <link href="css/main.css" rel="stylesheet">
    <link href="css/nav.css" rel="stylesheet">
    <link href="css/styles.css" rel="stylesheet">
</head>

<body>
    <nav class="nav">
        <div class="nav__logo">
            <div class="nav__content">
                <a href="index.html" class="nav__link">
                    Splash
                </a>
            </div>
        </div>

        <div class="nav__container" id="nav_links">
            <div class="nav__content">
                <a class="nav__link nav__link--active" href="#">Dunnoyet</a>
            </div>
        </div>
        <!-- https://www.w3schools.com/howto/howto_css_menu_icon.asp -->
        <a href="javascript:void(0);" class="nav__display" onclick="DisplayNavBurger()">
            <div class="nav__burger" onclick="ToggleBurger(this)">
                <div class="nav__bar nav__bar1"></div>
                <div class="nav__bar nav__bar2"></div>
                <div class="nav__bar nav__bar3"></div>
            </div>
        </a>
    </nav>
    <section class="container">
        <?php
        for ($i = 0; $i < 2; $i++) {
            echo $plants[$i];
        }
        ?>
    </section>


    <script src="js/nav.js" type="text/javascript" defer="defer"></script>
    <script src="js/plant.js" type="text/javascript"></script>
</body>

</html>