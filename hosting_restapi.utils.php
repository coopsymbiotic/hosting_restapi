<?php

/**
 * @file
 *   Helper functions for hosting_restapi.
 */

/**
 * Validate if the REST-ish function exists
 */
function hosting_restapi_invoke_function($func, $method) {
  $result = [];
  $f = 'hosting_restapi_' . $func . '_' . $method;

  try {
    if (function_exists($f)) {
      $result = $f();
    }
    else {
      $result = [
        'status' => 'error',
        'message' => "Unknown method for $func: $method",
      ];
    }
  }
  catch (Exception $e) {
    $result = [
      'status' => 'error',
      'message' => $e->getMessage(),
    ];
  }

  return $result;
}

/**
 * Checks if authentication token is valid for a given site.
 *
 * @param String $site_name
 * @param String $token
 * @returns Boolean
 */
function hosting_restapi_is_valid_auth_token($url, $token) {
  if (! $url) {
    throw new Exception('Missing url argument.');
  }

  $exists = db_query('SELECT count(*) as cpt FROM hosting_restapi_order WHERE url = :url AND token = :token', array(':url' => $url, ':token' => $token))->fetchField();
  return ($exists ? TRUE : FALSE);
}

/**
 * Given an authentication token, returns the invoice_id.
 * Returns NULL if the token is not valid.
 *
 * @param String $site_name
 * @param String $token
 * @returns Boolean
 */
function hosting_restapi_get_invoice_id_from_token($url, $token) {
  if (!$url) {
    throw new Exception('Missing url argument.');
  }
  if (!$token) {
    throw new Exception('Missing token argument.');
  }

  $invoice_id = db_query('SELECT invoice_id FROM hosting_restapi_order WHERE site = :url AND token = :token', array(':url' => $url, ':token' => $token))->fetchField();

  if (!$invoice_id) {
    throw new Exception('url/token authentication failed');
  }

  return $invoice_id;
}

/**
 * Validates the URL/invoice_id combo.
 * Returns the order_id.
 */
function hosting_restapi_validate_invoice_id_for_url($url, $invoice_id) {
  if (!$url) {
    throw new Exception('Missing url argument.');
  }
  if (!$invoice_id) {
    throw new Exception('Missing invoice_id argument.');
  }

  $order_id = db_query('SELECT id FROM hosting_restapi_order WHERE invoice_id = :invoice_id AND site = :url', array(':invoice_id' => $invoice_id, ':url' => $url))->fetchField();

  if (!$order_id) {
    throw new Exception('url/invoice_id authentication failed');
  }

  return $order_id;
}

/**
 * Returns a CiviCRM REST API object.
 */
function & hosting_restapi_civicrmapi() {
  require_once drupal_get_path('module', 'hosting_restapi') . '/hosting_restapi_civicrm/civicrm.api.php';

  $api = new civicrm_api3(array(
    'server' => variable_get('hosting_restapi_crmhost', NULL),
    'api_key' => variable_get('hosting_restapi_crmapikey', NULL),
    'key' => variable_get('hosting_restapi_crmkey', NULL),
    'path' => 'vendor/civicrm/civicrm-core/extern/rest.php',
  ));

  return $api;
}
