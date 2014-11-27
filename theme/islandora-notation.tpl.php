<?php
/**
 * @file
 * This is the template file for the object page for notation objects.
 */
?>
<?php if (isset($variables['metadata'])): ?>
  <div class="islandora-metadata">
    <?php print $variables['metadata']; ?>
  </div>
<?php endif; ?>
<?php if (isset($variables['notation'])): ?>
  <div class="notation">
    <?php print $variables['notation']; ?>
  </div>
<?php endif; ?>
