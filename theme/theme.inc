<?php

/**
 * @file
 * Theme hooks.
 */

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_person_ethnography(array &$variables) {
  module_load_include('inc', 'islandora_ethnography', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/metadata');
  module_load_include('inc', 'islandora_entities', 'includes/entities_rss');
  drupal_add_css(drupal_get_path('module', 'islandora_ethnography') . '/css/islandora-person.css');

  $biography = "";
  $object = $variables['object'];
  $object_pid = $object->id;

  if ($object['MADS']) {
    $mads = $object['MADS']->content;
    $simplexml = simplexml_load_string($mads);

    if ($object['BIO']) {
      $bio = $object['BIO']->content;
      $variables['biography'] = islandora_ethnography_replace_ethnography_links(html_entity_decode($bio));
    }

    $variables['title'] = (string) $simplexml->authority->titleInfo->title;
    $activities = $simplexml->fieldOfActivity;
    foreach ($activities as $activity) {
      if ((string) $activity) {
        $fields_of_activity[] = (string) $activity;
      }
    }
    if (isset($fields_of_activity)) {
      $list_variables = array(
        'items' => $fields_of_activity,
        'title' => t('Research Interests'),
        'type' => 'ul',
        'attributes' => array('class' => 'research_interests'),
      );
      $variables['activities'] = theme_item_list($list_variables);
    }
    $depts = $simplexml->affiliation->organization;
    $colleages = array();

    if (count($colleages) > 0) {
      foreach ($colleages as $dept => $members) {
        foreach ($members as $pid => $scholar) {
          $links[] = l($scholar, "islandora/object/$pid/view");
        }
        $list_variables = array(
          'items' => $links,
          'title' => t('Other Scholars in @dept', array('@dept' => $dept)),
          'type' => 'ul',
          'attributes' => array('class' => 'fellow_scholars'),
        );
        $variables['fellow_scholars'][] = theme_item_list($list_variables);
      }
    }
    $identifiers = $simplexml->identifier;
    foreach ($identifiers as $identifier) {
      if ($identifier['type'] == 'u1' && (string) $identifier) {
        $themed_links = islandora_ethnography_get_related((string) $identifier, $variables['title'], 'citations');
      }
    }
    $name = $variables['title'];
    // Add the RSS image, with menu callback.
    $form = array();
    $form['bookmark_rss'] = array(
      '#weight' => -1,
      '#markup' => l(theme('image', array(
          'path' => drupal_get_path('module', 'islandora_ethnography') . '/images/rss.png',
          'title' => t('Present Entity data as list in an RSS form'),
          'attributes' => array(),
          )
        ), "islandora/entities/$object_pid/rss", array('html' => TRUE)),
      '#prefix' => '<div><div id="islandora-entities-rss-format" style="float:left;">',
      '#suffix' => '</div><div class="islandora-entities-rss-format-name" style="float:left;">Subscribe to ' . $name . ' RSS feed</div></div>',
    );
    $variables['rss_feed'] = drupal_render($form);
    if (!empty($themed_links)) {
      $variables['recent_citations'] = $themed_links;
    }
  }

  $tn = $object->relationships->get(NULL, 'hasRepresentativeImage');
  if ($tn) {
    $tn_pid = $tn[0]['object']['value'];
    // Find out if that object has a 'JPG' or a 'MEDIUM_SIZE' datastream.
    $tn_data = islandora_ethnography_get_object_data_from_cache($tn_pid);
    $variables['tn'] = '';
    if ($tn_data) {
      if (in_array('JPG', $tn_data['datastreams'], TRUE)) {
        $variables['tn'] = "/islandora/object/$tn_pid/datastream/JPG/view";
      }
      elseif (in_array('MEDIUM_SIZE', $tn_data['datastreams'], TRUE)) {
        $variables['tn'] = "/islandora/object/$tn_pid/datastream/MEDIUM_SIZE/view";
      }
    }


    module_load_include('inc', 'islandora_solr_metadata', 'theme/theme');
    $tn_object = new stdClass();
    $tn_object->id = $tn_pid;
    $solr_fields = array(
      'mods_accessCondition_copyright_s' => array(
        'displayLabel' => 'Copyright',
        'value' => array(),
        'hyperlink' => '0',
        'weight' => '1',
      ),
      'mods_abstract_s' => array(
        'displayLabel' => 'Abstract',
        'value' => array(),
        'hyperlink' => '0',
        'weight' => '3',
      ),
    );
    $success = islandora_solr_metadata_query_fields($tn_object, $solr_fields);
    if ($success) {
      if (!empty($solr_fields['mods_abstract_s']['value'])) {
        $variables['tn_copyright'] .= $solr_fields['mods_abstract_s']['value'][0];
      }
      if (!empty($solr_fields['mods_accessCondition_copyright_s']['value'])) {
        $variables['tn_copyright'] = $solr_fields['mods_accessCondition_copyright_s']['value'][0];
      }
    }

  }
  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);
}

/**
 * Gets all members of department.
 *
 * @param string $dept
 *   Department being queried
 * @param string $pid
 *   Object pid, results will be excluded
 *
 * @return array
 *   Department members
 */
function islandora_ethnography_get_dept_members($dept, $pid) {
  $params = array(
    'fl' => array('MADS_title_ms PID'),
  );
  $query = "MADS_organization_ms:\"$dept\" AND -PID:\"$pid\"";
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
  $colleages = array();
  foreach ($json['response']['docs'] as $choice) {
    if (isset($choice['MADS_title_ms'])) {
      foreach ($choice['MADS_title_ms'] as $candidate) {
        $colleages[$choice['PID']] = $candidate;
      }
    }
  }
  return $colleages;
}

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_dept_ethnography(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/metadata');
  module_load_include('inc', 'islandora_ethnography', 'includes/utilities');

  $object = $variables['object'];
  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);
  if ($object['BIO']) {
    $bio = $object['BIO']->content;
    $variables['biography'] = islandora_ethnography_replace_ethnography_links(html_entity_decode($bio));
  }
  $tn = $object->relationships->get(NULL, 'hasRepresentativeImage');
  if ($tn) {
    $tn_pid = $tn[0]['object']['value'];
    $variables['tn'] = "/islandora/object/$tn_pid/datastream/JPG/view";
  }
}

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_tune_ethnography(array &$variables) {
  module_load_include('inc', 'islandora_ethnography', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/metadata');
  module_load_include('inc', 'islandora_entities', 'includes/entities_rss');
  drupal_add_css(drupal_get_path('module', 'islandora_ethnography') . '/css/islandora-person.css');

  $biography = "";
  $object = $variables['object'];
  $object_pid = $object->id;

  $expressions = $object->relationships->get(ISLANDORA_ETHNOGRAPHY_FRBROO, 'R3_is_realised_in');
  $expressions += $object->relationships->get(ISLANDORA_ETHNOGRAPHY_FRBROO, 'R3_is_realised_in');
  if ($expressions) {
    $variables['notation'] = array();
    foreach ($expressions as $expression) {
      $expression_obj = islandora_object_load($expression['object']['value']);
      if ($expression_obj) {
        $variables['notation'][] = theme('islandora_pdfjs', array('fedora_object' => $expression_obj, 'dsid' => 'PDF'));
      }
    }
    $variables['notation'] = implode(' ', $variables['notation']);
  }

  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);

  if ($object['BIO']) {
    $bio = $object['BIO']->content;
    $variables['biography'] = islandora_ethnography_replace_ethnography_links(html_entity_decode($bio));
  }
}

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_notation_ethnography(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/metadata');
  $variables['metadata'] = islandora_retrieve_metadata_markup($variables['object'], TRUE);
  $variables['notation'] = theme('islandora_pdfjs', array('fedora_object' => $variables['object']));
}

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_story_ethnography(array &$variables) {
  module_load_include('inc', 'islandora_ethnography', 'includes/utilities');
  $object = $variables['object'];
  if (isset($object['OBJ'])) {
    $content = html_entity_decode($object['OBJ']->content);
    $variables['content'] = islandora_ethnography_replace_ethnography_links($content);
  }
  else {
    $variables['content'] = "";
  }
}

/**
 * Implements hook_preprocess().
 */
function islandora_ethnography_preprocess_islandora_video_ethnography(array &$variables) {
  module_load_include('inc', 'islandora', 'includes/datastream');
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/solution_packs');
  module_load_include('inc', 'islandora', 'includes/authtokens');
  module_load_include('inc', 'islandora', 'includes/metadata');
  drupal_add_js('misc/form.js');
  drupal_add_js('misc/collapse.js');
  $videojs_transcript_path = libraries_get_path("videojs-transcript");
  drupal_add_js($videojs_transcript_path . '/dist/videojs-transcript.js', array('weight' => 10));
  drupal_add_js(drupal_get_path('module', 'islandora_ethnography') . '/js/transcript.js', array('weight'=>11, 'scope' => 'footer'));
  drupal_add_css($videojs_transcript_path . '/css/videojs-transcript2.css');
  $object = $variables['object'];
  $repository = $object->repository;
  // We should eventually remove the DC object and dc_array code as it only
  // exists to not break legacy implementations.
  try {
    if (isset($object['DC']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['DC'])) {
      $dc = $object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
    }
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retriveing object %s %t', array('%s' => $object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : NULL;
  $variables['dc_array'] = isset($dc_object) ? $dc_object->asArray() : array();
  $variables['islandora_object_label'] = $object->label;
  $variables['theme_hook_suggestions'][] = 'islandora_video__' . str_replace(':', '_', $object->id);
  $variables['parent_collections'] = islandora_get_parents_from_rels_ext($object);
  $variables['metadata'] = islandora_retrieve_metadata_markup($object);
  $variables['description'] = islandora_retrieve_description_markup($object);

  $viewer_dsid = 'PROXY_MP4';
  $viewer_dsid_backup = 'OBJ';

  $mime = 'rtmp/mp4';

  $video_params = array(
    'pid' => $object->id,
  );
  // Video player.
//  if (isset($object[$viewer_dsid]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$viewer_dsid])) {
//    $video_url = $object[$viewer_dsid]->url;
//    $video_url = str_replace('http:', 'rtmp:', $video_url);
//    $video_params += array(
//      'mime' => $mime,
//      'url' => $video_url,
//    );
//  }
  if (isset($object[$viewer_dsid_backup]) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object[$viewer_dsid_backup])) {
    $video_url = url("islandora/object/{$object->id}/datastream/$viewer_dsid_backup/view", array('absolute' => TRUE));
    $video_params += array(
      'mime' => 'video/mp4',
      'url' => $video_url,
    );
  }
  // Thumbnail.
  if (isset($object['TN']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['TN'])) {
    $video_params += array(
      'tn' => url("islandora/object/{$object->id}/datastream/TN/view", array('absolute' => TRUE)),
    );
  }
  else {
    $video_params += array(
      'tn' => '',
    );
  }

  /*
  $tracks = array();
  // Transcript
  if (isset($object['WEBVTT']) && islandora_datastream_access(ISLANDORA_VIEW_OBJECTS, $object['WEBVTT'])) {
    $tracks[] = url("islandora/object/{$object->id}/rawdatastream/WEBVTT", array('absolute' => TRUE));
  }
  if (!empty($tracks)) {
    $video_params += array(
      'tracks' => $tracks,
    );
  }
  */

  $viewer = islandora_get_viewer($video_params, 'islandora_video_viewers', $object);
  $variables['islandora_content'] = '';
  if (isset($object['OBJ'])) {
    $download_url = url('islandora/object/' . $object->id . '/datastream/OBJ/download', array('absolute' => TRUE));
    $variables['download_link'] = l(t('Download MP4'), $download_url);
  }
  if ($viewer) {
    $variables['islandora_content'] = $viewer;
  }
  else {
    $variables['islandora_content'] = NULL;
  }
  drupal_add_css(drupal_get_path('module', 'islandora_ethnography') . '/css/islandora-video.css');

  return array('' => $viewer);
}

function islandora_ethnography_preprocess_islandora_ethnography_transcript_block(array &$variables) {
  module_load_include('inc', 'islandora_ethnography', 'includes/utilities');
  // get the transcript object.

  $object = $variables['object'];
  $transcript_pids = islandora_ethnography_get_transcript_object($object);
  if (!$transcript_pids) {
    return;
  }
  foreach($transcript_pids as $pid) {
    $transcript_object = islandora_object_load($pid);
    $content = html_entity_decode($transcript_object['HTML']->content);
  }
  $variables['content'] = islandora_ethnography_replace_ethnography_links($content); // might as well leave this here if we want to do cool stuff with entity recog/manual additions in the future.
  $variables['block_title'] = t('Transcript');
  $edit_access = islandora_object_manage_access_callback(array(ISLANDORA_METADATA_EDIT), $transcript_object);
  $variables['edit_link'] = $edit_access ? l(t('Edit transcript'), url("islandora/object/{$transcript_object->id}/datastream/HTML/edit")) : '';
}
