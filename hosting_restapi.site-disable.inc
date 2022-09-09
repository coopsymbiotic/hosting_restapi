<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/disable' API.
 */
function hosting_restapi_site_disable() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $result = hosting_restapi_invoke_function('site_disable', $method);
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/disable POST' API.
 */
function hosting_restapi_site_disable_post() {
  $url = $_POST['url'];
  $invoice_id = $_POST['invoice'];
  $order_id = hosting_restapi_validate_invoice_id_for_url($url, $invoice_id);
  $node = hosting_get_site_by_url($url);
  hosting_add_task($node->nid, 'disable');

  return [
    'status' => 'success',
  ];
}
