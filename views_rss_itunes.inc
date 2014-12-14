<?php

/**
 * @file
 * Preprocess functions for Views RSS: iTunes Elements module.
 */

use Drupal\taxonomy\Entity\Term;

/**
 * Preprocess function for channel <itunes:category> element.
 */
function views_rss_itunes_preprocess_channel_category(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }

  $elements = array();
  foreach ($variables['elements'][0]['value'] as $tid) {
    // Load term object.
    $term = Term::load($tid);
    // Try to load parent term objects.
    $parents = \Drupal::entityManager()->getStorage('taxonomy_term')->loadParents($tid);
    // Term has a parent.
    if (count($parents)) {
      $parent = array_shift($parents);
      if (!isset($elements[$parent->id()])) {
        $elements[$parent->id()] = array(
          'key' => 'itunes:category',
          'attributes' => array(
            'text' => $parent->getName(),
          ),
        );
      }
      $elements[$parent->id()]['value'][$term->id()] = array(
        'key' => 'itunes:category',
        'attributes' => array(
          'text' => $term->getName(),
        ),
      );
    }
    // Term has no parent.
    else {
      $elements[$term->id()] = array(
        'key' => 'itunes:category',
        'attributes' => array(
          'text' => $term->getName(),
        ),
      );
    }
  }
  $variables['elements'] = $elements;
}

/**
 * Preprocess function for channel <itunes:image> element.
 */
function views_rss_itunes_preprocess_channel_image(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }

  $variables['elements'][0]['attributes'] = array(
    'href' => file_create_url($variables['elements'][0]['value']),
  );
  unset($variables['elements'][0]['value']);
}

/**
 * Preprocess function for channel <itunes:owner> element.
 */
function views_rss_itunes_preprocess_channel_owner(&$variables) {
  // No value = no preprocessing.
  if (empty($variables['elements'][0]['value'])) {
    return;
  }

  $values = explode(',', $variables['elements'][0]['value']);
  $variables['elements'][0]['value'] = array();

  // Add email subelement.
  $email = array_shift($values);
  if (!empty($email)) {
    $variables['elements'][0]['value']['itunes:email'] = trim($email);
  }

  // Add name subelement.
  $name = array_shift($values);
  if (!empty($name)) {
    $variables['elements'][0]['value']['itunes:name'] = trim($name);
  }
}

/**
 * Returns array of iTunes categories for channel <itunes:category>
 * element configuration in view options form.
 */
function views_rss_itunes_get_category_options() {
  $tree = \Drupal::entityManager()->getStorage('taxonomy_term')->loadTree('views_rss_itunes_category');
  $options = array();
  if (!empty($tree)) {
    foreach ($tree as $term) {
      $options[$term->tid] = str_pad(t($term->name), strlen(t($term->name)) + $term->depth * 2, '- ', STR_PAD_LEFT);
    }
  }
  return $options;
}

/**
 * Preprocess function for item <itunes:duration> element.
 */
function views_rss_itunes_preprocess_item_duration(&$variables) {
  // No raw values = no preprocessing.
  if (empty($variables['raw'])) {
    return;
  }

  $library = libraries_load('getID3');
  if (empty($library['loaded'])) {
    return;
  }

  $getID3 = new getID3;
  $variables['elements'] = array();
  foreach ($variables['raw'] as $item) {
    if (!empty($item['rendered']['#file'])) {
      $path = drupal_realpath($item['rendered']['#file']->getFileUri());
      $id3_info = $getID3->analyze($path);

      if (!empty($id3_info['playtime_string'])) {
        $variables['elements'][] = array(
          'key' => 'itunes:duration',
          'value' => $id3_info['playtime_string'],
        );
      }
    }
  }
}

/**
 * Preprocess function for item <itunes:image> element.
 */
function views_rss_itunes_preprocess_item_image(&$variables) {
  // No raw values = no preprocessing.
  if (empty($variables['raw'])) {
    return;
  }

  $variables['elements'] = array();
  foreach ($variables['raw'] as $item) {
    // File fields.
    if (!empty($item['rendered']['#file'])) {
      $file = $item['rendered']['#file'];
    }
    // Image fields.
    if (!empty($item['rendered']['#item']->entity)) {
      $file = $item['rendered']['#item']->entity;
    }

    // File entity found.
    if (!empty($file)) {
      // Image style is defined, need to link to resized version.
      if ($image_style_name = $item['rendered']['#image_style']) {
        $image_style = entity_load('image_style', $image_style_name);
        $href = $image_style->buildUrl($file->getFileUri());
      }
      // Normal image size.
      else {
        $href = $file->url();
      }
    }
    // No file entity found, but something still was assigned to be displayed
    // as enclosure, so we just put its value in the url attribute.
    elseif (!empty($item['rendered']['#markup'])) {
      $href = $item['rendered']['#markup'];
    }

    $element = array(
      'key' => 'itunes:image',
      'attributes' => array('href' => $href),
    );

    $variables['elements'][] = $element;
  }
}
