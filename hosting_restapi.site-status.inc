<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/status' API.
 */
function hosting_restapi_site_status() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $result = hosting_restapi_invoke_function('site_status', $method);
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/status GET' API.
 */
function hosting_restapi_site_status_get() {
  $url = $_GET['url'];
  $token = $_GET['token'];

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

  $api = hosting_restapi_civicrmapi();

  $api->Symbiocivicrm->Getstatus([
    'invoice_id' => $invoice_id,
  ]);

  $result = $api->result->values;

  if ($api->is_error()) {
    return [
      'status' => 'error',
      'invoice_id' => $invoice_id,
      'data' => $api->errorMsg(),
    ];
  }

  return [
    'status' => 'success',
    'data' => $result,
  ];
}
