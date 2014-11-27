<?php
/**
 * @file
 * This is the template file for the object page for person objects.
 */
?>
<div class="islandora-object islandora clearfix">

  <div class="col-left">
    <div class="islandora-object-image">
      <?php if (isset($variables['tn'])): ?>
        <img src="<?php print $variables['tn']; ?>"/>
      <?php endif; ?>
    </div>
    <div class="islandora-object-metadata">
      <?php if (isset($variables['metadata'])): ?>
        <?php print $variables['metadata']; ?>
      <?php endif; ?>
    </div>
    <div class="islandora-object-activities">
      <?php if (isset($variables['activities'])): ?>
        <?php print $variables['activities']; ?>
      <?php endif; ?>
    </div>
  </div>



  <div class="islandora-object-content">
    <?php if (isset($variables['biography'])): ?>
      <h3 class="bio"><?php print t('About this tune'); ?></h3>
      <p><?php print $variables['biography']; ?></p>
    <?php endif; ?>


  </div>

  <?php if (isset($variables['notation'])): ?>
    <div class="pdfjs"
    <?php print ($variables['notation']); ?>
    </div>
  <?php endif; ?>
</div>
