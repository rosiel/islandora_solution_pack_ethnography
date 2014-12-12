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
      $variables['biography'] = html_entity_decode($bio);
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
    $variables['tn'] = "/islandora/object/$tn_pid/datastream/JPG/view";
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
  $object = $variables['object'];
  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);
  if ($object['BIO']) {
    $bio = $object['BIO']->content;
    $variables['biography'] = html_entity_decode($bio);;
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
  if ($expressions) {
    $variables['notation'] = array();
    foreach ($expressions as $expression) {
      $expression_obj = islandora_object_load($expression['object']['value']);
      if ($expression_obj) {
        $variables['notation'][] = theme('islandora_pdfjs', array('fedora_object' => $expression_obj));
      }
    }
    $variables['notation'] = implode(' ', $variables['notation']);
  }

  $variables['metadata'] = islandora_retrieve_metadata_markup($object, TRUE);

  if ($object['BIO']) {
    $bio = $object['BIO']->content;
    $variables['biography'] = html_entity_decode($bio);
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
  $object = $variables['object'];
  $variables['content'] = html_entity_decode($object['OBJ']->content);
}
