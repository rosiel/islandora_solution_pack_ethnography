<?php

/**
 * @file
 * Holds helper functions for islandora_entities.
 */

/**
 * Format links to audio objects for audiojs_global.
 *
 * This function will scan html text for links to islandora objects
 * that have the class "ethnography-link". (fix: remove condition?)
 * It will add the classes "audiojs-global-link silent" and a
 * data-source attribute pointing to the audio datastream, currently
 * only PROXY_MP3.
 *
 * This won't do anything alone, but if the audiojs_global module
 * is enabled, these links will play in the global player instead
 * of redirecting to the islandora object.
 *
 * @param string $content
 *   The html content to search, as a string.
 *
 * @return string
 *   The same content, with links pimped.
 */
function islandora_ethnography_replace_ethnography_links($content) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');

  $xml = new DOMDocument();
  $content = preg_replace('/&(?!#?[a-z0-9]+;)/', '&amp;', $content);
  if (!$xml->loadHTML($content)) {
    watchdog("Islandora Ethnography", "Unable to load HTML.");
  }
  else {
    // Add tooltip code for Tipped.js
    $tipped_path = libraries_get_path("tipped");
    drupal_add_js($tipped_path . '/js/tipped/tipped.js', array('weight' => 10));
    drupal_add_js(drupal_get_path('module', 'islandora_ethnography') . '/js/tipped.js', array('weight'=>11, 'scope' => 'footer'));
    drupal_add_css($tipped_path . '/css/tipped/tipped.css');

    if (strpos($content, 'show-text-en') !== FALSE) {
      drupal_add_js(drupal_get_path('module', 'islandora_ethnography') . '/js/languageSwitcher.js');
    }

    foreach ($xml->getElementsByTagName('a') as $link) {
      $classes = explode(' ', $link->getAttribute('class'));
      $target = $link->getAttribute('href');
      $target = ltrim($target, "\/");
      if (strpos($target, 'islandora/object/' !== 0)) {
        continue;
      }

      // Only rewrite links with the class 'ethnography-link'. FIXME?
      if (in_array('ethnography-link', $classes)) {
        $pid_part = arg(2, $target);
        $pid_part = str_replace('%3A', ':', $pid_part);
        $pid_part = explode('#', $pid_part);
        $pid = $pid_part['0'];
        $escaped_pid = str_replace(array(':','/','_'), '-', $pid);
        $time = isset($pid_part['1']) ? $pid_part['1'] : NULL;

        // Get data about this object, either from cache, or from the object.
        $data = islandora_ethnography_get_object_data_from_cache($pid);
        if (!$data) {
          continue;
        }

        // If the object has a PROXY_MP3 datastream, trick the link out
        // with classes to make it play in the global audiojs player.
        if (in_array('PROXY_MP3', $data['datastreams'], TRUE)) {
          $classes[] = 'gsm2-playable-link';
          $classes[] = 'silent';
          $link->setAttribute('class', implode(' ', $classes));
          $link->setAttribute('data-source', '/islandora/object/' . $pid . '/datastream/PROXY_MP3/view.mp3');
          // And also give it a tooltip with a link to the transcript.
          if (!empty($data['transcripts'])) {
            $link_text = t("Read transcript");
          } else {
            $link_text = t("More info");
          }
          $link->setAttribute('data-tipped-options', "inline: 'ethno-link-tooltip-$escaped_pid'");
          // Create the tooltip span (divs show by default with our css).
          $tooltip = $xml->createElement('span');
          $tooltip->setAttribute('id', "ethno-link-tooltip-$escaped_pid");
          $tooltip->setAttribute('style', 'display:none');
          $tooltip->setAttribute('class', 'tipped-tooltip');
          // Create the tooltip content.
          $tooltip_title = $xml->createElement('span', $data['label']);
          $tooltip_title->setAttribute('class', 'tipped-title');
          $tooltip_break = $xml->createElement('br');
          $tooltip_text = $xml->createElement('a', $link_text);
          $tooltip_text->setAttribute('href', '/'.$target);
          $tooltip_text->setAttribute('target', '_blank');
          // Attach the elements.
          $tooltip->appendChild($tooltip_title);
          $tooltip->appendChild($tooltip_break);
          $tooltip->appendChild($tooltip_text);
          $link->parentNode->insertBefore($tooltip);

        }

        if (in_array('PROXY_MP4', $data['datastreams'], TRUE)) {
          // The following is almost copy-pasta from preprocess_islandora_video_ethnography.
          // DOES NOT CHECK PERMISSIONS FOR PROXY_MP4
          $source_url = $data['proxy_mp4'];
          $vars = array();
/*
          // Add RTMP source
          $source_url_rtmp = str_replace('http', 'rtmp', $source_url);
          $vars['items'][] = array(
            'uri' => $source_url_rtmp,
            'filemime' => 'rtmp/mp4',
          );
          // Add Apple HLS source
          $source_url_hls = $source_url . '/playlist.m3u8';
          $vars['items'][] = array(
            'uri' => $source_url_hls,
            'filemime' => 'application/vnd.apple.mpegurl',
          );

          // Add Silverlight Microsoft Smooth Streaming source (?not sure if works?)
          $source_url_mss = $source_url . '/Manifest';
          $vars['items'][] = array(
            'uri' => $source_url_mss,
            'filemime' => 'application/x-silverlight-2',
          );
*/
          // Add OBJ source. Or don't to force streaming.

          if (in_array('OBJ', $data['datastreams'])) {
            $vars['items'][] = array(
              'uri' => '/islandora/object/' . $pid . '/datastream/OBJ/view',
              'filemime' => 'video/mp4',
            );
          }


          // Add Thumbnail.
          if (in_array('TN', $data['datastreams'])) {
            $poster_url = '/islandora/object/' . $pid . '/datastream/TN/view';
            $poster_mime = 'image/jpg';
          }
          if (isset($poster_url)) {
            $vars['items'][] = array(
              'uri' => $poster_url,
              'filemime' => $poster_mime,
            );
          }

          $vars['player_id'] = 'videojs-'. str_replace(':', '_', $pid);
          $vars['posterimage_style'] = NULL;
          $vars['attributes'] = array(
            'width' => 640,
            'height' => 360,
            'loop' => FALSE,
            'autoplay' => FALSE,
            'preload' => 'none',
            'hidecontrols' => FALSE,
          );
          $viewer = theme('videojs', $vars);
          if ($viewer) {
            videojs_add();
            // This is silly, but we need to DOMDocument-ify the html string.
            $videoDOM = new DOMDocument;
            libxml_use_internal_errors(true);
            $videoDOM->loadHTML($viewer);
            $xpath = "/html/body/child::node()[1]";
            $videoDOMXpath = new DOMXpath($videoDOM);
            $videoNode = $videoDOMXpath->query($xpath)->item(0);
            $importedVideoNode = $xml->importNode($videoNode, TRUE);
            $link->parentNode->insertBefore($importedVideoNode, $link);
          }
        }
      }
    }
  }

  return $xml->saveHTML();
}


/*
 * Gets some data about the object based on the pid, from cache if possible.
 * Returns $data which is an array,
 *   'label'  (object label)
 *   'models' (array of content models)
 *   'datastreams' (array of datastream IDs)
 *   'proxy_mp4' (url of the proxy mp4 datastream)
 */
function islandora_ethnography_get_object_data_from_cache($pid) {
  $safe_pid = str_replace(':', '_', $pid);
  // Get data about this object, either from cache, or from the object.
  $data = cache_get($safe_pid, 'cache_islandora_ethnography');
  if (!$data) {
    if ($obj = islandora_object_load($pid)) {
      $data = islandora_ethnography_get_object_data($obj);
      cache_set($safe_pid, $data, 'cache_islandora_ethnography');
    }
  }
  else {
    $data = $data->data;
  }
  return $data;
}

/*
 * Create an array to cache basic info about objects.
 */
function islandora_ethnography_get_object_data(AbstractObject $object) {
  $data = array(
    'datastreams' => array(),
    'models' => array(),
    'proxy_mp4' => NULL,
    'label' => '',
    'transcripts' => array(),
  );

  foreach($object->models as $model) {
    $data['models'][] = $model;
  }
  foreach($object as $ds) {
    $data['datastreams'][] = $ds->id;
  }
  $data['label'] = $object->label;
  if ($object['PROXY_MP4']) {
    $data['proxy_mp4'] = $object['PROXY_MP4']->url;
  }
  // Get transcripts from Solr.
  $params = array(
    'fl' => array('PID'),
  );
  $query = "RELS_EXT_P130_shows_features_of_uri_ms:\"info:fedora/{$object->id}\"";
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query, 0, 1000, $params);
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Ethnography', $e, 'Got an exception while searching for transcript.', array(), WATCHDOG_ERROR);
  }
  $transcripts = array();
  foreach ($json['response']['docs'] as $transcript) {
    if (isset($transcript['PID'])) {
        $transcripts[] = $transcript['PID'];
    }
  }
  $data['transcripts'] = $transcripts;
  return $data;

}

/**
 * This function will add the entity content model to an object.
 *
 * Due to natural inheritance in the data model all entity type
 * objects will have the content model of entity as well as one
 * for their types.
 *
 * @param AbstractObject $fedora_object
 *   The object to add the relationship to.
 */
function islandora_ethnography_add_entity_content_model(AbstractObject $fedora_object) {
  $fedora_object->relationships->add(
      FEDORA_MODEL_URI, 'hasModel', 'islandora:entityCModel'
  );
}

/**
 * This function will run a query using Solr for the specified entity type.
 *
 * @param string $content_model
 *   The PID of the content model object representing they type of entity.
 * @param string $entities_query
 *   The value of the query to run for the Solr search.
 *
 * @return array
 *   The results of the query. Limited to PID, object label and EAC-CPF name.
 *   IslandoraSolrQueryProcessor->islandoraSolrResult
 */
function islandora_ethnography_solr_search_entities($content_model, $entities_query) {
  module_load_include('inc', 'islandora_solr_search', 'includes/utilities');
  $results = array();

  $content_model_field = variable_get(
      'islandora_solr_content_model_field', 'RELS_EXT_hasModel_uri_ms'
  );
  $entities_query_field = variable_get(
      'islandora_entities_query_field', 'eaccpf_name_complete_et'
  );
  $object_label_field = variable_get(
      'islandora_solr_object_label_field', 'fgs_label_s'
  );

  $query_processor = new IslandoraSolrQueryProcessor();
  $query_processor->solrQuery = format_string(
      '!entities_query_field:(!entities_query)', array(
        '!entities_query_field' => $entities_query_field,
        '!entities_query' => islandora_solr_lesser_escape($entities_query),
      )
  );
  $query_processor->solrStart = 0;
  $query_processor->solrLimit = 10;
  $query_processor->solrParams = array(
    'fl' => "$entities_query_field, $object_label_field, PID",
    'fq' => format_string(
        '!content_model_field:"info:fedora/!content_model"', array(
          '!content_model_field' => $content_model_field,
          '!content_model' => $content_model,
        )
    ),
  );

  $query_processor->executeQuery(FALSE);
  $solr_results = $query_processor->islandoraSolrResult;
  return $solr_results;
}

/**
 * This function will add an entity DS composite datastream to an object.
 *
 * This is a workaround of a known issue with temp files being lost in Tuque in
 * batches because of object destruction during the serialization process.
 *
 * @param AbstractObject $islandora_object
 *   The object to add the DS composite datastream to.
 */
function islandora_ethnography_ingest__entity_ds_composite_datastream(AbstractObject $islandora_object) {
  $module_path = drupal_get_path('module', 'islandora_entities');

  $ds_composite_datastream = $islandora_object->constructDatastream('DS-COMPOSITE-MODEL', 'X');
  $ds_composite_datastream->label = 'DS-COMPOSITE-MODEL';
  $ds_composite_datastream->mimetype = 'text/xml';
  $ds_composite_datastream->setContentFromFile("$module_path/xml/islandora_entity_ds_composite_model.xml", FALSE);
  $islandora_object->ingestDatastream($ds_composite_datastream);
}

/**
 * Gets all citations related to scholars's unique identifier.
 *
 * @param string $identifier
 *   Scholar's unique ID
 *
 * @return array
 *   Array of links pointing to citations
 */
function islandora_ethnography_get_related($identifier, $title, $type) {
  $base_query = array(
    'citations' => variable_get(
      'islandora_entities_citation_base_query',
      '+RELS_EXT_hasModel_uri_mt:"ir:citationCModel"'),
    'theses' => variable_get(
      'islandora_entities_thesis_base_query',
      '+RELS_EXT_hasModel_uri_mt:"ir:thesisCModel"'),
  );
  $query = "+mods_name_personal_displayForm_mt:\"($identifier)\"  {$base_query[$type]}";
  $params = array(
    'fl' => 'dc.title, PID',
  );

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query, 0, 20, $params);
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Entities', $e, 'Got an exception while searching entities for callback.', array(), WATCHDOG_ERROR);
  }
  $links = array();
  foreach ($json['response']['docs'] as $choice) {
    if (isset($choice['dc.title'])) {
      $links[] = l($choice['dc.title'][0], 'islandora/object/' . $choice['PID']);
    }
  }
  if (count($links) > 0) {
    $list_variables = array(
      'items' => $links,
      'title' => t('Recent @type for @scholar', array('@type' => drupal_ucfirst($type), '@scholar' => $title)),
      'type' => 'ul',
      'attributes' => array('class' => 'recent_citations'),
    );
    return theme_item_list($list_variables);
  }
  return NULL;
}

/**
 * Creates themed list of scholar's citations.
 *
 * @param AbstractObject $object
 *   Object representing scholar
 *
 * @return html
 *   themed list of citations
 */
function islandora_ethnography_create_theses_tab(AbstractObject $object) {
  module_load_include('inc', 'islandora_entities', 'includes/utilities');
  if ($object['MADS']) {
    $mads = $object['MADS']->content;
    $simplexml = simplexml_load_string($mads);
    $identifiers = $simplexml->identifier;
    foreach ($identifiers as $identifier) {
      if ($identifier['type'] == 'u1') {
        return islandora_entities_get_related((string) $identifier, (string) $simplexml->authority->titleInfo->title, 'theses');
      }
    }
  }
}

/**
 * Gets all citations related to scholars's unique identifier.
 *
 * @param string $identifier
 *   Scholar's unique ID
 *
 * @return array
 *   Array of links pointing to citations
 */
function islandora_ethnography_get_related_pids($identifier, $title, $type) {
  $base_query = array(
    'citations' => variable_get(
      'islandora_entities_citation_base_query',
      '+RELS_EXT_hasModel_uri_mt:"ir:citationCModel"'),
    'theses' => variable_get(
      'islandora_entities_thesis_base_query',
      '+RELS_EXT_hasModel_uri_mt:"ir:thesisCModel"'),
  );
  $query = "+mods_name_personal_displayForm_mt:\"($identifier)\"  {$base_query[$type]}";

  $params = array(
    'fl' => 'dc.title, mods_name_personal_displayForm_mt, PID',
  );

  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $search_results = $solr->search($query, 0, 20, $params);
    $json = json_decode($search_results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Entities', $e, 'Got an exception while searching entities for callback.', array(), WATCHDOG_ERROR);
  }
  $results = array();
  foreach ($json['response']['docs'] as $choice) {
    $results[$choice['PID']] = array(
      'PID' => $choice['PID'],
      'title' => $choice['dc.title'],
      'authors' => $choice['mods_name_personal_displayForm_mt'],
    );
  }
  return $results;
}

/**
 * Adds entity autocomplete paths to a scholar citation form.
 *
 * @param array $form
 *   Drupal Form API array.
 */
function islandora_ethnography_add_autocompletes_to_citation_form(&$form) {
  // Skip the non-numeric keys.
  // Hack in the autocomplete paths.
  foreach ($form['authors'] as $key => $value) {
    if (is_int($key)) {
      $form['authors'][$key]['family']['#autocomplete_path'] = 'islandora/entities/autocomplete/last_name';
      $form['authors'][$key]['qualified_name']['#autocomplete_path'] = 'islandora/entities/autocomplete/disambiguated';
      $form['authors'][$key]['affiliation']['#autocomplete_path'] = 'islandora/entities/autocomplete/department';
    }
  }
}

/**
 * Adds entity autocomplete paths to a scholar thesis form.
 *
 * @param array $form
 *   Drupal Form API array.
 */
function islandora_ethnography_add_autocompletes_to_thesis_form(&$form) {
  // Skip the non-numeric keys.
  // Hack in the autocomplete paths.
  foreach ($form['advisor'] as $key => $value) {
    if (is_int($key)) {
      $form['advisor'][$key]['advisor']['#autocomplete_path'] = 'islandora/entities/autocomplete/disambiguated';
    }
  }

  foreach ($form['committee'] as $key => $value) {
    if (is_int($key)) {
      $form['committee'][$key]['commitee']['#autocomplete_path'] = 'islandora/entities/autocomplete/disambiguated';
    }
  }

  foreach ($form['granter'] as $key => $value) {
    if (is_int($key)) {
      $form['granter'][$key]['department']['#autocomplete_path'] = 'islandora/entities/autocomplete/department';
    }
  }
}

function islandora_ethnography_get_transcript_object(AbstractObject $object) {
  // do an RI or solr query on cidoc:P130_shows_features_of.
  $query = <<<EOQ
PREFIX cidoc: <http://www.cidoc-crm.org/cidoc-crm/>
SELECT ?pid
FROM <#ri>
WHERE {
  ?pid <cidoc:P130_shows_features_of> <info:fedora/{$object->id}> ;
       <fedora-model:hasModel> <info:fedora/bdh:transcriptionCModel> ;
       <fedora-model:state> <fedora-model:Active> .
}
EOQ;
  $results = $object->repository->ri->sparqlQuery($query);

  // Get rid of the "extra" info...
  $map = function($o) {
    foreach ($o as $key => &$info) {
      $info = $info['value'];
    }

    $o = array_filter($o);

    return $o;
  };
  $pages = array_map($map, $results);

  // Grab the PIDs...
  $get_pid = function($o) {
    return $o['pid'];
  };
  $pids = array_map($get_pid, $pages);

  return $pids;
}

