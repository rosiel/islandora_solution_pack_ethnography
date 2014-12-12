<?php
/**
 * @file
 * This is the template file for the object page for person objects.
 */
?>
<?php if (isset($variables['tn'])): ?>
  <img src="<?php print $variables['tn']; ?>"/>
<?php endif; ?>
<?php if (isset($variables['biography'])): ?>
  <div class="biography">
    <?php print $variables['biography']; ?>
  </div>

<?php endif; ?>
