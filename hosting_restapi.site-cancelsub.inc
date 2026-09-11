<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/cancelsub' API.
 */
function hosting_restapi_site_cancelsub() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $result = hosting_restapi_invoke_function('site_cancelsub', $method);
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/cancelsub POST' API.
 */
function hosting_restapi_site_cancelsub_post() {
  $url = $_POST['url'];
  $token = $_POST['token'];

  if (!$url) {
    throw new Exception(t('The "url" parameter was empty.'));
  }

  if (!$token) {
    throw new Exception(t('The "token" parameter was empty.'));
  }

  $invoice_id = hosting_restapi_get_invoice_id_from_token($url, $token);

  if (!$invoice_id) {
    throw new Exception(t('Invalid token.'));
  }

  $api = hosting_restapi_civicrm_api($invoice_id);
  $api->Symbiocivicrm->cancelsub([
    'invoice_id' => $invoice_id,
    'details' => $_POST['details'] ?? '',
  ]);

  $result = $api->values;

  if ($api->is_error()) {
    return [
      'status' => 'error',
      'invoice_id' => $invoice_id,
      'data' => $api->errorMsg(),
    ];
  }

  return [
    'status' => 'success',
  ];
}
