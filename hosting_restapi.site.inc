<?php

/**
 * @file
 *   Hosting REST API functions, and Drupal hooks.
 */

/**
 * Implements the 'site' API.
 */
function hosting_restapi_site() {
  $method = strtolower($_SERVER['REQUEST_METHOD']);
  $f = 'hosting_restapi_site_' . $method;

  try {
    if (function_exists($f)) {
      $result = $f();
    }
    else {
      $result = array(
        'status' => 'error',
        'message' => 'Unknown method for site: ' . $_SERVER['REQUEST_METHOD'],
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
 * Implements the 'site GET' API (get info/status).
 */
function hosting_restapi_site_get() {
  // FIXME: we want the user to have live updates on the status of the site,
  // so either we grant some temporary access, or we grant to anons.
  // TODO: use the invoice_id
  // hosting_restapi_check_access($_GET['key'], $_GET['secret']);

  $url = $_GET['url'];
  $invoice_id = (isset($_GET['invoice']) ? $_GET['invoice'] : NULL);

  if (! $url) {
    throw new Exception(t('The "url" parameter was empty.'));
  }

  // If we had the invoice_id, return the site status
  // NB: as a security measure, the invoice_id and url must match.
  if ($invoice_id) {
    $order_id = db_result(db_query('SELECT id FROM hosting_restapi_order WHERE invoice_id = "%s" AND site = "%s"', $invoice_id, $url));

    if (! $order_id) {
      throw new Exception(t('Invalid invoice ID or URL. Please contact support for more information.'));
    }

    // Fetch the last log.
    $result = db_query('SELECT * FROM hosting_restapi_log WHERE order_id = %d ORDER BY created DESC limit 1', $order_id);

    if ($record = db_fetch_object($result)) {
      // If it's a new order, check if the cloning has begun.
      if ($record->task == HOSTING_RESTAPI_ORDER_STATUS_NEW) {
        // FIXME HARDCODE: 608 = clone task of demo.s.c
        $vid = db_result(db_query("SELECT max(vid) from hosting_task_arguments where nid = 608 and name = 'new_uri' and value = '%s'", $url));
        $clone_status = db_result(db_query("SELECT task_status FROM hosting_task WHERE vid = %d", $vid));

        if ($clone_status == HOSTING_TASK_PROCESSING) {
          $result = array(
            'status' => 'success',
            'data' => array(
              'site_url' => $site->title,
              'site_status' => HOSTING_RESTAPI_ORDER_STATUS_CLONE_INPROGRESS,
            ),
          );
          return $result;
        }
      }

      $result = array(
        'status' => 'success',
        'data' => array(
          'site_url' => $site->title,
          'site_status' => $record->task,
        ),
      );
      return $result;
    }
  }

  // Otherwise just return if the site exists or not.
  $site = hosting_get_site_by_url($url);

  if (! $site) {
    throw new Exception(t('Site not found. Please contact support for more information.'));
  }

  $result = array(
    'status' => 'success',
    'data' => array(
      'site_url' => $site->title,
      'site_status' => $site->site_status,
    ),
  );

  return $result;
}

/**
 * Implements the 'site POST' API (create).
 */
function hosting_restapi_site_post() {
  // hosting_restapi_check_access($_POST['key'], $_POST['secret']);

  // TODO : check if URL format is OK (i.e. no spaces, etc)
  $url = check_plain($_POST['url']);

  $site = _hosting_restapi_is_allowed_creation($url);

  // Check if the order/invoice already exists.
  $invoice_id = $_POST['invoice'];

  hosting_restapi_is_valid_invoice($invoice_id);

  // Log order in hosting_restapi_order.
  $record = new StdClass();
  $record->invoice_id = $invoice_id;
  $record->site = $url;
  $record->token = sha1($url . uniqid(mt_rand()));
  $record->ip = $_SERVER['REMOTE_ADDR'];
  $record->current_status = HOSTING_RESTAPI_ORDER_STATUS_NEW;
  $record->created = time();
  $record->updated = time();

  drupal_write_record('hosting_restapi_order', $record);

  // Clone a site
  $template_nid = variable_get('hosting_saas_template_site_nid', NULL);
  $new_platform_nid = variable_get('hosting_saas_target_platform', NULL);
  $new_db_server = variable_get('hosting_saas_db_server', 2); // 2 = DB server on single-server default setups

  // TODO: Don't do hosting_add_task immediately, implement hook instead
  $result = hosting_add_task($template_nid, 'clone', array(
    'new_uri' => $url,
    // 2 is the default nid for single-server setups
    'new_db_server' => $new_db_server,
    'target_platform' => $new_platform_nid,
    'aliases' => '',
    'redirection' => '',
    'ssl_enabled' => 2, // FIXME : Use template site's setting?
    'ssl_key' => 5, // FIXME : Use template site's ssl_key? (Assuming SNI)
  ));

  hosting_restapi_log($url, HOSTING_RESTAPI_ORDER_STATUS_NEW, t('The order is valid and has been created.'));
  return array('status' => 'success', 'data' => $result);
}

function _hosting_restapi_is_allowed_creation($url) {
  if (!$url) {
    throw new Exception('The "url" parameter was empty.');
  }

  // This function checks for uniqueness of domains and aliases.
  // TODO: we should also implement hook_allow_domain() to enforce domain?
  if (! hosting_domain_allowed($url)) {
    throw new Exception('The url is not allowed by local configurations: ' . $url);
  }

  $site = hosting_get_site_by_url($url);

  if ($site) {
    throw new Exception('The site already exists.');
  }

  $variables = array(
    'hosting_saas_target_platform',
    'hosting_saas_db_server',
    'hosting_saas_template_site_nid',
  );

  foreach ($variables as $to_check) {
    if (variable_get($to_check, NULL) === NULL) {
      throw new Exception("Variable hasn't been chosen yet: " . $to_check);
    }
  }

  $template_site_nid = variable_get('hosting_saas_template_site_nid', NULL);

  // This tells us whether there's a clone task running or queued
  // for the site
  $sql = "SELECT n.nid as nid, ht.rid, task_status, executed
          FROM hosting_task ht INNER JOIN node n
          ON ht.nid = n.nid AND ht.vid = n.vid
          WHERE ht.rid = %d AND ht.task_type = 'clone'
          AND (ht.task_status = -1 OR ht.task_status = 0)
          LIMIT 1";
  
  // TRUE = Go ahead, FALSE = stop
  if (db_fetch_object(db_query($sql, $template_site_nid))) {
    throw new Exception('Server is busy (clone operation already in progress).');
  }

  watchdog('hosting_restapi', '_hosting_restapi_is_allowed_creation');
  return $site;
}

/**
 * Checks if an invoice ID is valid (not already used, and valid in the transactional site).
 * i.e. checks if the invoice_id is valid in CiviCRM.
 *
 * @param String $invoice_id
 */
function hosting_restapi_is_valid_invoice($invoice_id) {
  return TRUE;

  if (! $invoice_id) {
    throw new Exception('Missing invoice.');
  }

  // Check if the invoice_id was already used.
  // XXX assumes that 1 invoice = 1 site. We don't support multi-quota stuff,
  // since we don't give access to Aegir.
  $exists = db_result(db_query('SELECT count(*) as cpt FROM hosting_restapi_order WHERE invoice_id = "%s"', array($invoice_id)));

  if ($exists) {
    watchdog('hosting_restapi', 'Invoice ID already used.');
    throw new Exception('Invalid invoice ID.');
  }

  // Check that the invoice_id is legit.. supporting only CiviCRM for now.
  require_once drupal_get_path('module', 'hosting_restapi') . '/civicrm.api.php';
  $api = new civicrm_api3(array(
    'server' => variable_get('hosting_restapi_crmhost', NULL),
    'api_key' => variable_get('hosting_restapi_crmapikey', NULL),
    'key' => variable_get('hosting_restapi_crmkey', NULL),
  ));

  $api->Contribution->Get(array('invoice_id' => $invoice_id));

  if ($api->count == 1) {
    return TRUE;
  }

  throw new Exception('Could not lookup invoice.');
}