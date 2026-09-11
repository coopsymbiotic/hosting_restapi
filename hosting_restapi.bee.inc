<?php

/**
 * Implements hook_bee_command().
 */
function hosting_restapi_bee_command() {
  $items = [];
  $items['hosting-restapi-test'] = [
    'description' => 'Test that the REST API connection works',
    'bootstrap' => BACKDROP_BOOTSTRAP_FULL,
    'callback' => 'bee_hosting_restapi_test',
    'options' => [
      'invoice' => [
        'description' => bt('Invoice ID to test'),
        'value' => bt('value'),
      ],
    ],
  ];
  return $items;
}

/**
 *
 */
function bee_hosting_restapi_test($arguments, $options) {
  $invoice_id = $options['invoice'] ?? NULL;
  if (!$invoice_id) {
    $invoice_id = db_query('SELECT invoice_id FROM hosting_restapi_order ORDER BY created DESC LIMIT 1')->fetchField();
  }

  if (!$invoice_id) {
    throw new Exception('No invoice in the database, cannot test.');
  }

  $api = hosting_restapi_civicrm_api($invoice_id);

  $api->Symbiocivicrm->getconfig([
    'invoice_id' => $invoice_id,
  ]);

  $settings = $api->values;

  if ($api->is_error()) {
    bee_message('Failed: ' . $api->errorMsg(), 'error');
    return;
  }

  print_r($settings);

}
