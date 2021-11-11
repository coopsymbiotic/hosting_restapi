<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/welcome' API.
 */
function hosting_restapi_site_config() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $f = 'hosting_restapi_site_config_' . $method;

  try {
    if (function_exists($f)) {
      $result = $f();
    }
    else {
      $result = array(
        'status' => 'error',
        'message' => 'Unknown method for site_config: ' . $_SERVER['REQUEST_METHOD'],
      );
    }
  }
  catch (Exception $e) {
    $result = array(
      'status' => 'error',
      'message' => $e->getMessage(),
    );
  }

  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/welcome POST' API.
 */
function hosting_restapi_site_config_post() {
  $url = $_POST['url'];
  $token = $_POST['token'];
  $loginurl = $_POST['loginurl'];

  if (!$url) {
    throw new Exception(t('The "url" parameter was empty.'));
  }

  if (!$token) {
    throw new Exception(t('The "token" parameter was empty.'));
  }

  if (!$loginurl) {
    throw new Exception(t('The "loginurl" parameter was empty.'));
  }

  $invoice_id = hosting_restapi_get_invoice_id_from_token($url, $token);

  if (!$invoice_id) {
    throw new Exception(t('Invalid token.'));
  }

  $api = hosting_restapi_civicrmapi();

  $api->Symbiocivicrm->Welcome([
    'invoice_id' => $invoice_id,
    'loginurl' => $loginurl,
  ]);

  $settings = $api->result->values;

  if ($api->is_error()) {
    return [
      'status' => 'error',
      'invoice_id' => $invoice_id,
      'data' => $api->errorMsg(),
    ];
  }

  return [
    'status' => 'success',
    'data' => $settings,
  ];
}
