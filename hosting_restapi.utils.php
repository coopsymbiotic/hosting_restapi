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

  $exists = db_result(db_query('SELECT count(*) as cpt FROM hosting_restapi_order WHERE url = "%s" AND token = "%s"', $url, $token));
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

  return db_result(db_query('SELECT invoice_id FROM hosting_restapi_order WHERE site = "%s" AND token = "%s"', $url, $token));
}

/**
 * Returns a CiviCRM REST API object.
 */
function & hosting_restapi_civicrmapi() {
  require_once drupal_get_path('module', 'hosting_restapi') . '/civicrm.api.php';

  $api = new civicrm_api3(array(
    'server' => variable_get('hosting_restapi_crmhost', NULL),
    'api_key' => variable_get('hosting_restapi_crmapikey', NULL),
    'key' => variable_get('hosting_restapi_crmkey', NULL),
  ));

  return $api;
}
