<?php

function islandora_ethnography_get_info($object, $format = 'raw') {
  $raw = islandora_ethnography_get_raw_info($object);
  if ($format == 'raw') {
    print($raw);
    return $raw;
  }
  elseif ($format = 'audio_player') {
    return islandora_ethnography_info_format_audio_player($raw);
  }
  else {
    return '<p>FOOBAR WA HA HA! </p>';
  }

}

function islandora_ethnography_get_raw_info(AbstractObject $object) {
  return array(
    'title' => 'test me',
    'description' => 'foobarino! lorem ipsum.',
    'link' => 'islandora/object/islandora:797',
  );
}

function islandora_ethnography_info_format_audio_player(Array $data) {
  $output = '';
  if (isset($data['title'])) {
    $output .= '<div class="islandora-audio-player-title">' . $data['title'] . '</div>';
  }
  if (isset($data['description'])) {
    $output .= '<div class="islandora-audio-player-description">' . $data['description'] . '</div>';

  }
    if (isset($data['link'])) {
      $output .= '<div class="islandora-audio-player-more-link">' . l('More info', $data['link']) . '</div>';

    }
  return $output;
}

function islandora_ethnography_deliver_datastream(FedoraObject $object, $dsid) {
  if (isset($object[$dsid])) {
    $string = $object[$dsid]->content;
    $string = str_replace("\r", "\n", $string);
    echo $string;
    //drupal_exit();
  }
}

/**
 * Returns array of items to autocomplete forms.
 *
 * @param string $type
 *   The short name of the content type.
 *
 * @return array
 *   array of all candidates
 */
function islandora_ethnography_autocomplete($type) {

  $mappings = array(
    'person' => array(
      'cmodel' => 'islandora:personCModel',
      'title' => 'mads_authority_titleInfo_title_mt',
    ),
    'group' => array(
      'cmodel' => 'islandora:organizationCModel',
      'title' => 'fgs_label_mt',
    ),
    'general' => array(
      'cmodel' => 'islandora:personCModel',
      'title' => 'fgs_label_mt',
    ),
    'tune' => array(
      'cmodel' => ISLANDORA_ETHNOGRAPHY_TUNE_CONTENT_MODEL,
      'title' => 'fgs_label_mt',
    ),
    'story' => array(
      'cmodel' => ISLANDORA_ETHNOGRAPHY_STORY_CONTENT_MODEL,
      'title' => 'fgs_label_mt',
    ),
    'image' => array(
      'cmodel' => 'islandora:storyCModel',
      'title' => 'fgs_label_mt',
    ),
  );
  $qualifier = $mappings[$type]['title'] . ':*';
  $path = request_path();
  if ($path) {
    $path_array = explode('/', $path);
    $stub = end($path_array);
    $qualifier = $mappings[$type]['title'] . ':' . $stub . '*';
  }
  $params = array(
    'fl' => array($mappings[$type]['title'], 'PID'),
  );
  $query = "$qualifier AND " . 'RELS_EXT_hasModel_uri_mt:"' . $mappings[$type]['cmodel'] . '"';
  //print_r($query);
  $url = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));
  $solr = new Apache_Solr_Service($url['host'], $url['port'], $url['path'] . '/');
  $solr->setCreateDocuments(FALSE);
  try {
    $results = $solr->search($query, 0, 1000, $params);
    $json = json_decode($results->getRawResponse(), TRUE);
  }
  catch (Exception $e) {
    watchdog_exception('Islandora Entities', $e, 'Got an exception while searching entities for callback.', array(), WATCHDOG_ERROR);
  }

  $list = array();
  foreach ($json['response']['docs'] as $choice) {
    if (isset($choice[$mappings[$type]['title']])) {
      $list[$choice['PID']] = $choice[$mappings[$type]['title']][0];
    }
  }

  drupal_json_output($list);
}
