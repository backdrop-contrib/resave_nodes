<?php
/**
 * @file
 * Administrative form include for the Resave Nodes module.
 *
 * This file has the breakout of the main function that does the node
 * resaving.  This was moved out of the resave_nodes.module file for better
 * organization.
 */


/**
 * Form builder.  Create and display the Resave Nodes config settings form.
 */
function resave_nodes_form($form, &$form_state) {
  $config = config('resave_nodes.settings');
  $content_types  = array();
  $validate_types = array();
  $everything     = entity_get_info();
  foreach ($everything as $type => $data) {
    if ((array_key_exists('bundles', $data)) && (!empty($data['bundles']))) {
      if ($type == 'node') {
        foreach ($data['bundles'] as $type_key => $type_data) {
          $content_types[$type_key] = $type_data['label'];
          $validate_types[]         = $type_key;
        }
      }
    }
  }
  config_set('resave_nodes.settings', 'validate_types', $validate_types);

  $selected_types         = $config->get('selected_types');
  $all_nodes              = $config->get('all_nodes');
  $scheduling             = $config->get('use_cron');
  $last_run               = $config->get('last_run');

  $form['content'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content Selection'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#description' => t('Select content/product types you would like
      periodically saved.  If you\'d like to resave nodes now, save the
      configuration first and then click \'Resave Nodes Now.\''),
  );

  if (!empty($content_types)) {
    $form['content']['content_types'] = array(
      '#title' => t('Content Types'),
      '#type' => 'checkboxes',
      '#options' => $content_types,
    );
    if (isset($selected_types)) {
      $form['content']['content_types']['#default_value'] = $selected_types;
    }
  }

  $form['content']['resave_all_nodes'] = array(
    '#title' => t('Resave All Nodes'),
    '#type' => 'checkboxes',
    '#options' => array(
      'resave_all_nodes' => t('Resave all nodes of the selected content types
      regardless of creation/change time.'),
    ),
    '#description' => t('By default, Resave Nodes will only resave nodes
      that have been created/changed since the last time it ran.  Selecting
      this option will cause all nodes of the selected content/product types to
      be resaved regardless of their creation/change times.'),
  );
  if (isset($all_nodes)) {
    $form['content']['resave_all_nodes']['#default_value'] = $all_nodes;
  }

  $form['scheduling'] = array(
    '#type' => 'fieldset',
    '#title' => t('Scheduling'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['scheduling']['description'] = array(
    '#type' => 'item',
    '#description' => t('Last run completed: @time',
      array(
      '@time' => format_date($last_run),
    )
    ),
  );

  $form['scheduling']['use_cron'] = array(
    '#title' => t('Resave nodes using...'),
    '#type' => 'radios',
    '#required' => TRUE,
    '#options' => array(
      'run_with_cron' => t('Backdrop Cron'),
      'none' => t('Do not run automatically'),
    ),
    '#default_value' => $scheduling,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save Configuration',
  );

  $form['run_now'] = array(
    '#type' => 'submit',
    '#value' => t('Resave Nodes Now'),
    '#validate' => array('resave_nodes_form_validate'),
    '#submit' => array('resave_nodes_do_that_thing'),
  );

  return $form;
}


/**
 * Form builder.  Validate values passed in the form.
 */
function resave_nodes_form_validate($form, &$form_state) {
  $validate_types = config_get('resave_nodes.settings', 'validate_types');

  // Validate the content types that are getting passed in.
  foreach ($form_state['values']['content_types'] as $name => $value) {
    if (($name == $value) || ($value == 0)) {
      // Good to go
    }
    else {
      form_set_error('content_types', 'Illegal content type.');
    }
    $valid_content_type = FALSE;
    foreach ($validate_types as $valid_name) {
      if ($name == $valid_name) {
        $valid_content_type = TRUE;
        break;
      }
    }
    if ($valid_content_type) {
      break;
    }
    else {
      form_set_error('content_types', 'Illegal content type.');
    }
  }

  // Verify if the 'resave_all_nodes' field has proper data.
  if (($form_state['values']['resave_all_nodes']['resave_all_nodes'] == 'resave_all_nodes') || ($form_state['values']['resave_all_nodes']['resave_all_nodes'] == 0)) {
    // Everything's cool.
  }
  else {
    form_set_error('resave_all_nodes', 'Invalid input for \'Resave All Nodes\' field.');
  }

  // Verify if the 'use_cron' field has proper data.
  if (($form_state['values']['use_cron'] == 'run_with_cron') || ($form_state['values']['use_cron'] == 'none')) {
    // Everything's cool.
  }
  else {
    form_set_error('use_cron', 'Invalid input for \'Resave nodes using...\' field.');
  }
}


/**
 * Form builder.  Save the selected content types.
 */
function resave_nodes_form_submit($form, &$form_state) {
  $config = config('resave_nodes.settings');
  config_set('resave_nodes.settings', 'selected_types', $form_state['values']['content_types']);
  config_set('resave_nodes.settings', 'all_nodes', $form_state['values']['resave_all_nodes']);
  config_set('resave_nodes.settings', 'use_cron', $form_state['values']['use_cron']);

  backdrop_set_message(t('The configuration has been saved.'));

  watchdog(
    'resave_nodes',
    'Configuration has been updated: <pre>@config</pre>',
    array(
    '@config' => print_r($config, TRUE),
  ),
    WATCHDOG_INFO
  );
}
