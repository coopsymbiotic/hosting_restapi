<?php

/**
 * @file
 *   Helper functions for hosting_restapi.
 */

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
  if (! $url) {
    throw new Exception('Missing url argument.');
  }

  return db_query('SELECT invoice_id FROM hosting_restapi_order WHERE site = :url AND token = :token', array(':url' => $url, ':token' => $token))->fetchField();
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
