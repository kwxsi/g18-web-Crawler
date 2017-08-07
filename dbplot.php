<?php

    define("HOST", "host=localhost");
    define("PORT", "port=5432");
    define("DBNAME", "dbname = WebCdb");
    define("USER", "user = postgres");
    define("PWD", "password = abcd1234");

    //connect to database
    $db = pg_connect(HOST. " ". PORT . " ". DBNAME. " " . USER . " ". PWD);
    if(!$db){
        die("<h1 style=\"padding: 20px, color: white, background-color:red\>ERROR! Unable to connect to database </h1>");
    }

        $sql = "SELECT country, latitude, longitude, link, mapserver, geoserver, qgis, deegree from public.webcrawlertable;";

    //run database query
    $ret = pg_query($db, $sql);

    if(!$ret){
        die("<h1 style=\"padding: 20px, color: white, background-color:red\>ERROR! Failed to retrieve data </h1>");
    }

    //Get data and put into array
    $country_name = array();
    $lat = array();
    $lng = array();
    $link = array();
    $mapserver = array();
    $geoserver = array();
    $qgis = array();
    $deegree = array();

    while($row = pg_fetch_row($ret)){
        array_push($country_name, $row[0]);
        array_push($lat, $row[1]);
        array_push($lng, $row[2]);
        array_push($link, $row[3]);
        array_push($mapserver, $row[4]);
        array_push($geoserver, $row[5]);
        array_push($qgis, $row[6]);
        array_push($deegree, $row[7]);
    }

    pg_close($db);

?>

<html>
    <head>
      <script type="text/javascript" src="leaflet\dist\leaflet.js"> </script>
      <link rel="stylesheet" href="leaflet\dist\leaflet.css">
    </head>
    <body>
        <div id="map" style="width: 100%; height: 800px"></div>
        <script type="text/javascript">

            var map;
            function init(){
                var country_name = [];
                var lat = [];
                var lng = [];
                var link = [];
                var mapserver = [];
                var geoserver = [];
                var qgis = [];
                var deegree = [];

                //Putting the values from the arrays
                <?php
                    for($i=0; $i<count($country_name); $i++){
                        echo "country_name[$i]='".$country_name[$i]."';\n";
                        echo "lat[$i]='".$lat[$i]."';\n";
                        echo "lng[$i]='".$lng[$i]."';\n";
                        echo "link[$i]='".$link[$i]."';\n";
                        echo "mapserver[$i]='".$mapserver[$i]."';\n";
                        echo "geoserver[$i]='".$geoserver[$i]."';\n";
                        echo "qgis[$i]='".$qgis[$i]."';\n";
                        echo "deegree[$i]='".$deegree[$i]."';\n";
                    }
                ?>

                map = L.map('map',{
                              center: [[5.6415, -0.1905]],
                              scrollWheelZoom: false,
                              inertia: true,
                              inertiaDeceleration: 2000
                            });
               map.setView([5.6415, -0.1905], 2);


                L.tileLayer('http://{s}.tile.osm.org/{z}/{x}/{y}.png', {
                              attribution: 'Map data © <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>',
                              maxZoom: 18
                            }).addTo(map);


               for (var i = 0; i < country_name.length; i++) {
                 var location = [];
                 location.push(lat[i]);
                 location.push(lng[i]);
                 var marker = L.marker(location).addTo(map);
                 var display = "<b>" + link[i] + "</b><br/>Country: "+country_name[i]+"<br/>Mapserver: " + mapserver[i] + "<br/>Geoserver: " + geoserver[i] + "<br/>QGIS: " + qgis[i] + "<br/> Deegree: " + deegree[i];
                 marker.bindPopup(display);
               }

             }

             init();
        </script>
    </body>
</html>
