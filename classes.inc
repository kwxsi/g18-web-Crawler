<?php
include_once 'helper_functions.inc';
include_once 'dependencies/php-crawl/libs/PHPCrawler.class.php';

//connecting to postgres server
define("HOST", "host=localhost");
define("PORT", "port=5432");
define("DBNAME", "dbname=WebCdb");
define("USER", "user=postgres");
define("PWD", "password=group18");

class Crawler extends PHPCrawler {

  public function handleDocumentInfo(PHPCrawlerDocumentInfo $pageInfo) {

    $source = strtolower($pageInfo->source);
    $keywords = array('mapserver', 'geoserver', 'qgis', 'deegree');
    $services = array('mapserver' => 'n','geoserver' => 'n','qgis' => 'n','deegree' => 'n');

    $found = false;
    foreach ($keywords as $keyword) {
      if(contains($source, $keyword)){
          $services[$keyword] = 'y';
          $found = true;
      }
    }

    if($found)$this->addToFound($pageInfo->url, $services);
  }

  private function addToFound($link, $services) {


    //connect to database
    $db = pg_connect(HOST. " ". PORT . " ". DBNAME. " " . USER . " ". PWD);
    if(!$db){
        die("<h1 style=\"padding: 20px, color: white, background-color:red\>ERROR! Unable to connect to database </h1>");
    }

    // $link = getDomain($link);
    // $sql = "SELECT * from public.webcrawlertable WHERE link='$link'";
    // $result = pg_query($db, $sql);
    //
    // $count = count(pg_fetch_all($result));
    //
    // if($count > 0)return;

    $host = getDomain($link);
    $details = $this->requestDetails($link);

    $location = array('0','0');
    $temp = explode(',',$details['loc']);

    if(array_key_exists(0, $temp))$location[0] = $temp[0];
    if(array_key_exists(1, $temp))$location[1] = $temp[1];

    $location = explode(',', $details['loc']);

    // if(!array_key_exists(0, $location))$location[0] = 0;
    // if(!array_key_exists(1, $location))$location[1] = 0;

    $sql = "INSERT INTO public.webcrawlertable(link,country,latitude,longitude,geoserver,mapserver,qgis,deegree) VALUES('$link','$details[country]','$location[0]','$location[1]','$services[geoserver]','$services[mapserver]','$services[qgis]', '$services[deegree]')";
    $ret = pg_query($db, $sql);
    // $ret = pg_query_params($db, $sql,$params);
    if(!$ret)die("<h1 style=\"padding: 20px, color: white, background-color:red\>ERROR! Failed to insert data into database </h1>");
      pg_close($db);
  }

  private function getIP($url) {
    $domain = getDomain($url);
    $ip_address = gethostbyname($domain);
    return $ip_address;
  }

  public function requestDetails($url) {
    $ip = $this->getIP($url);

    echo $ip."<br/>";
    $request = 'ipinfo.io/'.$ip;
    $curl_request = curl_init($request);
    curl_setopt($curl_request, CURLOPT_RETURNTRANSFER, true);
    $response = curl_exec($curl_request);
    curl_close($curl_request);

    $details = json_decode($response, true);
    return $details;
  }
}



?>
