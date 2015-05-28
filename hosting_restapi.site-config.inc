<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/config' API.
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
 * Implements the 'site/config GET' API.
 */
function hosting_restapi_site_config_get() {
  $url = $_GET['url'];
  $token = $_GET['token'];

  if (! $url) {
    throw new Exception(t('The "url" parameter was empty.'));
  }

  if (! $token) {
    throw new Exception(t('The "token" parameter was empty.'));
  }

  $invoice_id = hosting_restapi_get_invoice_id_from_token($url, $token);

  if (! $invoice_id) {
    throw new Exception(t('Invalid token.'));
  }

  // TODO:
  $settings = array();

  $api = hosting_restapi_civicrmapi();
  $api->Contribution->Getsingle(array('trxn_id' => $invoice_id));
  $api->Contact->Getsingle(array('contact_id' => $api->result->contact_id));

  $settings['organization'] = array(
    'name' => $api->result->display_name,
    'street_address' => $api->result->street_address,
    'city' => $api->result->city,
    'state_province_id' => $api->result->state_province_id,
    'country_id' => $api->result->country_id,
    'phone' => $api->result->phone,
    'email' => $api->result->email,
  );

  // TODO: individual information
  // must fetch using the relation?

  return array('status' => 'success', 'data' => $settings);
}
