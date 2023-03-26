<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/delete' API.
 */
function hosting_restapi_site_delete() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $result = hosting_restapi_invoke_function('site_delete', $method);
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/delete POST' API.
 */
function hosting_restapi_site_delete_post() {
  $url = $_POST['url'];
  $invoice_id = $_POST['invoice'];
  $order_id = hosting_restapi_validate_invoice_id_for_url($url, $invoice_id);
  $node = hosting_get_site_by_url($url);

  // Assume that it has been manually deleted
  if (!$node) {
    return [
      'status' => 'success',
    ];
  }

  // Extra security measure, require the site to be disabled first
  // (not that it makes a huge difference, since the disable API uses the same security)
  if ($node->site_status != HOSTING_SITE_DISABLED) {
    return [
      'status' => 'error',
      'message' => 'The site is not disabled, cannot delete',
    ];
  }

  hosting_add_task($node->nid, 'delete');

  return [
    'status' => 'success',
  ];
}
