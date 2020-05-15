<?php
/**
 * @file
 * General functions include for the Resave Nodes module.
 *
 * This file has the breakout of the main function that does the node
 * resaving.  This was moved out of the resave_nodes.module file for better
 * organization.
 */


/**
 * @todo Please document this function.
 */
function resave_nodes_do_that_thing($form = array(), &$form_state = array()) {
  $config = config('resave_nodes.settings');
  $last_run               = $config->get('last_run');

  $batch = FALSE;
  if ((isset($form_state['clicked_button']['#value'])) && ($form_state['clicked_button']['#value'] == 'Resave Nodes Now')) {
    // Run immediately (and form selections may have changed).
    $batch = TRUE;
    resave_nodes_form_validate($form, $form_state);
    resave_nodes_form_submit($form, $form_state);
    $selected_content_types = $form_state['values']['content_types'];
    $all_nodes              = $form_state['values']['resave_all_nodes'];
  } else {
    // Running during cron so load selections from config.
    $selected_content_types = $config->get('selected_types');
    $all_nodes              = $config->get('all_nodes');
  }
  $start_time = time();
  $operations = array();

  $total_num_nodes_saved = 0;
  if (!empty($selected_content_types)) {
    foreach ($selected_content_types as $type => $value) {
      if ($type === $value) {
        // Check the node table first.
        // Default action: resave only recently created/changed.
        if ($all_nodes['resave_all_nodes'] == '0') {
          $query = db_query('SELECT nid FROM {node} WHERE changed >= :last_cron_run AND type = :content_type',
            array(
            ":last_cron_run" => $last_run,
            ":content_type" => $value,
          )
          );
          // Resave everything for specified content type.
        }
        else {
          $query = db_query('SELECT nid FROM {node} WHERE type = :content_type',
            array(
            ":content_type" => $value,
          )
          );
        }
        $num_nodes_saved = 0;
        foreach ($query as $result) {
          if ($batch) {
            $operations[] = array(
              'resave_nodes_save_it',
              array($result->nid, 'node', TRUE),
            );
          }
          else {
            $num_nodes_saved       += resave_nodes_save_it($result->nid, 'node', $batch);
            $total_num_nodes_saved += $num_nodes_saved;
          }
        }
      }
    }
  }

  if ($batch) {
    $batch_job = array(
      'operations' => $operations,
      'finished' => 'resave_nodes_batch_finished',
      'title' => t('Resaving content.'),
      'error_message' => t('Something broke!'),
    );
    if (empty($batch_job)) {
      backdrop_set_message(t('There is no content to resave.'), 'status');
    }
    else {
      batch_set($batch_job);
    }
    return $batch_job;

  }
  else {
    $time_delta = time() - $start_time;
    watchdog(
      'resave_nodes',
      'Finished updating @num new/changed nodes. Runtime was @time seconds',
      array(
      '@num' => print_r($total_num_nodes_saved, TRUE),
      '@time' => print_r($time_delta, TRUE),
    ),
      WATCHDOG_INFO
    );
    config_set('resave_nodes.settings', 'last_run', $start_time + $time_delta);

    return array(
      'number_updated' => $total_num_nodes_saved,
      'run_time' => $time_delta,
    );
  }
}

/**
 * @todo Please document this function.
 */
function resave_nodes_save_it($id, $bundle_type, $batch = FALSE, &$context = array()) {
  $num_saved = 0;
  $title     = '';
  $type      = '';

  if ((!empty($id)) && (!empty($bundle_type))) {
    try {
      if ($bundle_type == 'node') {
        $node  = node_load($id);
        $title = $node->title;
        $type  = $node->type;
        node_save($node);
      }
      $num_saved++;

      if ($batch) {
        $context['results'][] = t('Resaved node "@res" of @content type "@ctype".', array(
          '@res' => $title,
          '@content' => $bundle_type,
          '@ctype' => $type,
        ));
      }
      else {
        watchdog(
          'resave_nodes',
          'Resaved node "@res" of @content type "@ctype"',
          array(
          '@res' => print_r($title, TRUE),
          '@content' => print_r($bundle_type, TRUE),
          '@ctype' => print_r($type, TRUE),
        ),
          WATCHDOG_INFO
        );
      }
    }
    catch (Exception $e) {
      watchdog_exception($bundle_type, $e);
      throw $e;
    }
  }

  return $num_saved;
}

/**
 * @todo Please document this function.
 */
function resave_nodes_batch_finished($success, $results, $operations) {
  if ($success) {
    $nodes    = 0;
    $products = 0;
    foreach ($results as $res) {
      if (strpos($res, 'of node type') !== false) {
        $nodes++;
      }
      else {
        $products++;
      }
    }
    backdrop_set_message(t('Resaved @nodes nodes.',
      array(
      '@nodes' => $nodes,
      // '@products' => $products,
      // '@total'    => count($results),
    )
    ));
    watchdog(
      'resave_nodes',
      'Resaved @nodes nodes : <pre>@results</pre>',
      array(
      '@nodes' => $nodes,
      '@results' => print_r($results, TRUE),
    ),
      WATCHDOG_INFO
    );

  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    backdrop_set_message(
      t('An error occurred while while resaving @operation with arguments : @args',
        array(
      '@operation' => $error_operation[0],
      '@args' => print_r($error_operation[0], TRUE),
    )
      )
    );
  }
}
