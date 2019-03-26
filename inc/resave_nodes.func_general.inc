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
 * Submit handler for the 'run now' button on the config page.
 */
function resave_nodes_run_now_submit() {
  // $binding_type = jhupbooks_import_determine_binding_type($type_to_import);
  // $product_elements = jhupbooks_import_grab_product_elements($type_to_import, $force_delta_import);
  // $books = jhupbooks_import_construct_books($binding_type, $product_elements);

  // if (empty($books)) {
  //   return array();
  // }

  // // Now we kick up the batch part of the book import process. First, build the
  // // operations and batch arrays. This involves going through the entire $books
  // // array and calling the needed functions.
  // $operations = array();
  // foreach ($books['bindings'] as $key => $book) {
  //   $operations[] = array(
  //     'jhupbooks_import_make_book',
  //     array($book, $binding_type),
  //   );
  // }
  // foreach ($books['invalid']['bindings'] as $key => $book) {
  //   $operations[] = array(
  //     'jhupbooks_import_delete_book',
  //     array($book, $binding_type),
  //   );
  // }
  // if (array_key_exists('titles', $books)) {
  //   foreach ($books['titles'] as $key => $book) {
  //     $operations[] = array(
  //       'jhupbooks_import_make_book',
  //       array($book, $binding_type),
  //     );
  //   }
  //   foreach ($books['invalid']['titles'] as $key => $book) {
  //     $operations[] = array(
  //       'jhupbooks_import_delete_book',
  //       array($book, $binding_type),
  //     );
  //   }
  // }

  // $batch = array(
  //   'operations'    => $operations,
  //   'finished'      => 'jhupbooks_import_finished',
  //   'title'         => t('Importing titles and bindings into Drupal.'),
  //   'error_message' => t('Something broke!'),
  // );

  // if (empty($batch)) {
  //   drupal_set_message(t('There are no book bindings/titles to import.'), 'status');
  // }
  // else {
  //   batch_set($batch);
  // }

  // return $batch;





  // $results = resave_nodes_do_that_thing();
  // drupal_set_message(t('Resaving nodes complete. @nodes nodes updated in @sec seconds.',
  //   array(
  //     '@nodes' => $results['number_updated'],
  //     '@sec'   => $results['run_time'],
  //   )
  // ));
}


function resave_nodes_do_that_thing() {
  $resave_nodes_last_run               = variable_get('resave_nodes_last_run');
  $resave_nodes_selected_content_types = variable_get('resave_nodes_selected_types', array());
  $resave_nodes_selected_product_types = variable_get('resave_nodes_selected_product_types', array());
  $resave_nodes_all_nodes              = variable_get('resave_nodes_all_nodes');

  $merged_types = array_merge($resave_nodes_selected_content_types, $resave_nodes_selected_product_types);

  $start_time = time();

  $total_num_nodes_saved = 0;
  if (!empty($merged_types)) {
    foreach ($merged_types as $type => $value) {
      if ($type === $value) {
        // Check the node table first.
        // Default action: resave only recently created/changed.
        if ($resave_nodes_all_nodes['resave_all_nodes'] == '0') {
          $query = db_query('SELECT nid FROM {node} WHERE changed >= :last_cron_run AND type = :content_type',
            array(
              ":last_cron_run" => $resave_nodes_last_run,
              ":content_type"  => $value,
            )
          );
        // Resave everything for specified content type.
        } else {
          $query = db_query('SELECT nid FROM {node} WHERE type = :content_type',
            array(
              ":content_type"  => $value,
            )
          );
        }
        $num_nodes_saved = 0;
        foreach ($query as $result) {
          $num_nodes_saved += resave_nodes_save_it($result->nid, 'node');
        }
        $total_num_nodes_saved += $num_nodes_saved;

        // Repeat the process for the commerce_product table (if installed).
        if ((module_exists('commerce_product')) && ($num_nodes_saved == 0)) {
          if ($resave_nodes_all_nodes['resave_all_nodes'] == '0') {
            $query = db_query('SELECT product_id FROM {commerce_product} WHERE changed >= :last_cron_run AND type = :content_type',
              array(
                ":last_cron_run" => $resave_nodes_last_run,
                ":content_type"  => $value,
              )
            );
          // Resave everything for specified product type.
          } else {
            $query = db_query('SELECT product_id FROM {commerce_product} WHERE type = :content_type',
              array(
                ":content_type"  => $value,
              )
            );
          }
          $num_nodes_saved = 0;
          foreach ($query as $result) {
            $num_nodes_saved += resave_nodes_save_it($result->product_id, 'commerce_product');
          }
          $total_num_nodes_saved += $num_nodes_saved;
        }
      }
    }
  }

  $time_delta = time() - $start_time;
  watchdog(
    'resave_nodes',
    'Finished updating @num new/changed nodes. Runtime was @time seconds',
    array(
      '@num'  => print_r($total_num_nodes_saved, TRUE),
      '@time' => print_r($time_delta, TRUE),
    ),
    WATCHDOG_INFO
  );
  variable_set('resave_nodes_last_run', $start_time + $time_delta);

  return array(
    'number_updated' => $total_num_nodes_saved,
    'run_time'       => $time_delta,
  );
}

function resave_nodes_save_it($id, $bundle_type) {
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
      } elseif ($bundle_type == 'commerce_product') {
        $product = commerce_product_load($id);
        $title = $product->title;
        $type  = $product->type;
        node_save($product);
      }
      $num_saved++;

      watchdog(
        'resave_nodes',
        'Resaved node "@res" of @content type "@ctype"',
        array(
          '@res'     => print_r($title, TRUE),
          '@content' => print_r($bundle_type, TRUE),
          '@ctype'   => print_r($type, TRUE),
        ),
        WATCHDOG_INFO
      );
    } catch (Exception $e) {
      watchdog_exception($bundle_type, $e);
      throw $e;
    }
  }

  return $num_saved;
}
