<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/getbilling' API.
 */
function hosting_restapi_site_getbilling() {
  $result = hosting_restapi_site_cancelsub_post();
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/getbilling POST' API.
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
  $api->Symbiocivicrm->getbilling([
    'invoice_id' => $invoice_id,
  ]);

  $result = $api->values;

  if ($api->is_error()) {
    return [
      'status' => 'error',
      'invoice_id' => $invoice_id,
      'data' => $api->errorMsg(),
    ];
  }

  return $result;
}
