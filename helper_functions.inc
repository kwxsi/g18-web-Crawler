<?php
/*************************************************************************
* This file contains helper funtions for the crawler, indexer and the search
* Created as: Part of the new and improved web software architecture crawler project
* Created by: George Osae
* Created on: 26th April, 2017
**************************************************************************/

// Turn of php xml warnings to enable user error handling

 libxml_use_internal_errors(true);

 // Returns the domain of a url
 function getDomain($url) {
   $parsed_url = parse_url($url);
   $domain = $parsed_url['host'];
   return $domain;
 }

 // Searches for a needle in a haystack
 function contains(string $haystack, $needle) {
   $pos = strpos($haystack, $needle);
   return $pos;
 }

 // Checks if $url is a potential wfs server
 function isPotentialServer($url) {
   $url = strtolower($url);
   $keywords = array('/geoserver',
                     '/ows',
                     'ogc',
                     '/mapserv',
                     '/wfs',
                     '/wms',
                     '/wps',
                     '/wcs',
                     'version=1.',
                     'version=2.',
                     'request=getcapabilities',
                     'request=getmap',
                     'request=getfeature',
                     'request=describefeaturetype');

   foreach ($keywords as $keyword) {
     if(contains($url, $keyword)){
       return true;
     }
   }
   return false;
 }

 // Creates a dom document from response
 function parseResponse($response) {
   return dom_import_simplexml(simplexml_load_string($response));
 }

 // Removes any get parameters from $url
 function removeGetParams($url) {
   if (contains($url, "?")) {
     $trimmed_url = (explode('?', $url))[0];
     return $trimmed_url;
   } else {
     return $url;
   }
 }

 // Sends a WFS GetCapabilities Request to a host specified by $url
 function sendCapabilitiesRequest($url) {
  $capabalities_request = "";
  $lowercase_url = strtolower($url); // For case insensitive comparisons

  //  first of all check if it was originally a wms, wps or wcs request
  if(contains($lowercase_url, "service=wms") || contains($lowercase_url, "service=wcs") || contains($lowercase_url, "service=wps")) {
    $url = removeGetParams($url);
    $lowercase_url = removeGetParams($lowercase_url);
  }


  // Else check if url is already a wfs getcapabilities request and act accordingly
  if(contains($lowercase_url,"request=getcapabilities")){/*Don't touch the url*/}
  else {
    $url = removeGetParams($url);
    $url = $url."?service=wfs&request=GetCapabilities";
  }

  $capabilities_request = $url;
  $curl_request = curl_init($capabilities_request);
  curl_setopt($curl_request, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:53.0) Gecko/20100101 Firefox/53.0");
  curl_setopt($curl_request, CURLOPT_RETURNTRANSFER, true);
  $response = curl_exec($curl_request);
  curl_close($curl_request);

  // $response = @file_get_contents($capabilities_request);

   return $response;
 }

 function getRequestableURL($url) {
   if(contains($url, "/wms") || contains($url, "/wps") || contains($url, "/wcs")) {
       $last_forwardslash_pos = strrpos($url, "/");
       $url = substr_replace($url, "/wfs", $last_forwardslash_pos);
   }
   return $url;
 }

 // Checks if the response sent by the host after
 // the GetCapabilities request is a valid WFS capabilities document
 // Performs a number of checks
 function validateResponse($response) {
   $simple_xml_object = simplexml_load_string($response);

   // Check first if response is valid xml
   // Next check if response contains certain keywords
   if ($simple_xml_object === false){
     echo "Not Valid xml\n";
     return false;
   }
   elseif (!containsKeyWords(strtolower($response)))return false;
   else
     return true;
 }

 // A Capabilities document should contain certain keywords
 // If they are all present then $response is a valid capabilities doc
 function containsKeyWords($response) {
   $response = strtolower($response);
   $keywords = array('wfs', 'getcapabilities', 'getfeature', 'describefeaturetype');

   foreach ($keywords as $keyword) {
     if(!contains($response, $keyword)){
       echo "Does not contain $keyword\n";
       return false;
     }
   }
   return true;
 }

 function getWFSVersion($parsed_doc) {
   $version = '2.0.0'; // The latest WFS version is assumed
   $version = $parsed_doc->getAttributeNode('version')->value;
   return $version;
 }

 // Returns an array of all formats supported by the Server
 // Desired formats are output formats for the GetFeature operation
 // and wkt crs format for conversions to and from geometry objects
 function getSupportedFormats($parsed_doc, $version="2.0.0") {
   $supported_formats = array();

   if($version === "1.0.0") {
     $operations = $parsed_doc->getElementsByTagName('GetFeature');
     $result_formats;
     foreach ($operations as $operation) {
       $result_formats = $operation->getElementsByTagName('Resultformat');
       break;
     }
    //  $result_formats = $get_feature_operation->getElementsByTagName('resultformat');

     $output_formats;
     foreach ($result_formats as $result_format) {
       $output_formats = $result_format->getElementsByTagName('*');
     }

     foreach ($output_formats as $format) {
       array_push($supported_formats, strtolower($format->tagName));
     }

   } else {
     $operations = $parsed_doc->getElementsByTagName('Operation');
     $parameters = getDesiredElement($operations, 'GetFeature', 'Parameter');
     $allowed_values = getDesiredElement($parameters, 'outputFormat', 'AllowedValues');

     $values = "values";

     if ($allowed_values->length == 0) {
       $values = getDesiredElement($parameters, 'outputFormat', 'Value');
     } else {
       foreach ($allowed_values as $allowed_value) {
         $values = $allowed_value->getElementsByTagName('Value');
       }
     }

     foreach ($values as $value) {

       array_push($supported_formats, strtolower($value->nodeValue));
     }
   }

   $wkt_crs_support = getWKTSupport($parsed_doc, $version);
   if($wkt_crs_support)array_push($supported_formats, 'wkt');
   return $supported_formats;
 }

 // The WKT CRS is not an output format but rather used for operations
 // It is used to define shapes (polygons, circles etc) on a map for drawing
 function getWKTSupport($parsed_doc, $version) {

   if($version === "1.0.0") {
     $functions = $parsed_doc->getElementsByTagName('Function_Name');
     if($functions->length == 0)return false;
     foreach ($functions as $function) {
       if($function->nodeValue == 'toWKT' || $function->nodeValue == 'geomFromWKT')
        return true;
     }
   } else {
     $functions = $parsed_doc->getElementsByTagName('Function');
     if($functions->length == 0)return false;
     foreach ($functions as $function) {
       if($function->getAttribute('name') == 'toWKT' || $function->getAttribute('name') == 'geomFromWKT')
         return true;
     }
   }


   return false;
 }


 // Helper function for getSupportedOutputFormats
 function getDesiredElement($node_list, $parent_attribute, $child_tag_name) {

   $desired_child = new DOMNodelist();
   foreach ($node_list as $node) {
     if ($node->getAttribute("name") == $parent_attribute) {
       $desired_child = $node->getElementsByTagName($child_tag_name);
       break;
     }
   }
   return $desired_child;
 }

// Helper function to build an sql query from options selected. Used by search.php
function buildQuery($countries_to_search_in, $formats_to_search_for) {
  $query = "";

  if(count($formats_to_search_for) == 0 && count($countries_to_search_in) == 0) {
      $query = $query."select * from servers where 1";
  } elseif (count($formats_to_search_for) > 0 && count($countries_to_search_in) == 0) {
      $query = $query. "select * from servers where (";
      foreach ($formats_to_search_for as $format) {
        $query = $query." $format = 'y'";
        if($format !== $formats_to_search_for[count($formats_to_search_for) - 1]) $query = $query." and";
      }

      $query = $query.")";
  } elseif (count($formats_to_search_for) == 0 && count($countries_to_search_in) > 0) {
    $query = $query. "select * from servers where (";
    foreach ($countries_to_search_in as $country) {
      $query = $query." country = '$country'";
      if($country !== $countries_to_search_in[count($countries_to_search_in) - 1]) $query = $query." or";
    }

    $query = $query.")";
  } else {
    $query = $query. "select * from servers where (";
    foreach ($formats_to_search_for as $format) {
      $query = $query." $format = 'y'";
      if($format !== $formats_to_search_for[count($formats_to_search_for) - 1]) $query = $query." and";
    }

    $query = $query.") and (";
    foreach ($countries_to_search_in as $country) {
      $query = $query." country = '$country'";
      if($country !== $countries_to_search_in[count($countries_to_search_in) - 1]) $query = $query." or";
    }

    $query = $query.")";
  }

  return $query;
}

 ?>
