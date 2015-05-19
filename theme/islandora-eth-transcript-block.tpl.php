<?php
/**
 * @file
 * This is the template file for the object page for person objects.
 */
?>

<div class="ethnography-transcript islandora-object islandora clearfix">

  <div class="islandora-object-content islandora-transcript-content">
    <?php if (isset($variables['block_title'])): ?>
      <h2><?php print $variables['block_title']; ?></h2>
    <?php endif; ?>
    <?php if (isset($variables['content'])): ?>
      <?php print $variables['content']; ?>
    <?php endif; ?>
  </div>

</div>
