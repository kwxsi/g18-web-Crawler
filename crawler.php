<?php

include_once 'classes.inc';

set_time_limit(3600);
$url = $_POST['url'];
$depth = $_POST['depth'];

$crawler = new Crawler();

// Normal parameters
$crawler->setURL($url);
$crawler->setCrawlingDepthLimit($depth);

// Parameters for better crawling performance
$crawler->addContentTypeReceiveRule("#text/html#");
$crawler->addURLFilterRule("#\.(jpg|jpeg|gif|png|css|js|less|ico|pdf)$# i");
$crawler->setFollowMode(0);
$crawler->enableAggressiveLinkSearch(false);
$crawler->excludeLinkSearchDocumentSections(PHPCrawlerLinkSearchDocumentSections::SCRIPT_SECTIONS);

$crawler->go();


?>
