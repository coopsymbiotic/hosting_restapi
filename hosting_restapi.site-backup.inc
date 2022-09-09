<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site/backup' API.
 */
function hosting_restapi_site_backup() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $result = hosting_restapi_invoke_function('site_backup', $method);
  echo json_encode($result);
  drupal_exit();
}

/**
 * Implements the 'site/backup GET' API.
 */
function hosting_restapi_site_backup_get() {
  $url = $_GET['url'];
  $token = $_GET['token'];
  $new = $_GET['new'];
  $invoice_id = hosting_restapi_get_invoice_id_from_token($url, $token);
  $node = hosting_get_site_by_url($url);

  if ($new) {
    hosting_add_task($node->nid, 'backup');

    return [
      'status' => 'success',
    ];
  }

  // Duplicates hosting_site_backup_list(), which returns only a formatted description
  // (we just want an ISO date and size)
  $result = db_query("SELECT bid, description, size, filename, FROM_UNIXTIME(timestamp) as date_created FROM {hosting_site_backups} WHERE site = :site ORDER BY timestamp DESC LIMIT 3", array(':site' => $node->nid));
  $backups = [];

  foreach ($result as $object) {
    $backups[$object->bid] = [
      'id' => $object->bid,
      'date' => $object->date_created,
      'filename' => $object->filename,
      'size' => $object->size,
    ];
  }

  return [
    'status' => 'success',
    'data' => $backups,
  ];
}
