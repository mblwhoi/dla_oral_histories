<?php
// $Id: luceneapi_cck.admin.inc,v 1.3 2010/01/26 02:23:49 cpliakas Exp $

/**
 * @file
 * Administrative settings callbacks for the Search Lucene CCK module.
 *
 * @ingroup luceneapi_cck
 */

/**
 * Administrative settings for Search Lucene CCK.
 *
 * @return
 *   A FAPI array containing the admin settings form.
 */
function luceneapi_cck_admin_settings() {
  $form = array('cck' => array());

  // Stores all available CCK fields.
  $form['content_fields'] = array('#type' => 'value', '#value' => content_fields());
  $form['index_settings'] = array('#type' => 'value', '#value' => array());

  // If no CCK fields are defined, adds message and bails.
  if (empty($form['content_fields']['#value'])) {
    $form['description'] = array(
      '#weight' => -30,
      '#value' => t(
        'No CCK fields are defined. Visit a content type\'s <a href="@content-type-page">manage fields</a> section to create a field.',
        array('@content-type-page' => url('admin/content/types'))
      ),
    );
    return;
  }

  // Help text on what this module does.
  $form['description'] = array(
    '#weight' => -30,
    '#value' => t(
      'Although CCK data is added to the node body when content is indexed, it may be useful to index the CCK values in separate Lucene fields as well. The most common use case is allowing for faceted search of CCK data. This interface allows administrators to select which <a href="@field-type-page" target="_blank">Lucene field types</a> will be used to store the CCK data. This module respects the settings in each content type\'s <em>Display fields</em> tab in the <a href="@content-type-page">manage fields</a> sections, so the data will not be indexed if the <em>Exclude</em> checkbox is selected for indexing. The <em>Field indexed</em> column in the table below denotes that at least one content type will index the CCK data with the node body.',
      array(
        '@field-type-page' => url('http://drupal.org/node/655724', array('absolute' => TRUE)),
        '@content-type-page' => url('admin/content/types'),
      )
    )
  );

  // Formats field types.
  $options = array(
    '0' => t('Do not index'),
    'unstored' => t('UnStored'),
    'keyword' => t('Keyword'),
    'unindexed' => t('UnIndexed'),
    'text' => t('Text'),
    'binary' => t('Binary'),
  );
  foreach ($form['content_fields']['#value'] as $field) {
    // Checks if field is indexed by at least one content type.
    $types = luceneapi_cck_field_indexed($field['field_name']);
    $form['index_settings']['#value'][$field['field_name']] = !empty($types);
    // Adds select box to associate a Lucene index field type with the CCK field.
    $variable = sprintf('luceneapi_cck:%s:field_type', $field['field_name']);
    $form['field_type'][$variable] = array(
      '#type' => 'select',
      '#title' => '',
      '#options' => $options,
      '#default_value' => variable_get($variable, 0),
    );
  }

  // Gets bias field checkboxes, sortable fields checkboxes.
  $options = array();
  foreach ($form['content_fields']['#value'] as $field) {
    $options[$field['field_name']] = '';
  }
  $form['luceneapi_cck:bias_fields'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get('luceneapi_cck:bias_fields', array()),
  );
  $form['luceneapi_cck:sortable_fields'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get('luceneapi_cck:sortable_fields', array()),
  );

  // Finalizes the form and returns.
  $form = system_settings_form($form);
  $form['#theme'] = 'luceneapi_cck_admin_settings_form';
  return $form;
}

/**
 * Themes the admin settings form into a table.
 *
 * @param $form
 *   A FAPI array containing the form.
 * @return
 *   A themed form element.
 */
function theme_luceneapi_cck_admin_settings_form($form) {
  $output = '';

  // Initializes table headers.
  $headers = array(
    t('CCK field'),
    t('Field indexed'),
    t('Lucene field type'),
    array('data' => t('Expose as bias field'), 'class' => 'checkbox'),
    array('data' => t('Expose as sortable field'), 'class' => 'checkbox'),
    t('Description'),
  );

  // Builds rows of CCK fields.
  $rows = array();
  foreach ($form['content_fields']['#value'] as $field) {
    $rows[] = array(
      'data' => array(
        array(
          'data' => check_plain($field['widget']['label']),
        ),
        array(
          'data' => ($form['index_settings']['#value'][$field['field_name']]) ? t('Yes') : t('No'),
        ),
        array(
          'data' => drupal_render($form['field_type']['luceneapi_cck:'. $field['field_name'] .':field_type']),
        ),
        array(
          'data' => drupal_render($form['luceneapi_cck:bias_fields'][$field['field_name']]),
          'class' => 'checkbox'
        ),
        array(
          'data' => drupal_render($form['luceneapi_cck:sortable_fields'][$field['field_name']]),
          'class' => 'checkbox'
        ),
        array(
          'data' => filter_xss($field['widget']['description']),
        ),
      ),
    );
  }

  // Themes the table.
  $form['cck'] = array(
    '#value' => theme('table', $headers, $rows, array('id' => 'luceneapi_cck-table')),
    '#weight' => -20,
  );

  // Renders the form.
  $output .= drupal_render($form);
  return $output;
}
