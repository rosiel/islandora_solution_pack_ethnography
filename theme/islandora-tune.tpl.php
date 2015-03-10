<?php
/**
 * @file
 * This is the template file for the object page for person objects.
 */
?>
<div class="islandora-object islandora clearfix">

  <div class="col-left">
    <div class="islandora-object-image islandora-tune">
      <?php if (isset($variables['tn'])): ?>
        <img class="islandora-thumbnail islandora-tune" src="<?php print $variables['tn']; ?>"/>
      <?php endif; ?>
    </div>
    <div class="islandora-object-metadata islandora-tune">
      <?php if (isset($variables['metadata'])): ?>
        <?php print $variables['metadata']; ?>
      <?php endif; ?>
    </div>
    <div class="islandora-object-activities islandora-tune">
      <?php if (isset($variables['activities'])): ?>
        <?php print $variables['activities']; ?>
      <?php endif; ?>
    </div>
  </div>



  <div class="islandora-object-content islandora-tune">
    <?php if (isset($variables['biography'])): ?>
      <h3 class="bio islandora-tune"><?php print t('About this tune'); ?></h3>
      <p><?php print $variables['biography']; ?></p>
    <?php endif; ?>


  </div>

  <?php if (isset($variables['notation'])): ?>
    <div class="pdfjs islandora-tune"
    <?php print ($variables['notation']); ?>
    </div>
  <?php endif; ?>
</div>
