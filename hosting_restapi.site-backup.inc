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
  $f = 'hosting_restapi_site_backup_' . $method;

  try {
    if (function_exists($f)) {
      $result = $f();
    }
    else {
      $result = array(
        'status' => 'error',
        'message' => 'Unknown method for site_backup: ' . $_SERVER['REQUEST_METHOD'],
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
 * Implements the 'site/backup GET' API.
 */
function hosting_restapi_site_backup_get() {
  $url = $_GET['url'];
  $token = $_GET['token'];
  $new = $_GET['new'];

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
