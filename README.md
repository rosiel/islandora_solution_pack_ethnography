# Islandora Ethnography Solution Pack


## Introduction

This module is for adding features that can be used to create musical ethnographies.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Islandora Basic Collection](https://github.com/Islandora/islandora_solution_pack_collection)
* [Islandora Solr](https://github.com/Islandora/islandora_solr_search)
* [Islandora Solr Metadata](https://github.com/Islandora/islandora_solr_metadata)
* [Islandora Bookmark](https://github.com/Islandora/islandora_bookmark)
* [Libraries](https://www.drupal.org/project/libraries)
* [Tipped js](https://www.tippedjs.com/) (optional - for tooltips on audio links)
* []

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Included:

### Tune Content Model `islandora:BDHTuneCModel`

This represents the _concept_ of a musical 'tune', e.g. 'Pigeon on the Gatepost'. It is a conceptual entity and has no OBJ datastream. It may contain a MODS (for title, and other structured metadata) and a BIO datastream containing an HTML snippet about the tune.

[A tune may be related to another tune by the _isVersionOf_ relationship in the RELS-EXT]

### Notation Content Model `islandora:BDHNotationCModel`

This represents a notated version of a tune. It contains a MODS record, a PDF that can be displayed in the browser, and optionally a .MUS file as the OBJ datastream, and/or a music XML file as the MusicXML datastream.

[A notation object may be related to a tune through the _R3i_realises_ relationship in the RELS-EXT.]

### Story Content Model `islandora:storyCModel`

This contains, as its OBJ datastream, a snippet of HTML containing comments from the curator.

[A story may include links to other objects in the repository, and if so, a pre-processing function may rewrite those links to have fancy tooltips showing extra information about the linked objects, or (if audio objects) to be playable in the browser on the story page.]

### Video Snippet Content Model `islandora:videoSnippetCModel`

This is a clone of the existing video content model. It was intended to represent a snippet that had been taken from a larger full-length video object, but that was never realized. 

[The module creates the content model as `islandora:videoSnippetCModel` but the code refers to objects with `bdh:videoSnippetCModel`. It looks like there was an incomplete attempt at refactoring.]

## Other Features

This module overrides the theme for the following objects using `hook_CMODEL_PID_islandora_view_object_alter()` :

* islandora:personCModel [Entities Solution Pack]
* islandora:organizationCModel [Entities Solution Pack]

It provides an autocomplete path, `/islandora/ethnography/autocomplete/[type]`, for types: `person`, `group`, `tune`, `story`, or `image`.  It searches on the fgs_label_mt solr field and returns the object's PID. This is used by the Relationships Editor module.

It processes the text that is displays from OBJ datastreams and nodes, to enrich "ethnography links" for various object types:
 * Audio (i.e. has a PROXY_MP3 datastream): links play in the "global" player (on the same page without navigating to the audio object's page). The link to the actual object displays in a tooltip using the Tipped.js library (not included).
 * Video (i.e. has a PROXY_MP4 datastream): replace the link with a videojs player (that plays the OBJ datastream - yes, this is inconsistent).

* Provides default icons (thumbnails) for the "ethnography content types" in solr search results
* Provides a block, "Islandora Ethnography Expression Of", which can be placed on Object pages. If the current object is a Notation object, it will show the Tune that Notation is an expression of.
* Provides links to download WAV or MP3's of Audio objects that appear on their object pages.







