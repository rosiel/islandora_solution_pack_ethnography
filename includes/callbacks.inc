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