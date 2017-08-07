<?php
     function crawler($crawl_links, $urls){
        $collection = array();
        $dom = new DOMDocument();
        while(count($crawl_links) !=0){
            $ch = curl_init();
            curl_setopt($ch, CURLOPT_URL, array_pop($crawl_links));
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);              
            $html = curl_exec($ch);

                //Parse and get URLs
            @$dom->loadHTML($html);
            foreach($dom->getElementsByTagName('a') as $tag){
                $foo_url = $tag->getAttribute('href');
               if(!in_array($foo_url, $collection)){
                    array_push($collection, $foo_url);
                }    
            }
            curl_close($ch);
        }
        return $collection;
    }

    if(isset($_POST['crawl'])){
        //Getting values entered from html form
        $url = $_POST['url'];
        $keyword = $_POST['keyword'];
        $depth = intval($_POST['depth']);

        //Gonna crawl through
        $urls = array($url);
        $foo_urls = $urls;
        while($depth !=0){
            $foo_urls = crawler($foo_urls, $urls);
            $urls = array_merge($urls, $foo_urls);
            $depth--;
        }
        
        foreach($urls as $foo_url){
            echo "$foo_url <br>";
        }

    }

?>

<html>
    <head>
        <title>Creeping on Your Website</title>
        <link rel="stylesheet" href="spiderCrawl.css"/>
    </head>
    <body>
        <form action="<?php echo $_SERVER['PHP_SELF']; ?>" method="POST" >
            <fieldset>
                <legend>PHP Creepies</legend>
                <label for="url">URL</label>
                <input name="url" type="text" required /><br>
                <label for="keyword">Keyword</label>
                <input name="keyword" type="text" required /><br>
                <label for="depth">Depth</label>
                <input name="depth" type="text" required /><br>
                <input name="crawl" type="submit" value="Crawl"><br>
                <textarea name="results" rows="20" cols="40" ></textarea>
            </fieldset>
       </form>
    </body>
</html>
